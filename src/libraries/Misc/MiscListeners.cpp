#include <Misc/MiscListeners.h>

#include <Utility/DebugLog.h>

SetCameraSpeedListener::SetCameraSpeedListener(Camera* camera,
		Direction direction, float speed) {
	p_camera = camera;
	m_direction = direction;
	m_speed = speed;
}

SetCameraSpeedListener::~SetCameraSpeedListener() {
}

void SetCameraSpeedListener::call() {
	if (m_direction == FORWARD) {
		p_camera->setSpeedForward(m_speed);
	}
	if (m_direction == RIGHT) {
		p_camera->setSpeedRight(m_speed);
	}
}

DispatchComputeShaderListener::DispatchComputeShaderListener(
		ComputeShader* shader, int num_groups_x, int num_groups_y, int num_groups_z) {
	p_computeShader = shader;
	m_num_groups_x = num_groups_x;
	m_num_groups_y = num_groups_y;
	m_num_groups_z = num_groups_z;
}

DispatchComputeShaderListener::~DispatchComputeShaderListener() {
}

int* DispatchComputeShaderListener::getNumGroupsXPointer() {
	return &m_num_groups_x;
}

int* DispatchComputeShaderListener::getNumGroupsYPointer() {
	return &m_num_groups_y;
}

int* DispatchComputeShaderListener::getNumGroupsZPointer() {
	return &m_num_groups_z;
}

void DispatchComputeShaderListener::call() {
	p_computeShader->useProgram();
	p_computeShader->dispatch(m_num_groups_x, m_num_groups_y, m_num_groups_z);
}
