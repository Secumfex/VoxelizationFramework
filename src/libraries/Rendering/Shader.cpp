#include "Rendering/Shader.h"

Shader::Shader(std::string vertexShader, std::string fragmentShader)
{
	m_programHandle = ShaderTools::makeShaderProgram(vertexShader.c_str(), fragmentShader.c_str());

	int total = -1;

	glGetProgramiv( m_programHandle, GL_ACTIVE_UNIFORMS, &total );

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
