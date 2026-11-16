#include <algorithm>
#include <cmath>
#include <cstdio>
#include <limits>

#include <GLFW/glfw3.h>
#include <imgui.h>

#include "TrajectoryView.hpp"

namespace
{
    constexpr float MarginLeft = 60.0f;
    constexpr float MarginRight = 25.0f;
    constexpr float MarginTop = 25.0f;
    constexpr float MarginBottom = 45.0f;
    constexpr float GridSpacingPx = 80.0f; // przybliżona odległość linii siatki
    constexpr float BallRadiusPx = 6.0f;

    // "Ładny" krok siatki: 1, 2 lub 5 * 10^n
    double niceStep(double raw)
    {
        const double magnitude = std::pow(10.0, std::floor(std::log10(raw)));
        const double n = raw / magnitude;
        if (n <= 1.0) return magnitude;
        if (n <= 2.0) return 2.0 * magnitude;
        if (n <= 5.0) return 5.0 * magnitude;
        return 10.0 * magnitude;
    }

    void drawCircle(double cx, double cy, double r)
    {
        constexpr int segments = 24;
        glBegin(GL_TRIANGLE_FAN);
        glVertex2d(cx, cy);
        for (int i = 0; i <= segments; ++i)
        {
            const double a = 2.0 * 3.14159265358979 * i / segments;
            glVertex2d(cx + r * std::cos(a), cy + r * std::sin(a));
        }
        glEnd();
    }
}

void TrajectoryView::setTrajectory(const StepsStack& head_)
{
    head = head_;
    hasTrajectory = true;
    playTime = 0.0;

    stats = {};
    stats.valid = true;
    minX = maxX = maxY = 0.0;

    // Przejście po liście jednokierunkowej: granice i statystyki lotu
    for (const StepsStack* node = &head; node != nullptr; node = node->next)
    {
        const ProjectileState& s = node->state;
        minX = std::min(minX, s.position.x);
        maxX = std::max(maxX, s.position.x);
        maxY = std::max(maxY, s.position.y);
        stats.flightTime = s.time;
        stats.range = s.position.x;
        stats.points++;
    }
    stats.maxHeight = maxY;

    maxX = std::max(maxX, minX + 1.0);
    maxY = std::max(maxY, 1.0);
}

void TrajectoryView::clear()
{
    head = {};
    hasTrajectory = false;
    stats = {};
    minX = 0.0;
    maxX = 100.0;
    maxY = 50.0;
    playTime = 0.0;
}

void TrajectoryView::draw(float x, float y, float w, float h, bool animate, float playbackSpeed)
{
    const ImGuiIO& io = ImGui::GetIO();

    // Obszar wykresu w pikselach okna
    const float px = x + MarginLeft;
    const float py = y + MarginTop;
    const float pw = w - MarginLeft - MarginRight;
    const float ph = h - MarginTop - MarginBottom;
    if (pw < 10.0f || ph < 10.0f) return;

    // Zakres świata [m] z tą samą skalą na obu osiach, z małym zapasem
    const double x0 = minX, y0 = 0.0;
    const double metersPerPixel = std::max((maxX - minX) * 1.05 / pw, maxY * 1.1 / ph);
    const double x1 = x0 + metersPerPixel * pw;
    const double y1 = y0 + metersPerPixel * ph;
    const double step = niceStep(metersPerPixel * GridSpacingPx);

    // --- OpenGL: siatka i trajektoria we współrzędnych świata ---
    const int fbHeight = static_cast<int>(io.DisplaySize.y * io.DisplayFramebufferScale.y);
    glViewport(static_cast<int>(px * io.DisplayFramebufferScale.x),
               fbHeight - static_cast<int>((py + ph) * io.DisplayFramebufferScale.y),
               static_cast<int>(pw * io.DisplayFramebufferScale.x),
               static_cast<int>(ph * io.DisplayFramebufferScale.y));

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(x0, x1, y0, y1, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glLineWidth(1.0f);
    glColor3f(0.22f, 0.22f, 0.24f);
    glBegin(GL_LINES);
    for (int i = static_cast<int>(std::ceil(x0 / step)); i * step <= x1; ++i)
    {
        glVertex2d(i * step, y0);
        glVertex2d(i * step, y1);
    }
    for (int i = 1; i * step <= y1; ++i)
    {
        glVertex2d(x0, i * step);
        glVertex2d(x1, i * step);
    }
    glEnd();

    double currentTime = 0.0;
    if (hasTrajectory)
    {
        const double endTime = animate ? playTime : std::numeric_limits<double>::infinity();
        const ProjectileState* last = &head.state;

        glLineWidth(2.0f);
        glColor3f(0.35f, 0.6f, 0.95f);
        glBegin(GL_LINE_STRIP);
        for (const StepsStack* node = &head; node != nullptr; node = node->next)
        {
            if (node->state.time > endTime) break;
            glVertex2d(node->state.position.x, node->state.position.y);
            last = &node->state;
        }
        glEnd();

        glColor3f(0.9f, 0.9f, 0.9f);
        drawCircle(last->position.x, std::max(last->position.y, 0.0), BallRadiusPx * metersPerPixel);
        currentTime = last->time;

        if (animate && playTime < stats.flightTime) playTime += io.DeltaTime * playbackSpeed;
    }

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    // --- Ramka i opisy osi (tekst przez ImGui, rysowany w tej samej klatce OpenGL) ---
    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    const ImU32 axisColor = IM_COL32(200, 200, 200, 255);
    const ImU32 textColor = IM_COL32(170, 170, 170, 255);
    auto toScreenX = [&](double wx) { return px + static_cast<float>((wx - x0) / metersPerPixel); };
    auto toScreenY = [&](double wy) { return py + ph - static_cast<float>((wy - y0) / metersPerPixel); };

    dl->AddRect({px, py}, {px + pw, py + ph}, axisColor);

    char buf[32];
    for (int i = static_cast<int>(std::ceil(x0 / step)); i * step <= x1; ++i)
    {
        std::snprintf(buf, sizeof(buf), "%g", i * step);
        const float tw = ImGui::CalcTextSize(buf).x;
        dl->AddText({toScreenX(i * step) - tw / 2.0f, py + ph + 4.0f}, textColor, buf);
    }
    for (int i = 0; i * step <= y1; ++i)
    {
        std::snprintf(buf, sizeof(buf), "%g", i * step);
        const ImVec2 ts = ImGui::CalcTextSize(buf);
        dl->AddText({px - ts.x - 6.0f, toScreenY(i * step) - ts.y / 2.0f}, textColor, buf);
    }
    dl->AddText({px + pw - ImGui::CalcTextSize("x [m]").x, py + ph + 22.0f}, axisColor, "x [m]");
    dl->AddText({px - 45.0f, py - 22.0f}, axisColor, "y [m]");

    if (hasTrajectory)
    {
        std::snprintf(buf, sizeof(buf), "t = %.2f s", currentTime);
        dl->AddText({px + pw - ImGui::CalcTextSize(buf).x - 8.0f, py + 6.0f}, textColor, buf);
    }
}
