#ifndef INPUTMANAGER_H
#define INPUTMANAGER_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Utility/SubjectListenerPattern.h"

class InputManager : public Subject{
protected:
	double m_cursorX;
	double m_cursorY;

	double m_cursorDiffX;
	double m_cursorDiffY;
public:
	InputManager();
	~InputManager();
	
	void bindAsGlobalInputManager();

	/* Listener Interfaces */
	void attachListenerOnKeyPress(Listener* listener, int key, int action = GLFW_PRESS);
	void attachListenerOnMouseButtonPress(Listener* listener, int button, int action = GLFW_PRESS);
	void attachListenerOnCursorPos(Listener* listener);	// called everytime the mouse moves

	/* Callback Functions */
	static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	void key(GLFWwindow* window, int key, int scancode, int action, int mods);

	static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
	void mouseButton(GLFWwindow* window, int button, int action, int mods);

	static void cursorPosCallback(GLFWwindow* window, double x, double y);
	void cursorPos(GLFWwindow* window, double x, double y);

	double getCursorDiffY() const;
	void setCursorDiffY(double cursorDiffY);
	double getCursorDiffX() const;
	void setCursorDiffX(double cursorDiffX);
	double getCursorX() const;
	void setCursorX(double cursorX);
	double getCursorY() const;
	void setCursorY(double cursorY);
};

static InputManager* global_im = 0;

#endif
