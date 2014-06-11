#include <Misc/MiscListeners.h>

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

