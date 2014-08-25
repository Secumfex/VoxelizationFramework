#ifndef INPUTFIELD_H
#define INPUTFIELD_H

#include <Input/InputManager.h>
#include <Utility/SubjectListenerPattern.h>
#include <glm/glm.hpp>

// class that resembles a field on screen that can be clicked and hovered
class InputField : public Subject
{
protected:
	bool m_hovered; // currently hovered
	bool m_pressed; // currently pressed

	int m_xPos;	// x window coordinate
	int m_yPos;	// y window coordinate

	int m_width;	// pixel width
	int m_height; // pixel height
public:
	InputField(int x, int y, int width, int height, InputManager* inputManager = 0, int button = GLFW_MOUSE_BUTTON_LEFT);
	virtual ~InputField();

	virtual void press();
	virtual void hover();
	virtual void release();

	void createMouseListeners( InputManager* inputManager, int button );

	void attachListenerOnHover( Listener* listener);
	void attachListenerOnPress( Listener* listener);
	void attachListenerOnRelease( Listener* listener);

	int getHeight() const;
	void setHeight(int height);
	int getWidth() const;
	void setWidth(int width);
	int getXPos() const;
	void setXPos(int xPos);
	int getYPos() const;
	void setYPos(int yPos);
	bool isHovered() const;
	void setHovered(bool hovered);
	bool isPressed() const;
	void setPressed(bool pressed);

	glm::vec3 getCenter();
};

// listener that calls hover() when called
class InputFieldHoveredListener : public Listener
{
private:
	InputField* p_inputField;
	InputManager* p_inputManager;
public:
	InputFieldHoveredListener(InputField* inputField, InputManager* inputManager);
	virtual bool testHovered();
	virtual void call();
};

// listener that calls press() when called
class InputFieldPressedListener : public Listener
{
private:
	InputField* p_inputField;
	InputManager* p_inputManager;
public:
	InputFieldPressedListener(InputField* inputField, InputManager* inputManager);
	virtual void call();
};

// listener that calls release() when called
class InputFieldReleasedListener : public Listener
{
private:
	InputField* p_inputField;
	InputManager* p_inputManager;
public:
	InputFieldReleasedListener(InputField* inputField, InputManager* inputManager);
	virtual void call();
};
#endif
