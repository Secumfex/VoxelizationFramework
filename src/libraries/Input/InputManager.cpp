#include "InputManager.h"

#include <sstream>

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
	std::stringstream ss;
	ss << key << "_KEY_" << action;
	call( ss.str() );
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
	std::stringstream ss;
	ss << button << "_BUTTON_" << action;
	call (ss.str());
}

void InputManager::attachListenerOnKeyPress(Listener* listener, int key, int action)
{
	std::stringstream ss;
	ss << key << "_KEY_" << action;
	attach(listener, ss.str() );
}

void InputManager::attachListenerOnMouseButtonPress(Listener* listener, int button, int action)
{
	std::stringstream ss;
	ss << button << "_BUTTON_" << action;
	attach(listener, ss.str() );
}