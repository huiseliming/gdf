#include "Renderer/Window.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <codecvt>
#include <locale>
Window::Window()
{
}

Window::Window(std::string applicationName, int width, int height)
{
	Create(applicationName, width, width);
}

Window::Window(std::wstring applicationName, int width, int height)
{
	Create(applicationName, width, width);
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
	std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(applicationName);
	Create(applicationName, width, width);
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

void Window::FramebufferResizeCallback(GLFWwindow* window, int width, int height)
{
	//auto application_ptr = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
	//application_ptr->get_render_engine_ptr()->Resize();
}

GLFWwindow* Window::GetGLFWWindow() { return m_pGLFWWindow; }

bool Window::ShouldClose() 
{
	return glfwWindowShouldClose(m_pGLFWWindow);
}