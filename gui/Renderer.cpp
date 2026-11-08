#include "Renderer.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <numbers>

#include <GLFW/glfw3.h>

namespace gui
{

namespace
{

struct Color
{
    float r, g, b, a;
};

constexpr double kPi = std::numbers::pi;

void setColor(Color c) { glColor4f(c.r, c.g, c.b, c.a); }

void fillRect(float x0, float y0, float x1, float y1, Color c)
{
    setColor(c);
    glBegin(GL_QUADS);
    glVertex2f(x0, y0);
    glVertex2f(x1, y0);
    glVertex2f(x1, y1);
    glVertex2f(x0, y1);
    glEnd();
}

void vGradient(float x0, float y0, float x1, float y1, Color bottom, Color top)
{
    glBegin(GL_QUADS);
    setColor(bottom);
    glVertex2f(x0, y0);
    glVertex2f(x1, y0);
    setColor(top);
    glVertex2f(x1, y1);
    glVertex2f(x0, y1);
    glEnd();
}

void line(float x0, float y0, float x1, float y1, float width, Color c)
{
    glLineWidth(width);
    setColor(c);
    glBegin(GL_LINES);
    glVertex2f(x0, y0);
    glVertex2f(x1, y1);
    glEnd();
}

void fillCircle(float cx, float cy, float r, Color c, int seg = 40)
{
    setColor(c);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= seg; ++i)
    {
        const double a = 2.0 * kPi * i / seg;
        glVertex2f(cx + static_cast<float>(std::cos(a)) * r, cy + static_cast<float>(std::sin(a)) * r);
    }
    glEnd();
}

void ringCircle(float cx, float cy, float r, float width, Color c, int seg = 48)
{
    glLineWidth(width);
    setColor(c);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < seg; ++i)
    {
        const double a = 2.0 * kPi * i / seg;
        glVertex2f(cx + static_cast<float>(std::cos(a)) * r, cy + static_cast<float>(std::sin(a)) * r);
    }
    glEnd();
}

double niceStep(double minWorld)
{
    const double p = std::pow(10.0, std::floor(std::log10(minWorld)));
    for (double m : {1.0, 2.0, 5.0, 10.0})
        if (m * p >= minWorld) return m * p;
    return 10.0 * p;
}

}

void Renderer::drawScene(const Game& game, const Camera& cam, double vw, double vh) const
{
    if (vw < 10.0 || vh < 10.0) return;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, vw, 0.0, vh, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

    const float W = static_cast<float>(vw);
    const float H = static_cast<float>(vh);
    const double s = cam.scale;
    const float groundY = static_cast<float>(cam.sy(0.0, vh));
    auto SX = [&](double wx) { return static_cast<float>(cam.sx(wx, vw)); };
    auto SY = [&](double wy) { return static_cast<float>(cam.sy(wy, vh)); };


    vGradient(0, 0, W, H, {0.62f, 0.80f, 0.95f, 1.f}, {0.07f, 0.12f, 0.30f, 1.f});

    {
        const double step = niceStep(90.0 / s);
        const double left = cam.worldX(0.0, vw);
        const double right = cam.worldX(vw, vw);
        const double bottom = std::max(0.0, cam.worldY(0.0, vh));
        const double top = cam.worldY(vh, vh);
        const Color grid{1.f, 1.f, 1.f, 0.10f};

        int guard = 0;
        for (double x = std::ceil(left / step) * step; x <= right && guard < 200; x += step, ++guard)
            line(SX(x), std::max(groundY, 0.f), SX(x), H, 1.f, grid);
        guard = 0;
        for (double y = std::ceil(bottom / step) * step; y <= top && guard < 200; y += step, ++guard)
            if (y > 0.0) line(0, SY(y), W, SY(y), 1.f, grid);
    }

    // ---- Ziemia ----
    if (groundY > 0.f)
    {
        const float gy = std::min(groundY, H);
        vGradient(0, 0, W, gy, {0.10f, 0.20f, 0.10f, 1.f}, {0.24f, 0.42f, 0.22f, 1.f});
        line(0, groundY, W, groundY, 2.f, {0.08f, 0.16f, 0.08f, 1.f});

        const double step = niceStep(90.0 / s);
        const double left = cam.worldX(0.0, vw);
        const double right = cam.worldX(vw, vw);
        int guard = 0;
        for (double x = std::ceil(left / step) * step; x <= right && guard < 200; x += step, ++guard)
            line(SX(x), groundY, SX(x), groundY - 7.f, 1.5f, {1.f, 1.f, 1.f, 0.55f});
    }


    for (const auto& ghost : game.ghosts())
    {
        glLineWidth(1.5f);
        setColor({1.f, 1.f, 1.f, 0.22f});
        glBegin(GL_LINE_STRIP);
        for (const Vec2& p : ghost) glVertex2f(SX(p.x), SY(p.y));
        glEnd();
    }

    if (game.showPreview && game.previewValid())
    {
        const auto& st = game.preview().states;
        const std::size_t stride = std::max<std::size_t>(1, st.size() / 1500);
        glEnable(GL_LINE_STIPPLE);
        glLineStipple(3, 0x00FF);
        glLineWidth(2.f);
        setColor({0.4f, 1.f, 0.55f, 0.9f});
        glBegin(GL_LINE_STRIP);
        for (std::size_t i = 0; i < st.size(); i += stride) glVertex2f(SX(st[i].position.x), SY(st[i].position.y));
        glVertex2f(SX(game.preview().landing.x), SY(game.preview().landing.y));
        glEnd();
        glDisable(GL_LINE_STIPPLE);
    }


    {
        const Target& t = game.target();
        const float cx = SX(t.center.x);
        const float cy = SY(t.center.y);
        const float r = std::max(static_cast<float>(t.radius * s), 4.f);

        if (cy - r > groundY + 1.f)
        {
            line(cx, groundY, cx, cy - r, 4.f, {0.40f, 0.27f, 0.14f, 1.f});
            fillRect(cx - 8.f, groundY - 1.f, cx + 8.f, groundY + 3.f, {0.30f, 0.20f, 0.10f, 1.f});
        }
        else
        {
            line(cx - r * 0.5f, groundY, cx, cy - r * 0.2f, 3.f, {0.40f, 0.27f, 0.14f, 1.f}); // podpórki
            line(cx + r * 0.5f, groundY, cx, cy - r * 0.2f, 3.f, {0.40f, 0.27f, 0.14f, 1.f});
        }

        if (game.targetHit()) fillCircle(cx, cy, r * 1.18f, {0.2f, 1.f, 0.3f, 0.35f});
        fillCircle(cx, cy, r, {0.96f, 0.96f, 0.92f, 1.f});              // 25 pkt
        fillCircle(cx, cy, r * 0.5f, {0.85f, 0.12f, 0.12f, 1.f});       // 50 pkt
        fillCircle(cx, cy, r * 0.2f, {1.f, 0.85f, 0.15f, 1.f});         // 100 pkt
        ringCircle(cx, cy, r, 2.f, {0.1f, 0.1f, 0.1f, 1.f});
        ringCircle(cx, cy, r * 0.5f, 1.f, {0.1f, 0.1f, 0.1f, 0.7f});
    }

    // ---- Ślad i kula ----
    const bool ballVisible = !game.flight().empty() &&
                             (game.phase() == Phase::Flying || game.sinceFinish() < 1.5);
    if (ballVisible)
    {
        const auto& st = game.flight().states;
        const std::size_t last = game.ballIndex();
        const std::size_t stride = std::max<std::size_t>(1, last / 2000);
        glLineWidth(2.5f);
        setColor({1.f, 0.62f, 0.15f, 0.95f});
        glBegin(GL_LINE_STRIP);
        for (std::size_t i = 0; i <= last; i += stride) glVertex2f(SX(st[i].position.x), SY(st[i].position.y));
        const Vec2 b = game.ballPosition();
        glVertex2f(SX(b.x), SY(b.y));
        glEnd();

        const float bx = SX(b.x);
        const float by = SY(b.y);
        fillCircle(bx, by, 5.5f, {0.08f, 0.08f, 0.10f, 1.f}, 24);
        fillCircle(bx - 1.6f, by + 1.6f, 1.8f, {0.7f, 0.7f, 0.75f, 1.f}, 12);
    }


    {
        const float px = SX(game.config.muzzle.x);
        const float py = SY(game.config.muzzle.y);
        const double ang = game.config.angle_rad;
        const float dx = static_cast<float>(std::cos(ang));
        const float dy = static_cast<float>(std::sin(ang));
        const float nx = -dy;
        const float ny = dx;

        if (py - groundY > 6.f)
        {
            fillRect(px - 15.f, groundY, px + 15.f, py - 4.f, {0.45f, 0.45f, 0.48f, 1.f});
            line(px - 15.f, groundY, px - 15.f, py - 4.f, 2.f, {0.25f, 0.25f, 0.28f, 1.f});
            line(px + 15.f, groundY, px + 15.f, py - 4.f, 2.f, {0.25f, 0.25f, 0.28f, 1.f});
        }

        // koło
        fillCircle(px, py, 16.f, {0.40f, 0.26f, 0.12f, 1.f});
        fillCircle(px, py, 12.f, {0.55f, 0.37f, 0.18f, 1.f});
        for (int i = 0; i < 6; ++i)
        {
            const float a = static_cast<float>(i * kPi / 3.0);
            line(px, py, px + std::cos(a) * 15.f, py + std::sin(a) * 15.f, 2.f, {0.32f, 0.20f, 0.09f, 1.f});
        }
        ringCircle(px, py, 16.f, 2.f, {0.18f, 0.11f, 0.05f, 1.f});

        // lufa (zwężający się trapez)
        const float L = 48.f;
        const float back = 12.f;
        const float wb = 9.f;
        const float wf = 6.5f;
        glBegin(GL_QUADS);
        setColor({0.22f, 0.24f, 0.28f, 1.f});
        glVertex2f(px - dx * back + nx * wb, py - dy * back + ny * wb);
        glVertex2f(px + dx * L + nx * wf, py + dy * L + ny * wf);
        glVertex2f(px + dx * L - nx * wf, py + dy * L - ny * wf);
        glVertex2f(px - dx * back - nx * wb, py - dy * back - ny * wb);
        glEnd();
        line(px - dx * back + nx * wb * 0.5f, py - dy * back + ny * wb * 0.5f,
             px + dx * L + nx * wf * 0.5f, py + dy * L + ny * wf * 0.5f, 2.f, {0.55f, 0.58f, 0.64f, 0.8f});
        line(px + dx * (L - 4.f) + nx * (wf + 1.5f), py + dy * (L - 4.f) + ny * (wf + 1.5f),
             px + dx * (L - 4.f) - nx * (wf + 1.5f), py + dy * (L - 4.f) - ny * (wf + 1.5f), 4.f, {0.12f, 0.13f, 0.16f, 1.f});
        fillCircle(px, py, 6.f, {0.12f, 0.13f, 0.16f, 1.f}, 20);

        // błysk i dym przy wystrzale
        const double tf = game.sinceFire();
        if (tf < 1.2 && !game.flight().empty())
        {
            const float tipX = px + dx * L;
            const float tipY = py + dy * L;
            if (tf < 0.12)
            {
                const float k = static_cast<float>(1.0 - tf / 0.12);
                fillCircle(tipX + dx * 8.f, tipY + dy * 8.f, 8.f + 22.f * k, {1.f, 0.55f, 0.1f, 0.85f * k}, 24);
                fillCircle(tipX + dx * 8.f, tipY + dy * 8.f, 4.f + 12.f * k, {1.f, 0.95f, 0.5f, k}, 20);
            }
            const float fade = static_cast<float>(1.0 - tf / 1.2);
            for (int i = 0; i < 5; ++i)
            {
                const float d = 10.f + static_cast<float>(tf) * (14.f + 10.f * i);
                const float rad = 5.f + static_cast<float>(tf) * (10.f + 4.f * i);
                fillCircle(tipX + dx * d, tipY + dy * d + static_cast<float>(tf) * 8.f, rad,
                           {0.75f, 0.75f, 0.78f, 0.35f * fade}, 20);
            }
        }
    }

    glDisable(GL_LINE_SMOOTH);
    glLineWidth(1.f);
}

void Renderer::drawLabels(ImDrawList* dl, const Game& game, const Camera& cam, ImVec2 origin, double vw, double vh) const
{
    if (vw < 10.0 || vh < 10.0) return;

    auto P = [&](double sx, double sy) { return ImVec2(origin.x + static_cast<float>(sx), origin.y - static_cast<float>(sy)); };
    const ImU32 white = IM_COL32(255, 255, 255, 210);
    const ImU32 dim = IM_COL32(255, 255, 255, 140);

    const double s = cam.scale;
    const double step = niceStep(90.0 / s);
    const double groundY = cam.sy(0.0, vh);
    const double left = cam.worldX(0.0, vw);
    const double right = cam.worldX(vw, vw);


    int guard = 0;
    for (double x = std::ceil(left / step) * step; x <= right && guard < 200; x += step, ++guard)
    {
        const std::string label = formatDistance(x);
        const ImVec2 sz = ImGui::CalcTextSize(label.c_str());
        const ImVec2 p = P(cam.sx(x, vw), groundY - 10.0);
        if (groundY > 14.0 && p.y < origin.y)
            dl->AddText(ImVec2(p.x - sz.x * 0.5f, p.y), dim, label.c_str());
    }

    const double bottom = std::max(0.0, cam.worldY(0.0, vh));
    const double top = cam.worldY(vh, vh);
    guard = 0;
    for (double y = std::ceil(bottom / step) * step; y <= top && guard < 200; y += step, ++guard)
        if (y > 0.0) dl->AddText(P(6.0, cam.sy(y, vh) + 14.0), dim, formatDistance(y).c_str());


    {
        const Target& t = game.target();
        char buf[96];
        std::snprintf(buf, sizeof buf, "CEL  %s", formatDistance(t.center.x - game.config.muzzle.x).c_str());
        const ImVec2 sz = ImGui::CalcTextSize(buf);
        const double r = std::max(t.radius * s, 4.0);
        const ImVec2 p = P(cam.sx(t.center.x, vw), cam.sy(t.center.y, vh) + r + 18.0);
        dl->AddText(ImVec2(p.x - sz.x * 0.5f, p.y), IM_COL32(255, 230, 120, 255), buf);
    }


    {
        char buf[48];
        std::snprintf(buf, sizeof buf, "%.1f deg", game.config.angle_rad * 180.0 / kPi);
        const ImVec2 sz = ImGui::CalcTextSize(buf);
        const ImVec2 p = P(cam.sx(game.config.muzzle.x, vw), cam.sy(game.config.muzzle.y, vh) + 76.0);
        dl->AddText(ImVec2(p.x - sz.x * 0.5f, p.y), white, buf);
    }


    const ShotReport& rep = game.report();
    if (rep.valid && game.phase() == Phase::Idle && game.sinceFinish() < 5.0)
    {
        const float alpha = static_cast<float>(std::clamp(5.0 - game.sinceFinish(), 0.0, 1.0));
        ImU32 col = IM_COL32(255, 120, 120, static_cast<int>(255 * alpha));
        if (rep.error) col = IM_COL32(255, 160, 80, static_cast<int>(255 * alpha));
        else if (rep.points >= 100) col = IM_COL32(255, 220, 60, static_cast<int>(255 * alpha));
        else if (rep.points > 0) col = IM_COL32(120, 255, 140, static_cast<int>(255 * alpha));

        ImFont* font = ImGui::GetFont();
        const float fs = 26.f;
        const ImVec2 sz = font->CalcTextSizeA(fs, 10000.f, 0.f, rep.text.c_str());
        const ImVec2 p = P(vw * 0.5, vh - 50.0);
        dl->AddText(font, fs, ImVec2(p.x - sz.x * 0.5f + 2.f, p.y + 2.f), IM_COL32(0, 0, 0, static_cast<int>(200 * alpha)), rep.text.c_str());
        dl->AddText(font, fs, ImVec2(p.x - sz.x * 0.5f, p.y), col, rep.text.c_str());
    }
}

}
