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

	m_executionTime = 0.0;
	m_queryTime = false;
	m_startTime = 0;
	m_stopTime = 0;
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

	if ( m_queryTime )
	{
		startTime();
	}

	p_computeShader->dispatch(m_num_groups_x, m_num_groups_y, m_num_groups_z);

	if ( m_queryTime )
	{
		stopTime();
	}
}

int DispatchComputeShaderListener::getNumGroupsX() const {
	return m_num_groups_x;
}

void DispatchComputeShaderListener::setNumGroupsX(int numGroupsX) {
	m_num_groups_x = numGroupsX;
}

int DispatchComputeShaderListener::getNumGroupsY() const {
	return m_num_groups_y;
}

void DispatchComputeShaderListener::setNumGroupsY(int numGroupsY) {
	m_num_groups_y = numGroupsY;
}

int DispatchComputeShaderListener::getNumGroupsZ() const {
	return m_num_groups_z;
}

void DispatchComputeShaderListener::setNumGroupsZ(int numGroupsZ) {
	m_num_groups_z = numGroupsZ;
}

ComputeShader* DispatchComputeShaderListener::getComputeShader() {
	return p_computeShader;
}

GLuint64 DispatchComputeShaderListener::getStopTime() const {
	return m_stopTime;
}

double DispatchComputeShaderListener::getExecutionTime() const {
	return m_executionTime;
}

bool DispatchComputeShaderListener::isQueryTime() const {
	return m_queryTime;
}

void DispatchComputeShaderListener::startTime() {
	// generate OpenGL query objects
	glGenQueries(2, &m_queryID[0] );

	// request current time-stamp ( before dispatch )
	glQueryCounter(m_queryID[0], GL_TIMESTAMP);
}

void DispatchComputeShaderListener::stopTime() {
	// request current time-stamp ( after dispatch )
	glQueryCounter(m_queryID[1], GL_TIMESTAMP);

	// wait for query to become available ( when dispatch finished )
	unsigned int stopTimerAvailable = 0;
	while (!stopTimerAvailable )
	{
	    glGetQueryObjectuiv(m_queryID[1],
	    		GL_QUERY_RESULT_AVAILABLE,
	    		&stopTimerAvailable);
	}

	// retrieve query results
	glGetQueryObjectui64v(m_queryID[0], GL_QUERY_RESULT, &m_startTime);
	glGetQueryObjectui64v(m_queryID[1], GL_QUERY_RESULT, &m_stopTime);

	// compute execution time
	m_executionTime = (m_stopTime - m_startTime) / 1000000.0;
}

void DispatchComputeShaderListener::setQueryTime(bool queryTime) {
	m_queryTime = queryTime;
}

GLuint64 DispatchComputeShaderListener::getStartTime() const {
	return m_startTime;
}

void DispatchComputeShaderListener::setComputeShader(
		ComputeShader* computeShader) {
	p_computeShader = computeShader;
}

#include "Utility/UtilityListeners.h"

DebugPrintDoubleListener* DispatchComputeShaderListener::getPrintExecutionTimeListener(std::string label) {
	return new DebugPrintDoubleListener(&m_executionTime, "COMPUTE " + label + " execution time ( ms ) : ");
}
