#include <Misc/RotatingNode.h>

#include <glm/gtc/matrix_transform.hpp>

RotatingNode::RotatingNode(Node* parent)
{
	setParent(parent);
	m_angle = 0.0f;

	m_object = 0;
}
void RotatingNode::update(float d_t)
{
	rotate( glm::rotate (glm::mat4(1.0f), m_angle, m_rotationAxis ) );
}
void RotatingNode::setRotationAxis(glm::vec3 rotationAxis)
{
	m_rotationAxis = rotationAxis;
}
void RotatingNode::setAngle(float angle)
{
	m_angle = angle;
}
