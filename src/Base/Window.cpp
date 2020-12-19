#include "Base/Window.h"
#include "Base/Exception.h"
#include "Base/StringTool.h"

Window::Window() : pGLFWWindow_(nullptr), width_(0), height_(0)
{
}

Window::Window(std::string applicationName, int width, int height)
{
    Create(applicationName, width, height);
}

Window::Window(std::wstring applicationName, int width, int height)
{
    Create(applicationName, width, height);
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
    // glfwSetWindowUserPointer(pGLFWWindow_, application_ptr__);
    // glfwSetFramebufferSizeCallback(pGLFWWindow_, Window::FramebufferResizeCallback);
}

void Window::Create(std::wstring applicationName, int width, int height)
{
    Create(String::ConvertString(applicationName), width, height);
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
