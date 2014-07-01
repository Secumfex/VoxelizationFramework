#ifndef SHADER_H_
#define SHADER_H_

#include "Utility/ShaderTools.h"
#include "Utility\SubjectListenerPattern.h"
#include <string>
#include <vector>
#include <map>
#include <glm/gtc/type_ptr.hpp>


class Shader : public Subject{
protected:
	std::map<std::string, GLuint> m_uniformHandles;	
	std::vector<std::string> m_uniformNames;			

	GLuint m_programHandle;	

public:

	/** \brief constructor
	 *
	 * @param vertexShader
	 * @param fragmentShader
	 */
	Shader(std::string vertexShader, std::string fragmentShader);

	/** \brief destructor
	 *
	 */
	virtual ~Shader();

	/** \brief getter
	 *
	 * @return program handle
	 */
	GLuint getProgramHandle();


	/** \brief this uploads a single uniform to this shader program
	 *
	 * @param uniformMatrix
	 * @param uniformName
	 */
	bool uploadUniform(glm::mat4 uniformMatrix, std::string uniformName);

	/** \brief this uploads a single uniform to this shader program
	 *
	 * @param uniformVector
	 * @param uniformName
	 */
	bool uploadUniform(glm::vec3 uniformVector, std::string uniformName);

	/** \brief this uploads a single uniform to this shader program
	 *
	 * @param uniformVector
	 * @param uniformName
	 */
	bool uploadUniform(glm::vec4 uniformVector, std::string uniformName);

	/** \brief this uploads a single uniform to this shader program
	 *
	 * @param uniformVariable as GLfloat
	 * @param uniformName
	 */
	bool uploadUniform(GLfloat uniformVariable, std::string uniformName);

	/** \brief this uploads a single uniform to this shader program
	 *
	 * @param uniformVariable as GLuint
	 * @param uniformName
	 */
	bool uploadUniform(GLint uniformVariable, std::string uniformName);


	/** \brief checks on wheather the shader program owns a certain uniform variable
	 *
	 * @param uniformName
	 * @return true or false
	 */
	bool hasUniform(std::string uniformName);

	/** \brief getter
	 *
	 *@return vector of uniform names
	 */
	std::vector<std::string> getUniformNames();


	/** \brief binds the shader program
	 *
	 */
	void useProgram();
};

class ComputeShader : public Subject{
private:
	void makeShaderProgram(std::string vert, std::string frag);
protected:

	GLuint m_programHandle;

	int m_num_groups_x;
	int m_num_groups_y;
	int m_num_groups_z;
public:

	/** \brief constructor
	 *
	 * @param computeShader source path
	 */
	ComputeShader(std::string computeShader);

	/** \brief destructor
	 *
	 */
	virtual ~ComputeShader();

	/** \brief getter
	 *
	 * @return program handle
	 */
	GLuint getProgramHandle();

	/** \brief binds the shader program
	 *
	 */
	void useProgram();

	/** \brief dispatch shader program
	 * dispatch this shaderProgram using the specified groups
	 * @param num_groups_x
	 * @param num_groups_y
	 * @param num_groups_z
	 */
	void dispatch( GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z );

	/** \brief dispatch shader program indirectly
	 *
	 * @param indirect byte offset to the buffer currently bound to GL_DISPATCH_INDIRECT_BUFFER
	 */
	void dispatchIndirect( GLintptr indirect );

	void setProgramHandle(GLuint programHandle);
	int getNumGroupsX() const;
	void setNumGroupsX(int numGroupsX);
	int getNumGroupsY() const;
	void setNumGroupsY(int numGroupsY);
	int getNumGroupsZ() const;
	void setNumGroupsZ(int numGroupsZ);
};

#endif
