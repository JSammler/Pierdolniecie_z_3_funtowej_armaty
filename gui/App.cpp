#include "App.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <numbers>

#include <GLFW/glfw3.h>

#include "Menu.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl2.h"

namespace gui
{

namespace
{

constexpr double kDegToRad = std::numbers::pi / 180.0;
constexpr double kMaxAngleRad = 89.9 * kDegToRad;

void glfwErrorCallback(int code, const char* description)
{
    std::cerr << "GLFW error " << code << ": " << description << '\n';
}

} // namespace

App::App(const SimulationConfig& config)
    : startConfig_(config), game_(config)
{
}

void App::handleInput(double vw, double vh, double windowHeight)
{
    ImGuiIO& io = ImGui::GetIO();
    SimulationConfig& cfg = game_.config;

    if (!io.WantTextInput)
    {
        if (ImGui::IsKeyPressed(ImGuiKey_Space, false)) game_.fire();
        if (ImGui::IsKeyPressed(ImGuiKey_R, false)) game_.newTarget();

        const double angleStep = io.KeyCtrl ? 5.0 : (io.KeyShift ? 0.1 : 0.5);
        if (ImGui::IsKeyPressed(ImGuiKey_UpArrow))
        {
            cfg.angle_rad = std::clamp(cfg.angle_rad + angleStep * kDegToRad, 0.0, kMaxAngleRad);
            game_.configChanged();
        }
        if (ImGui::IsKeyPressed(ImGuiKey_DownArrow))
        {
            cfg.angle_rad = std::clamp(cfg.angle_rad - angleStep * kDegToRad, 0.0, kMaxAngleRad);
            game_.configChanged();
        }

        const double forceStep = io.KeyShift ? 0.005 : 0.02;
        if (ImGui::IsKeyPressed(ImGuiKey_RightArrow))
        {
            cfg.force_N *= 1.0 + forceStep;
            game_.configChanged();
        }
        if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow))
        {
            cfg.force_N = std::max(cfg.force_N * (1.0 - forceStep), 1.0);
            game_.configChanged();
        }
    }


    const double mx = io.MousePos.x - kMenuWidth;
    const double my = windowHeight - io.MousePos.y;
    const bool overScene = !io.WantCaptureMouse && mx >= 0.0 && mx <= vw && my >= 0.0 && my <= vh;
    if (overScene)
    {
        if (io.MouseWheel != 0.0f)
        {
            camera_.autoFit = false;
            camera_.zoomAt(mx, my, vw, vh, std::pow(1.15, static_cast<double>(io.MouseWheel)));
        }
        if (io.MouseDown[1] || io.MouseDown[2])
        {
            camera_.autoFit = false;
            camera_.pan(io.MouseDelta.x, -io.MouseDelta.y);
        }
    }
}

int App::run()
{
    glfwSetErrorCallback(glfwErrorCallback);
    if (!glfwInit())
    {
        std::cerr << "Nie udalo sie zainicjalizowac GLFW\n";
        return 1;
    }

    GLFWwindow* window = glfwCreateWindow(1400, 860, "3-funtowa armata", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Nie udalo sie utworzyc okna OpenGL\n";
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    ImGui::StyleColorsDark();
    ImGui::GetStyle().WindowRounding = 0.0f;
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL2_Init();

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        if (glfwGetWindowAttrib(window, GLFW_ICONIFIED))
        {
            glfwWaitEventsTimeout(0.1);
            continue;
        }

        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        int winW = 0, winH = 0, fbW = 0, fbH = 0;
        glfwGetWindowSize(window, &winW, &winH);
        glfwGetFramebufferSize(window, &fbW, &fbH);
        const double pxScale = winW > 0 ? static_cast<double>(fbW) / winW : 1.0;
        const double vw = std::max(1.0, winW - static_cast<double>(kMenuWidth));
        const double vh = std::max(1.0, static_cast<double>(winH));
        const double dt = std::min(static_cast<double>(io.DeltaTime), 0.1);

        handleInput(vw, vh, winH);
        game_.update(dt);
        drawMenu(game_, camera_, startConfig_, static_cast<float>(winH));
        camera_.update(game_, vw, vh, dt);

        glViewport(0, 0, fbW, fbH);
        glDisable(GL_SCISSOR_TEST);
        glClearColor(0.05f, 0.06f, 0.09f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        const int vpX = static_cast<int>(std::lround(kMenuWidth * pxScale));
        const int vpW = std::max(1, fbW - vpX);
        glViewport(vpX, 0, vpW, fbH);
        glEnable(GL_SCISSOR_TEST);
        glScissor(vpX, 0, vpW, fbH);
        renderer_.drawScene(game_, camera_, vw, vh);
        glDisable(GL_SCISSOR_TEST);

        renderer_.drawLabels(ImGui::GetBackgroundDrawList(), game_, camera_,
                             ImVec2(kMenuWidth, static_cast<float>(winH)), vw, vh);

        ImGui::Render();
        glViewport(0, 0, fbW, fbH);
        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

}
