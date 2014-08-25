#include "Frame.h"

#include <Scene/Node.h>

Frame::Frame(int xPos, int yPos, int width, int height)
{
	m_xPos = xPos;
	m_yPos = yPos;
	m_width = width;
	m_height = height;
}

Frame::~Frame()
{

}

InputField* Frame::createInputField( int offsetX, int offsetY, int width, int height, InputManager* inputManager, int button )
{
	InputField* result = new InputField( m_xPos + offsetX, m_yPos + offsetY, width, height, inputManager, button);
	m_inputFields.push_back(result);

	return result;
}

glm::vec3 Frame::getRelativeCenter(InputField* inputField) {
	// pixel Pos
	glm::vec3 result = inputField->getCenter();

	// pixel Pos relative to upper left pixel of frame
	result.x -= m_xPos;
	result.y -= m_yPos;

	// relative position in frame in relation to upper left corner
	result.x /= m_width;
	result.y /= m_height;

	// relative positon in frame in relation to lower left corner
	result.y = 1.0f - result.y;

	return result;
}

glm::vec3 Frame::getRelativeSize(InputField* inputField) {
	// pixel size
	glm::vec3 result = glm::vec3( inputField->getWidth(), inputField->getHeight(), 1.0f);

	// size in relation to frame size
	result.x = result.x / m_width;
	result.y = result.y / m_height;

	return result;
}

void Frame::alignNodeWithInputFieldCenter(InputField* inputField, Node* node) {
	node->translate( getRelativeCenter(inputField) );
}

void Frame::alignNodeWithInputFieldSize(InputField* inputField, Node* node) {
	node->scale( getRelativeSize(inputField) );
}
