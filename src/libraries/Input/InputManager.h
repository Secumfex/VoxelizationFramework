#ifndef INPUTMANAGER_H
#define INPUTMANAGER_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>

class InputManager {
protected:
public:
	InputManager();
	~InputManager();
	
	void bindAsGlobalInputManager();

	static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	void key(GLFWwindow* window, int key, int scancode, int action, int mods);

	static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
	void mouseButton(GLFWwindow* window, int button, int action, int mods);

};

static InputManager* global_im = 0;

#endif