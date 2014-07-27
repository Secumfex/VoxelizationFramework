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

class DebugPrintDoubleListener;

// will dispatch a compute shader when called
class DispatchComputeShaderListener : public Listener
{
protected:
	bool m_queryTime;
	unsigned int m_queryID[2];
	GLuint64 m_startTime;
	GLuint64 m_stopTime;
	double m_executionTime;

	void startTime();
	void stopTime();

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
	int getNumGroupsX() const;
	void setNumGroupsX(int numGroupsX);
	int getNumGroupsY() const;
	void setNumGroupsY(int numGroupsY);
	int getNumGroupsZ() const;
	void setNumGroupsZ(int numGroupsZ);
	ComputeShader* getComputeShader();
	void setComputeShader(ComputeShader* computeShader);

	GLuint64 getStartTime() const;
	GLuint64 getStopTime() const;
	double getExecutionTime() const;
	bool isQueryTime() const;
	void setQueryTime(bool queryTime);

	DebugPrintDoubleListener* getPrintExecutionTimeListener(std::string label);
};

#endif
