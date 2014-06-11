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
	SetCameraSpeedListener(Camera* camera, Direction direction, float speed)
	{
		p_camera = camera;
		m_direction = direction;
		m_speed = speed;
	}
	virtual ~SetCameraSpeedListener(){}

	void call()
	{
		if (m_direction == FORWARD)
		{
			p_camera->setSpeedForward(m_speed);
		}
		if (m_direction == RIGHT)
		{
			p_camera->setSpeedRight(m_speed);
		}
	}
};
