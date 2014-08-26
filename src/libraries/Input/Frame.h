#ifndef FRAME_H
#define FRAME_H

#include <Input/InputManager.h>
#include <Input/InputField.h>
#include <Resources/ResourceManager.h>

#include <list>

class Node;
class RenderableNode;

class Frame
{
private:
	int m_xPos;	// pixelPos of upperLeftCorner
	int m_yPos;	// pixelPos of upperLeftCorner
	int m_width;// pixelWidth
	int m_height;//pixelHeight

	std::list< InputField* > m_inputFields;

public:
	Frame(int xPos = 0, int yPos = 0, int width = 0, int height = 0);
	virtual ~Frame();

	InputField* createInputField( int offsetX, int offsetY, int width, int height, InputManager* inputManager = 0, int button = GLFW_MOUSE_BUTTON_LEFT );
	std::pair< InputField*, std::pair< Node*, RenderableNode* > > createButton( int offsetX, int offsetY, int width, int height, InputManager* inputManager = 0, int button = GLFW_MOUSE_BUTTON_LEFT, ResourceManager* resourceManager = 0, Texture* texture = 0);

	glm::vec3 getRelativeCenter( InputField* inputField ); // returns the position relative to the top left corner of the frame
	glm::vec3 getRelativeSize( InputField* inputField );   // returns the size relative to the size of of the frame

	void alignNodeWithInputFieldCenter( InputField* inputField, Node* node );	// align with center position, relative to BOTTOM left corner
	void alignNodeWithInputFieldSize( InputField* inputField, Node* node );     // align with size, relative to field size
};

#endif
