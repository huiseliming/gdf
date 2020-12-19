#pragma once
#include "Base/NonCopyable.h"
#include <string>

#include <glm/vec2.hpp>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

struct GLFWwindow;

struct Mouse {
    glm::vec2 position;
    glm::vec2 offset;
};

class Window : public NonCopyable
{
public:
    Window();
    virtual ~Window();
    void Create(std::string applicationName = "Application", int width = 800, int height = 600);
    void Destroy();

    void PollEvents();
    bool ShouldClose();

    static void CursorPosCallback(GLFWwindow *window, double xpos, double ypos);
    static void ScrollCallback(GLFWwindow *window, double xoffset, double yoffset);
    static void MouseButtonCallback(GLFWwindow *window, int button, int action, int mods);
    static void FramebufferResizeCallback(GLFWwindow *window, int width, int height);

    void CreateWindowSurface(VkInstance instance, VkSurfaceKHR *surface);

    GLFWwindow *GetGLFWWindow();

    static void glfwInit();
    static void glfwTerminate();

private:
    GLFWwindow *pGLFWWindow_;
    int width_;
    int height_;
    Mouse mouse_;
};
