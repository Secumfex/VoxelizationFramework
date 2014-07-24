#include "Rendering/Shader.h"

#include "Utility/DebugLog.h"
Shader::Shader()
{
	m_programHandle = 0;
}

Shader::Shader(std::string vertexShader, std::string fragmentShader)
{
	m_programHandle = ShaderTools::makeShaderProgram(vertexShader.c_str(), fragmentShader.c_str());

	requestUniformNames();
}

void Shader::requestUniformNames()
{
	int total = -1;

	glGetProgramiv( m_programHandle, GL_ACTIVE_UNIFORMS, &total );

	DEBUGLOG->log("Shader ProgramHandle : ", m_programHandle);
	DEBUGLOG->log("Uniforms in this Shader: ", total);

	int i= 0;
	for(i=0; i<total; ++i)  {
		int name_len=-1, num=-1;
		GLenum type = GL_ZERO;
		char name[100];
		glGetActiveUniform( m_programHandle, GLuint(i), sizeof(name)-1,
				&name_len, &num, &type, name );
		name[name_len] = 0;
		GLuint location = glGetUniformLocation( m_programHandle, name );


		m_uniformHandles.insert(std::pair<std::string, GLuint>(name, location));

		m_uniformNames.push_back(name);
	}
}

Shader::~Shader()
{
	glDeleteProgram(m_programHandle);
}

GLuint Shader::getProgramHandle()
{
	return m_programHandle;
}

void Shader::setProgramHandle(GLuint handle)
{
	m_programHandle = handle;
}

void Shader::useProgram()
{
	glUseProgram(m_programHandle);
}

bool Shader :: hasUniform(std::string uniformName){
	return ( m_uniformHandles.find(uniformName) != m_uniformHandles.end() );
}

bool Shader :: uploadUniform(glm::mat4 uniformMatrix, std::string uniformName){

	if(m_uniformHandles.find(uniformName)!=m_uniformHandles.end()){
		glUniformMatrix4fv(m_uniformHandles[uniformName], 1, GL_FALSE, glm::value_ptr(uniformMatrix));
		return true;
	}else
		return false;
}

bool Shader :: uploadUniform(glm::vec3 uniformVector, std::string uniformName){
	if(m_uniformHandles.find(uniformName)!=m_uniformHandles.end()){
		glUniform3f(m_uniformHandles[uniformName], uniformVector.x, uniformVector.y, uniformVector.z);
		return true;
	}else
		return false;
}

bool Shader :: uploadUniform(glm::vec4 uniformVector, std::string uniformName){
	if(m_uniformHandles.find(uniformName)!=m_uniformHandles.end()){
		glUniform3f(m_uniformHandles[uniformName], uniformVector.x, uniformVector.y, uniformVector.z);
		return true;
	}else
		return false;
}

bool Shader::uploadUniform(GLfloat uniformVariable, std::string uniformName){
	if(m_uniformHandles.find(uniformName)!=m_uniformHandles.end()){
		glUniform1f(m_uniformHandles[uniformName], uniformVariable);
		return true;
	}else
		return false;
}
bool Shader::uploadUniform(GLint uniformVariable, std::string uniformName){
	if(m_uniformHandles.find(uniformName)!=m_uniformHandles.end()){
		glUniform1i(m_uniformHandles[uniformName], uniformVariable);
		return true;
	}else
		return false;
}

std::vector<std::string> Shader::getUniformNames()
{
	return m_uniformNames;
}

ComputeShader::ComputeShader(std::string computeShader) {
	m_programHandle = ShaderTools::makeComputeShaderProgram( computeShader.c_str() );

	GLint* params = new GLint[3];

	glGetProgramiv( m_programHandle, GL_COMPUTE_WORK_GROUP_SIZE, params );

	m_local_group_size_x = params[0];
	m_local_group_size_y = params[1];
	m_local_group_size_z = params[2];

	DEBUGLOG->log("ComputeShader local_group_size_x : ", m_local_group_size_x );
	DEBUGLOG->log("ComputeShader local_group_size_y : ", m_local_group_size_y );
	DEBUGLOG->log("ComputeShader local_group_size_z : ", m_local_group_size_z );

	requestUniformNames();
}

ComputeShader::~ComputeShader() {
}

int ComputeShader::getLocalGroupSizeX() const {
	return m_local_group_size_x;
}

int ComputeShader::getLocalGroupSizeY() const {
	return m_local_group_size_y;
}

int ComputeShader::getLocalGroupSizeZ() const {
	return m_local_group_size_z;
}

void ComputeShader::dispatch(GLuint num_groups_x, GLuint num_groups_y,
		GLuint num_groups_z) {
	glDispatchCompute( num_groups_x, num_groups_y, num_groups_z);
}

const std::map<std::string, GLuint>& Shader::getUniformHandles() const {
	return m_uniformHandles;
}
