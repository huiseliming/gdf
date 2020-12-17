#pragma once
#include <string>
struct GLFWwindow;

class Window
{
public:
	Window();
	explicit Window(std::string applicationName, int width = 800, int height = 600);
	explicit Window(std::wstring applicationName, int width = 800, int height = 600);
	virtual ~Window();

	void Create(std::string applicationName = "Application", int width = 800, int height = 600);
	void Create(std::wstring applicationName = L"Application", int width = 800, int height = 600);
	void Destroy();

	void PollEvents();
	bool ShouldClose();


	static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);

	GLFWwindow* GetGLFWWindow();

private:
	GLFWwindow* m_pGLFWWindow;
	int m_width;
	int m_height;
};
