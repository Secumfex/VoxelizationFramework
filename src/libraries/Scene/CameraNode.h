#ifndef CAMERANODE_H
#define CAMERANODE_H

#include <Scene/Camera.h>
#include <Scene/Node.h>

#include <glm/gtc/matrix_transform.hpp>

class CameraNode : public Node, public Updatable
{
	private:
	Camera* m_camera;

	public:
	CameraNode( Camera* camera , Node* parent = 0);
	virtual ~CameraNode();

	virtual void update(float d_t = 0.1f);

	Camera* getCamera();
	void setCamera(Camera* camera);
};

#endif
