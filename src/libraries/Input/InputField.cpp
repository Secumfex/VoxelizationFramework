#include "InputField.h"

int InputField::getHeight() const {
	return m_height;
}

void InputField::setHeight(int height) {
	m_height = height;
}

int InputField::getWidth() const {
	return m_width;
}

void InputField::setWidth(int width) {
	m_width = width;
}

int InputField::getXPos() const {
	return m_xPos;
}

void InputField::setXPos(int xPos) {
	m_xPos = xPos;
}

int InputField::getYPos() const {
	return m_yPos;
}

InputField::InputField(int x, int y, int width, int height,
		InputManager* inputManager, int button) {
	m_xPos = x;
	m_yPos = y;
	m_width = width;
	m_height = height;

	if ( inputManager )
	{
		createMouseListeners(inputManager, button);
	}
}

InputField::~InputField() {
	// TODO delete listeners associated with this
}

void InputField::press() {
	call("PRESS");
}

void InputField::hover() {
	call("HOVER");
}

void InputField::release() {
	call("RELEASE");
}

void InputField::createMouseListeners( InputManager* inputManager, int button ) {
	inputManager->attachListenerOnCursorPos( new InputFieldHoveredListener(this, inputManager) );
	inputManager->attachListenerOnMouseButtonPress( new InputFieldPressedListener(this, inputManager), button, GLFW_PRESS);
	inputManager->attachListenerOnMouseButtonPress( new InputFieldReleasedListener(this, inputManager), button, GLFW_RELEASE);
}

void InputField::setYPos(int yPos) {
	m_yPos = yPos;
}

InputFieldHoveredListener::InputFieldHoveredListener(InputField* inputField,
		InputManager* inputManager) {
	p_inputField = inputField;
	p_inputManager = inputManager;
}

void InputFieldHoveredListener::call() {
	if ( p_inputField && p_inputManager )
	{
		if ( p_inputManager->getCursorX() >= p_inputField->getXPos()
				&& p_inputManager->getCursorX() <= p_inputField->getXPos() + p_inputField->getWidth()
				&& p_inputManager->getCursorY() >= p_inputField->getYPos()
				&& p_inputManager->getCursorY() <= p_inputField->getYPos() + p_inputField->getHeight())
		{
			p_inputField->setHovered(true);
			p_inputField->hover();
		}
		else
		{
			p_inputField->setHovered(false);
		}
	}

}

InputFieldPressedListener::InputFieldPressedListener(InputField* inputField,
		InputManager* inputManager) {
	p_inputField = inputField;
	p_inputManager = inputManager;
}

void InputFieldPressedListener::call() {
	if ( p_inputField )
	{
		if ( p_inputField->isHovered() )
		{
			p_inputField->setPressed( true );
			p_inputField->press();
		}
		else
		{
			p_inputField->setPressed( false );
		}
	}
}

InputFieldReleasedListener::InputFieldReleasedListener(InputField* inputField,
		InputManager* inputManager) {
	p_inputField = inputField;
	p_inputManager = inputManager;
}

void InputFieldReleasedListener::call() {
	if ( p_inputField )
	{
		if ( p_inputField->isPressed() )
		{
			p_inputField->release();
			p_inputField->setPressed(false);
		}
	}
}

bool InputField::isHovered() const {
	return m_hovered;
}

bool InputField::isPressed() const {
	return m_pressed;
}

void InputField::attachListenerOnHover(Listener* listener) {
	attach(listener, "HOVER");
}

void InputField::attachListenerOnPress(Listener* listener) {
	attach(listener, "PRESS");
}

void InputField::attachListenerOnRelease(Listener* listener) {
	attach(listener, "RELEASE");
}

void InputField::setPressed(bool pressed) {
	m_pressed = pressed;
}

void InputField::setHovered(bool hovered) {
	m_hovered = hovered;
}
