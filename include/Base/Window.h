#pragma once
#include "Base/Keyboard.h"
#include "Base/Mouse.h"
#include "Base/NonCopyable.h"
#include <string>


#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

struct GLFWwindow;

class Window : public NonCopyable
{
public:
    Window();
    virtual ~Window();
    void Create(const std::string &applicationName = "Application",
                const int width = 800,
                const int height = 600);
    void Destroy();

    void PollEvents();
    bool ShouldClose();

    void CreateWindowSurface(const VkInstance instance, VkSurfaceKHR *surface);

    void preProcessing();
    void postProcessing();

    GLFWwindow *pGLFWWindow()
    {
        return pGLFWWindow_;
    }

    Mouse &mouse()
    {
        return mouse_;
    }

    Keyboard &keyboard()
    {
        return keyboard_;
    }

private:
    // Mouse Callback
    static void CursorPosCallback(GLFWwindow *window, double xpos, double ypos);
    static void CursorEntryCallback(GLFWwindow *window, int entered);
    static void ScrollCallback(GLFWwindow *window, double xoffset, double yoffset);
    static void MouseButtonCallback(GLFWwindow *window, int button, int action, int mods);
    // Keyboard Callback
    static void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
    static void CharCallback(GLFWwindow *window, unsigned int codepoint);
    // Window Callback
    static void FramebufferResizeCallback(GLFWwindow *window, int width, int height);

private:
    GLFWwindow *pGLFWWindow_;
    int width_;
    int height_;
    Mouse mouse_;
    Keyboard keyboard_;
};
