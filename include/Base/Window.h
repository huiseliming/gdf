#pragma once
#include "Base/Keyboard.h"
#include "Base/Mouse.h"
#include "Base/NonCopyable.h"
#include <string>

#include <GLFW/glfw3.h>

struct GLFWwindow;
namespace gdf
{
class GDF_EXPORT Window : public NonCopyable
{
public:
    Window();
    virtual ~Window();
    void Create(const std::string &applicationName = "Application", const int width = 800, const int height = 600);
    void Destroy();

    void PollEvents();
    bool ShouldClose();

    void preProcessing();
    void postProcessing();

    VkResult GetVkSurfaceKHR(const VkInstance instance, VkSurfaceKHR *surface);

    GLFWwindow *pGLFWWindow()
    {
        return pGLFWWindow_;
    }

    int width()
    {
        return width_;
    }
    int height()
    {
        return height_;
    }
    Mouse &mouse()
    {
        return mouse_;
    }

    Keyboard &keyboard()
    {
        return keyboard_;
    }
    bool framebufferResized()
    {
        return framebufferResized_;
    }

    static bool GetRequiredInstanceExtensions(std::vector<const char *> &glfwRequiredInstanceExtensions);

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
    static void WindowSizeCallback(GLFWwindow *window, int width, int height);
    static void FramebufferResizeCallback(GLFWwindow *window, int width, int height);

private:
    GLFWwindow *pGLFWWindow_{nullptr};
    int width_{0};
    int height_{0};
    Mouse mouse_;
    Keyboard keyboard_;
    bool framebufferResized_{false};
    
};
} // namespace gdf