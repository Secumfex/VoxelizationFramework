#ifndef CAMERANODE_H
#define CAMERANODE_H

#include <Scene/Camera.h>
#include <Scene/Node.h>

#include <glm/gtc/matrix_transform.hpp>

class CameraNode : public Node, public Camera
{
	public:
	CameraNode( Node* parent = 0);
	virtual ~CameraNode();

	virtual void setModelMatrix(glm::mat4 modelMatrix);
	virtual void translate(glm::vec3 translate);
	virtual void scale(glm::vec3 scale);
	virtual void rotate(float angle, glm::vec3 axis);
	virtual void multiply(glm::mat4 transform);

	virtual void update(float d_t = 0.1f);
	virtual void updatePosition( float deltaTime );

	virtual void setPosition(float x, float y, float z);
	virtual void setPosition(glm::vec3 newPos);

	virtual void setDirection(glm::vec3 dir);

	void updateCameraFromNode();

	glm::mat4 getViewMatrix();
};

#endif
