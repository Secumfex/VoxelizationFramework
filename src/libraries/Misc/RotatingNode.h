#ifndef ROTATINGNODE_H
#define ROTATINGNODE_H

class RotatingNode : public Updatable, public RenderableNode
{
	private:
	glm::vec3 m_rotationAxis;
	float m_angle;

	public:
	RotatingNode(Node* parent = 0)
	{
		setParent(parent);
		m_angle = 0.0f;
	}
	void update(float d_t = 0.1f)
	{
		rotate( glm::rotate (glm::mat4(1.0f), m_angle, m_rotationAxis ) );
	}
	void setRotationAxis(glm::vec3 rotationAxis)
	{
		m_rotationAxis = rotationAxis;
	}
	void setAngle(float angle)
	{
		m_angle = angle;
	}

};

#endif
