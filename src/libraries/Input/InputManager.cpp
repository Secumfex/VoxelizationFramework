#include "InputManager.h"

#include <sstream>

InputManager::InputManager()
{
	m_cursorX = 0.0;
	m_cursorY = 0.0;
	m_cursorDiffX = 0.0;
	m_cursorDiffY = 0.0;
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

double InputManager::getCursorDiffY() const {
	return m_cursorDiffY;
}

double InputManager::getCursorDiffX() const {
	return m_cursorDiffX;
}

void InputManager::setCursorDiffX(double cursorDiffX) {
	m_cursorDiffX = cursorDiffX;
}

double InputManager::getCursorX() const {
	return m_cursorX;
}

void InputManager::setCursorX(double cursorX) {
	m_cursorX = cursorX;
}

double InputManager::getCursorY() const {
	return m_cursorY;
}

void InputManager::setCursorY(double cursorY) {
	m_cursorY = cursorY;
}

void InputManager::setCursorDiffY(double cursorDiffY) {
	m_cursorDiffY = cursorDiffY;
}

void InputManager::cursorPos(GLFWwindow* window, double x, double y)
{
	m_cursorDiffX = x - m_cursorX;
	m_cursorDiffY = y - m_cursorY;

	m_cursorX = x;
	m_cursorY = y;

	call("CURSORPOS");
}

void InputManager::cursorPosCallback(GLFWwindow* window, double x, double y)
{
	if(global_im)
		{
			global_im->cursorPos(window, x, y);
		}
}

void InputManager::attachListenerOnKeyPress(Listener* listener, int key, int action)
{
	std::stringstream ss;
	ss << key << "_KEY_" << action;
	attach(listener, ss.str() );
}

void InputManager::attachListenerOnCursorPos(Listener* listener)
{
	attach(listener, "CURSORPOS");
}

void InputManager::attachListenerOnMouseButtonPress(Listener* listener, int button, int action)
{
	std::stringstream ss;
	ss << button << "_BUTTON_" << action;
	attach(listener, ss.str() );
}
