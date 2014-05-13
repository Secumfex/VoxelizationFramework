#include "Turntable.h"

#include <glm/gtc/matrix_transform.hpp>

Turntable::Turntable(Node* node, InputManager* inputManager)
{
	p_node = node;
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
	p_node->rotate(glm::rotate(glm::mat4(1.0f), phi   * m_sensitivity ,glm::vec3(0.0f,1.0f,0.0f)));
	p_node->rotate(glm::rotate(glm::mat4(1.0f), theta * m_sensitivity ,glm::vec3(1.0f,0.0f,0.0f)));
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
