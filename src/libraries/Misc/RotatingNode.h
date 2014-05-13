#ifndef ROTATINGNODE_H
#define ROTATINGNODE_H

#include <Utility/Updatable.h>
#include <Scene/RenderableNode.h>

class RotatingNode : public Updatable, public RenderableNode
{
	private:
	glm::vec3 m_rotationAxis;
	float m_angle;

	public:
	RotatingNode(Node* parent = 0);
	void update(float d_t = 0.1f);
	void setRotationAxis(glm::vec3 rotationAxis);
	void setAngle(float angle);

};

#endif
