#include "Base/Window.h"
#include "Base/StringTool.h"
#include "Base/Exception.h"

Window::Window()
    : m_pGLFWWindow(nullptr)
    , m_width(0)
    , m_height(0)
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
    m_width = width;
    m_height = height;
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    m_pGLFWWindow = glfwCreateWindow(m_width, m_height, applicationName.c_str(), nullptr, nullptr);
    //glfwSetWindowUserPointer(m_pGLFWWindow, application_ptr__);
    //glfwSetFramebufferSizeCallback(m_pGLFWWindow, Window::FramebufferResizeCallback);
}

void Window::Create(std::wstring applicationName, int width, int height)
{
    Create(String::ConvertString(applicationName), width, height);
}

void Window::Destroy()
{
    if (m_pGLFWWindow != nullptr) {
        glfwDestroyWindow(m_pGLFWWindow);
        m_pGLFWWindow = nullptr;
    }
}

void Window::PollEvents()
{
    glfwPollEvents();
}

bool Window::ShouldClose() 
{
    return glfwWindowShouldClose(m_pGLFWWindow);
}


void Window::FramebufferResizeCallback(GLFWwindow* window, int width, int height)
{
    //auto application_ptr = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
    //application_ptr->get_render_engine_ptr()->Resize();
}

void Window::CreateWindowSurface(VkInstance instance, VkSurfaceKHR* surface)
{
    if (glfwCreateWindowSurface(instance, m_pGLFWWindow, nullptr, surface))
        THROW_EXCEPT("Failed to create window surface");
}

GLFWwindow* Window::GetGLFWWindow() { return m_pGLFWWindow; }

    
void Window::glfwInit()
{
    ::glfwInit();
}

void Window::glfwTerminate()
{
    ::glfwTerminate();
}
    
