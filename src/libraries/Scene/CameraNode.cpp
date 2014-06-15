#include "CameraNode.h"

CameraNode::CameraNode(Camera* camera, Node* parent)
  : Node( parent )
{
	m_camera = camera;
	if( m_camera )
	{
		translate( m_camera->getPosition() );
	}
}


CameraNode::~CameraNode()
{
}

void CameraNode::update(float d_t)
{
	// m_camera->update(d_t);
	if (m_camera)
	{
		// read Camera state (speed)
		float speedRight = m_camera->getSpeedRight();
		float speedForward = m_camera->getSpeedForward();

		// move node accordingly ( locally )
		setModelMatrix(glm::translate( m_modelMatrix, glm::vec3 ( 1.0f, 0.0f, 0.0f) * speedRight * d_t) );
		setModelMatrix(glm::translate( m_modelMatrix, glm::vec3 ( 0.0f, 0.0f, -1.0f) * speedForward * d_t) );

		// write new world position into Camera
		glm::vec3 newPosition = glm::vec3( getAccumulatedModelMatrix() * glm::vec4( 0.0f, 0.0f ,0.0f ,1.0f) );
		m_camera->setPosition( newPosition );
	}
}

void CameraNode::setCamera(Camera* camera)
{
	m_camera = camera;
}

Camera* CameraNode::getCamera()
{
	return m_camera;
}
