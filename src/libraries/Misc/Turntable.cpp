#include "Turntable.h"

Turntable::Turntable(Node* node, InputManager* inputManager, Camera* camera)
{
	p_node = node;
	p_cam = camera;
	p_inputManager = inputManager;
	m_dragActive = false;
	m_sensitivity = 0.01f;
}

Turntable::~Turntable()
{

}

void Turntable::update(float d_t)
{
	if(m_dragActive && p_inputManager)
	{
		dragBy((float) p_inputManager->getCursorDiffX(), (float) p_inputManager->getCursorDiffY());
	}
}

void Turntable::setDragActive(bool drag)
{
	m_dragActive = drag;
}

bool Turntable::getDragActive()
{
	return m_dragActive;
}

void Turntable::setSensitivity(float sensitivity)
{
	m_sensitivity = sensitivity;
}

void Turntable::setNode(Node* node)
{
	p_node = node;
}

void Turntable::setInputManager(InputManager* inputManager)
{
	p_inputManager = inputManager;
}

void Turntable::dragBy(float phi, float theta)
{
	// first: rotate "turn vector" as proposed by view matrix
	glm::mat4 transformMatrix = glm::mat4(1.0f);
	if ( p_cam )
	{
			 transformMatrix = glm::inverse( p_cam->getViewMatrix() );
			 transformMatrix = glm::inverse( p_cam->getViewMatrix() );
	}

	glm::vec3 yRotation = glm::vec3 ( transformMatrix * glm::vec4 ( 0.0, 1.0, 0.0, 0.0) );
	glm::vec3 xRotation = glm::vec3 ( transformMatrix * glm::vec4 ( 1.0, 0.0, 0.0, 0.0) );

	p_node->rotate( phi   * m_sensitivity , yRotation );
	p_node->rotate( theta * m_sensitivity , xRotation );
}

Turntable::ToggleTurntableDragListener::ToggleTurntableDragListener(Turntable* turntable)
{
	p_turntable = turntable;
}

void Turntable::ToggleTurntableDragListener::call()
{
	if (p_turntable && p_turntable->getDragActive())
	{
		p_turntable->setDragActive(false);
	}
	else{
		p_turntable->setDragActive(true);
	}
}
