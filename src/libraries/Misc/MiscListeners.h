#ifndef MISCLISTENERS_H
#define MISCLISTENERS_H

#include <Utility/SubjectListenerPattern.h>
#include <Scene/Camera.h>

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

#endif
