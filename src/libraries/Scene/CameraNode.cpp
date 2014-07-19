#include "CameraNode.h"

#include "Utility/DebugLog.h"

CameraNode::CameraNode( Node* parent )
  : Node( parent )
  , Camera()
{

}


CameraNode::~CameraNode()
{
}

void CameraNode::updatePosition( float deltaTime )
{
	// compute translation
	glm::vec3 translation = getSpeedRight() * deltaTime * glm::vec3 (m_modelMatrix *  glm::vec4(1.0f, 0.0f, 0.0f, 0.0f) );
	translation += getSpeedForward() * deltaTime * glm::vec3 ( m_modelMatrix * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f) );

	translate(translation);
}

void CameraNode::update(float d_t)
{
	// update position as Camera would do
	updatePosition(d_t);
}

void CameraNode::updateCameraFromNode()
{
	Camera::setPosition( glm::vec3 ( getAccumulatedModelMatrix() * glm::vec4(0,0,0,1) ) );
	Camera::setDirection(glm::vec3 ( getAccumulatedModelMatrix() * glm::vec4(0.0f,0.0f,-1.0f,0.0f) ) );
}

void CameraNode::multiply(glm::mat4 transform)
{
	Node::multiply(transform);

	updateCameraFromNode();
}

void CameraNode::translate( glm::vec3 translate )
{
	// translate Node
	Node::translate( translate );

	// update camera position
	Camera::setPosition( Camera::getPosition() + translate );
}

void CameraNode::scale( glm::vec3 scale )
{
	// DO NOTHING
	DEBUGLOG->log("ERROR: cannot scale a CameraNode");
}

void CameraNode::rotate( float angle, glm::vec3 axis)
{
	// rotate Node as usual
	Node::rotate( angle, axis);

	// update view direction
	glm::vec4 newDirection = glm::transpose ( glm::inverse( Node::getAccumulatedModelMatrix() ) ) * glm::vec4(0.0f,0.0f,-1.0f,0.0f);
	Camera::setDirection(glm::vec3 ( newDirection) );
}

void CameraNode::setModelMatrix(glm::mat4 modelMatrix)
{
	Node::setModelMatrix(modelMatrix);

	Camera::setPosition( glm::vec3 ( getAccumulatedModelMatrix() * glm::vec4(0,0,0,1) ) );
	Camera::setDirection( glm::vec3 ( glm::transpose(glm::inverse ( getAccumulatedModelMatrix() ) ) * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f) ) );

}

void CameraNode::setPosition(float x ,float y,float z)
{
	// update camera position as usual
	Camera::setPosition(x,y,z);

	// compute local offset to parent node
	if ( m_parent )
	{
		glm::vec3 parentPosition = glm::vec3 (m_parent->getAccumulatedModelMatrix() * glm::vec4( 0.0f, 0.0f, 0.0f, 1.0f) );
		glm::vec3 delta = Camera::getPosition() - parentPosition;
		setModelMatrix( glm::translate( glm::mat4(1.0f) , delta ) );
	}
	else
	{
		setModelMatrix(glm::translate(glm::mat4(1.0f), glm::vec3(x,y,z)));
	}
}

void CameraNode::setPosition( glm::vec3 newPos )
{
	// update camera position as usual
	Camera::setPosition(newPos);

	// compute local offset to parent node
	if ( m_parent )
	{
		glm::vec3 parentPosition = glm::vec3 (m_parent->getAccumulatedModelMatrix() * glm::vec4( 0.0f, 0.0f, 0.0f, 1.0f) );
		glm::vec3 delta = Camera::getPosition() - parentPosition;
		setModelMatrix( glm::translate( glm::mat4(1.0f) , delta ) );
	}
	else
	{
		setModelMatrix(glm::translate(glm::mat4(1.0f), glm::vec3(newPos)));
	}
}

void CameraNode::setDirection( glm::vec3 dir )
{
	// update camera direction as usual
	Camera::setDirection( dir );

	// compute local offset to parent node TODO
	DEBUGLOG->log("ERROR : updating model matrix direction is not yet implemented ... ");
}

glm::mat4 CameraNode::getViewMatrix()
{
	// update everything first
	updateCameraFromNode();

	// then compute as usual
	return Camera::getViewMatrix();
}
