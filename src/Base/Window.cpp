#include "Base/Window.h"
#include "Base/Exception.h"
#include "Base/StringTool.h"

Window::Window() : pGLFWWindow_(nullptr), width_(0), height_(0)
{
}

Window::~Window()
{
    Destroy();
}

void Window::Create(std::string applicationName, int width, int height)
{
    width_ = width;
    height_ = height;
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    pGLFWWindow_ = glfwCreateWindow(width_, height_, applicationName.c_str(), nullptr, nullptr);
    glfwSetWindowUserPointer(pGLFWWindow_, this);
    glfwSetCursorPosCallback(pGLFWWindow_, Window::CursorPosCallback);
    glfwSetScrollCallback(pGLFWWindow_, Window::ScrollCallback);
    glfwSetMouseButtonCallback(pGLFWWindow_, Window::MouseButtonCallback);
    // glfwSetFramebufferSizeCallback(pGLFWWindow_, Window::FramebufferResizeCallback);
}

void Window::Destroy()
{
    if (pGLFWWindow_ != nullptr) {
        glfwDestroyWindow(pGLFWWindow_);
        pGLFWWindow_ = nullptr;
    }
}

void Window::PollEvents()
{
    glfwPollEvents();
}

bool Window::ShouldClose()
{
    return glfwWindowShouldClose(pGLFWWindow_);
}

void Window::CursorPosCallback(GLFWwindow *window, double xpos, double ypos)
{
    Window *pWindow = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
    pWindow->mouse_.position = {xpos, ypos};
}

void Window::ScrollCallback(GLFWwindow *window, double xoffset, double yoffset)
{
    Window *pWindow = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
    pWindow->mouse_.offset = {xoffset, yoffset};
}

void Window::MouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
{
    Window *pWindow = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
}

void Window::FramebufferResizeCallback(GLFWwindow *window, int width, int height)
{
    // auto application_ptr = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
    // application_ptr->get_render_engine_ptr()->Resize();
}

void Window::CreateWindowSurface(VkInstance instance, VkSurfaceKHR *surface)
{
    if (glfwCreateWindowSurface(instance, pGLFWWindow_, nullptr, surface))
        THROW_EXCEPT("Failed to create window surface");
}

GLFWwindow *Window::GetGLFWWindow()
{
    return pGLFWWindow_;
}

void Window::glfwInit()
{
    ::glfwInit();
}

void Window::glfwTerminate()
{
    ::glfwTerminate();
}
