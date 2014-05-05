#ifndef SHADER_H_
#define SHADER_H_

#include "Utility/ShaderTools.h"
#include "Utility\SubjectListenerPattern.h"
#include <string>
#include <vector>
#include <map>
#include <glm/gtc/type_ptr.hpp>


class Shader : public Subject{
private:
	void makeShaderProgram(std::string vert, std::string frag); 
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

#endif
