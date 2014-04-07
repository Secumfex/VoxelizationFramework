#ifndef WINDOWMANAGER_H
#define WINDOWMANAGER_H


#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Utility/SubjectListenerPattern.h"

#include <vector>

class WindowManager : public Subject
{
private:
	std::vector< GLFWwindow* > m_windows;
	GLFWwindow* m_activeWindow;

public:
	WindowManager();
	~WindowManager();

	/* Methods */
	GLFWwindow* createWindow(int width, int height);
	std::vector< GLFWwindow* > getWindows();
	void setSize  (GLFWwindow* window, int width, int height);
	void setWidth (GLFWwindow* window, int width);
	void setHeight(GLFWwindow* window, int height);

	void setActiveWindow(GLFWwindow* window);
	GLFWwindow* getActiveWindow();
	void closeWindows();
	
	void destroyWindow(GLFWwindow* window);

	/* Listener Interfaces */
	void attachListenerOnWindowClose(Listener* listener);
	
	/*	Utility	*/
	void bindAsGlobalWindowManager();
	
	static void windowCloseCallback(GLFWwindow* window);
	void windowClose(GLFWwindow* window);
};

static WindowManager* global_wm = 0;

#endif