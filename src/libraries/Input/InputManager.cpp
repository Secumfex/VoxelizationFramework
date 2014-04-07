#include "InputManager.h"

InputManager::InputManager()
{

}

InputManager::~InputManager()
{

}

void InputManager::bindAsGlobalInputManager()
{
	global_im = this;
}

void InputManager::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if(global_im)
	{
		global_im->key(window, key, scancode, action, mods);
	}
}
	

void InputManager::key(GLFWwindow* window, int key, int scancode, int action, int mods)
{

}

void InputManager::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	if(global_im)
	{
		global_im->mouseButton(window, button, action, mods);
	}
}

void InputManager::mouseButton(GLFWwindow* window, int button, int action, int mods)
{

}