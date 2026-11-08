#pragma once

struct GLFWwindow;

// Okno aplikacji: GLFW + kontekst OpenGL + inicjalizacja Dear ImGui.
class Window
{
    GLFWwindow* handle = nullptr;

public:
    Window(int width, int height, const char* title);
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    bool shouldClose() const;

    // Obsługa zdarzeń, wyczyszczenie ekranu i rozpoczęcie klatki ImGui
    void beginFrame();
    // Narysowanie ImGui i zamiana buforów
    void endFrame();
};
