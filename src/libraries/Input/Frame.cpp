#include "Frame.h"

#include <Scene/Node.h>
#include <Scene/RenderableNode.h>

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

// returns a pair consisting of the input field and a position node with a scale node attached and both of which have a reference to the object
std::pair< InputField*, std::pair < Node*, RenderableNode* > > Frame::createButton( int offsetX, int offsetY, int width, int height, InputManager* inputManager, int button, ResourceManager* resourceManager, Texture* texture)
{
	// create display object
	Node* dPosNode = new Node();
	RenderableNode* dScaleNode = new RenderableNode( dPosNode );
	if ( resourceManager )
	{
		Object* dObject = new Object( *( resourceManager->getQuad() ) );
		Material* dMaterial = new Material( *dObject->getMaterial() );
		dMaterial->setAttribute( "uniformHasTexture", 1.0f );
		dMaterial->setAttribute( "uniforTextureTransparency", 0.0f );

		if ( texture )
		{
			dMaterial->setTexture( "uniformTexture", texture );
		}

		dObject->setMaterial( dMaterial );
		dPosNode->setObject( dObject );
		dScaleNode->setObject( dObject );
	}

	// create input field
	InputField* inputField = createInputField( offsetX, offsetY, width, height, inputManager, button );

	// place Node center and scale according to input field
	alignNodeWithInputFieldCenter( inputField, dPosNode );
	alignNodeWithInputFieldSize( inputField, dScaleNode );

	return std::pair< InputField*, std::pair < Node*, RenderableNode* > > ( inputField, std::pair<Node*, RenderableNode* >( dPosNode, dScaleNode) );
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
