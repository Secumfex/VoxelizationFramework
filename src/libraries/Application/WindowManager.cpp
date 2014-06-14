#include "WindowManager.h"

WindowManager::WindowManager()
{
	m_activeWindow = 0;
}

WindowManager::~WindowManager()
{
	closeWindows();
}

void WindowManager::closeWindows()
{
	for (unsigned int i = 0; i < m_windows.size(); i++)
	{
		glfwDestroyWindow(m_windows[i]);
	}
	m_windows.clear();
}

GLFWwindow* WindowManager::createWindow(int width, int height )
{
	GLFWwindow* new_window = glfwCreateWindow(width, height, " ", NULL, NULL);

	if(!new_window)
	{
		glfwTerminate();
	}

	m_windows.push_back(new_window);
	setActiveWindow(new_window);

	return new_window;
}

std::vector< GLFWwindow* > WindowManager::getWindows()
{
	return m_windows;
}


void WindowManager::setSize	(GLFWwindow* window, int width, int height)
{
	glfwSetWindowSize(window, width, height);
}

void WindowManager::setWidth (GLFWwindow* window, int width)
{
	int old_width, old_height;
	glfwGetWindowSize(window, &old_width, &old_height);
	glfwSetWindowSize(window, width, old_height);
}

void WindowManager::setHeight(GLFWwindow* window, int height)
{
	int old_width, old_height;
	glfwGetWindowSize(window, &old_width, &old_height);
	glfwSetWindowSize(window, old_width, height);
}

void WindowManager::setActiveWindow(GLFWwindow* window)
{
	glfwMakeContextCurrent(window);
	m_activeWindow = window;
}

GLFWwindow* WindowManager::getActiveWindow()
{
	return m_activeWindow;
}

void WindowManager::windowCloseCallback(GLFWwindow* window)
{
	if(global_wm)
	{
		global_wm->windowClose(window);
	}
}

void WindowManager::windowClose(GLFWwindow* window)
{
	destroyWindow(window);
	call("WINDOW_CLOSE");
}

void WindowManager::attachListenerOnWindowClose(Listener* listener)
{
	attach(listener, "WINDOW_CLOSE");
}

void WindowManager::bindAsGlobalWindowManager()
{
	global_wm = this;
}

void WindowManager::destroyWindow(GLFWwindow* window)
{
	for (unsigned int i = 0; i < m_windows.size(); i++)
	{
		if (m_windows[i] == window)
		{
			glfwDestroyWindow(window);
			m_windows.erase(m_windows.begin()+i);
			if (m_activeWindow == window)
			{
				m_activeWindow = 0;
			}
			break;
		}
	}
}