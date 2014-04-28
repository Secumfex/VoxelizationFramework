#ifndef INPUTMANAGER_H
#define INPUTMANAGER_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Utility/SubjectListenerPattern.h"

class InputManager : public Subject{
protected:
public:
	InputManager();
	~InputManager();
	
	void bindAsGlobalInputManager();

	/* Listener Interfaces */
	void attachListenerOnKeyPress(Listener* listener, int key, int action = GLFW_PRESS);
	void attachListenerOnMouseButtonPress(Listener* listener, int button, int action = GLFW_PRESS);

	/* Callback Functions */
	static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	void key(GLFWwindow* window, int key, int scancode, int action, int mods);

	static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
	void mouseButton(GLFWwindow* window, int button, int action, int mods);

};

static InputManager* global_im = 0;

#endif