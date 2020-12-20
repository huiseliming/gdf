#include "Base/Window.h"
#include "Base/Exception.h"
#include "Base/StringTool.h"

Window::Window() : pGLFWWindow_(nullptr), width_(0), height_(0), resized_(false)
{
}

Window::~Window()
{
    Destroy();
}

void Window::Create(const std::string &applicationName, const int width, const int height)
{
    // exist window check
    if (pGLFWWindow_ != nullptr)
        return;
    width_ = width;
    height_ = height;
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    pGLFWWindow_ = glfwCreateWindow(width_, height_, applicationName.c_str(), nullptr, nullptr);
    glfwSetWindowUserPointer(pGLFWWindow_, this);
    // mouse callback
    glfwSetCursorPosCallback(pGLFWWindow_, Window::CursorPosCallback);
    glfwSetCursorEnterCallback(pGLFWWindow_, Window::CursorEntryCallback);
    glfwSetScrollCallback(pGLFWWindow_, Window::ScrollCallback);
    glfwSetMouseButtonCallback(pGLFWWindow_, Window::MouseButtonCallback);
    // keyboard callback
    glfwSetKeyCallback(pGLFWWindow_, Window::KeyCallback);
    glfwSetCharCallback(pGLFWWindow_, Window::CharCallback);
    // window callback
    glfwSetWindowSizeCallback(pGLFWWindow_, Window::WindowSizeCallback);
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
    preProcessing();
    glfwPollEvents();
    postProcessing();
}

bool Window::ShouldClose()
{
    return glfwWindowShouldClose(pGLFWWindow_);
}

void Window::CreateWindowSurface(const VkInstance instance, VkSurfaceKHR *surface)
{
    if (glfwCreateWindowSurface(instance, pGLFWWindow_, nullptr, surface))
        THROW_EXCEPT("Failed to create window surface");
}

bool Window::GetRequiredInstanceExtensions(std::vector<std::string>& glfwRequiredInstanceExtensions)
{
    if (!glfwVulkanSupported())
        return false;
    uint32_t extensionCount;
    const char **extensionNames = glfwGetRequiredInstanceExtensions(&extensionCount);
    for (uint32_t i = 0; i < extensionCount; i++)
        glfwRequiredInstanceExtensions.push_back(extensionNames[i]);
    return true;
}

void Window::preProcessing()
{
    mouse_.preProcessing(*this);
    keyboard_.preProcessing(*this);
    resized_ = false;
}

void Window::postProcessing()
{
    mouse_.postProcessing(*this);
    keyboard_.postProcessing(*this);
}

void Window::CursorPosCallback(GLFWwindow *window, double xpos, double ypos)
{
    Window *pWindow = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
    pWindow->mouse_.position = {xpos, ypos};
}

void Window::CursorEntryCallback(GLFWwindow *window, int entered)
{
    Window *pWindow = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
    if (entered == GLFW_TRUE)
        pWindow->mouse_.inWindow = true;
    else
        pWindow->mouse_.inWindow = false;
}

void Window::ScrollCallback(GLFWwindow *window, double xoffset, double yoffset)
{
    Window *pWindow = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
    pWindow->mouse_.offset = {xoffset, yoffset};
}

void Window::MouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
{
    Window *pWindow = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
    if (action == GLFW_PRESS) {
        pWindow->mouse_.pressedButtons.insert(button);
        pWindow->mouse_.heldButtons.insert(button);
    } else if (action == GLFW_RELEASE) {
        pWindow->mouse_.releasedButtons.insert(button);
        pWindow->mouse_.heldButtons.erase(button);
    }
}

void Window::KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    Window *pWindow = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
    if (action == GLFW_PRESS) {
        pWindow->keyboard_.pressedKeys.insert(key);
        pWindow->keyboard_.heldKeys.insert(key);
    } else if (action == GLFW_REPEAT) {
        pWindow->keyboard_.repeatKey = key;
        pWindow->keyboard_.heldKeys.insert(key);
    } else if (action == GLFW_RELEASE) {
        pWindow->keyboard_.releasedKeys.insert(key);
        pWindow->keyboard_.heldKeys.erase(key);
    }
}

void Window::CharCallback(GLFWwindow *window, unsigned int codepoint)
{
    Window *pWindow = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
    pWindow->keyboard_.unicodePoint = codepoint;
}

void Window::WindowSizeCallback(GLFWwindow *window, int width, int height)
{
    Window *pWindow = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
    pWindow->resized_ = true;
}

void Window::FramebufferResizeCallback(GLFWwindow *window, int width, int height)
{
    // auto application_ptr = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
    // application_ptr->get_render_engine_ptr()->Resize();
}