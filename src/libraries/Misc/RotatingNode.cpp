#include <Misc/RotatingNode.h>

RotatingNode::RotatingNode(Node* parent = 0)
{
	setParent(parent);
	m_angle = 0.0f;
}
void RotatingNode::update(float d_t = 0.1f)
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
