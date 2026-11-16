#include <filesystem>
#include <stdexcept>

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl2.h>

#include "Window.hpp"

namespace
{
    // Font systemowy z polskimi znakami (domyślny font ImGui ich nie ma)
    void loadFont()
    {
        static const ImWchar ranges[] = {
            0x0020, 0x00FF, // Basic Latin + Latin-1
            0x0100, 0x017F, // Latin Extended-A (ą, ć, ę, ł, ń, ś, ź, ż)
            0,
        };
        for (const char* path : {"C:/Windows/Fonts/segoeui.ttf", "C:/Windows/Fonts/arial.ttf"})
        {
            if (std::filesystem::exists(path))
            {
                ImGui::GetIO().Fonts->AddFontFromFileTTF(path, 17.0f, nullptr, ranges);
                return;
            }
        }
        ImGui::GetIO().Fonts->AddFontDefault();
    }
}

Window::Window(int width, int height, const char* title)
{
    if (!glfwInit()) throw std::runtime_error("Failed to initialize GLFW");

    handle = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!handle)
    {
        glfwTerminate();
        throw std::runtime_error("Failed to create window");
    }
    glfwMakeContextCurrent(handle);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::GetIO().IniFilename = nullptr;
    ImGui::StyleColorsDark();
    loadFont();

    ImGui_ImplGlfw_InitForOpenGL(handle, true);
    ImGui_ImplOpenGL2_Init();
}

Window::~Window()
{
    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(handle);
    glfwTerminate();
}

bool Window::shouldClose() const
{
    return glfwWindowShouldClose(handle);
}

void Window::beginFrame()
{
    glfwPollEvents();

    int fbWidth, fbHeight;
    glfwGetFramebufferSize(handle, &fbWidth, &fbHeight);
    glViewport(0, 0, fbWidth, fbHeight);
    glClearColor(0.11f, 0.11f, 0.12f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui_ImplOpenGL2_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void Window::endFrame()
{
    ImGui::Render();
    ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(handle);
}
