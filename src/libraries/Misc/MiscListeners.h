#ifndef MISCLISTENERS_H
#define MISCLISTENERS_H

#include <Utility/SubjectListenerPattern.h>
#include <Scene/Camera.h>
#include <Rendering/Shader.h>

class SetCameraSpeedListener : public Listener
{
public:
	enum Direction {FORWARD, RIGHT};
protected:
	Camera* p_camera;
	float m_speed;
	Direction m_direction;
	public:
	SetCameraSpeedListener(Camera* camera, Direction direction, float speed);
	virtual ~SetCameraSpeedListener();

	void call();
};

// will dispatch a compute shader when called
class DispatchComputeShaderListener : public Listener
{
protected:
	ComputeShader* p_computeShader;
	int m_num_groups_x;
	int m_num_groups_y;
	int m_num_groups_z;
public:
	DispatchComputeShaderListener(ComputeShader* shader, int num_groups_x = 0, int num_groups_y = 0, int num_groups_z = 0);
	virtual ~DispatchComputeShaderListener();

	int* getNumGroupsXPointer();
	int* getNumGroupsYPointer();
	int* getNumGroupsZPointer();

	virtual void call();
};

#endif
