#ifndef UNIFORM_H
#define UNIFORM_H

#include <string>
#include <Rendering/Shader.h>

#include <Utility/DebugLog.h>

class Uploadable
{
public:
	virtual void uploadUniform(Shader* shader);
	virtual ~Uploadable();
};

/**
 * make sure there are only valid uniform types allowed...
 */
template< class T>
class Uniform : public Uploadable
{
protected:
	T* p_value;
	std::string m_name;
public:
	Uniform(std::string name, T* valuePtr) {
		p_value = valuePtr;
		m_name = name;
	}

	virtual ~Uniform() {
	}

	void uploadUniform(Shader* shader) {
		if (p_value != 0) {
			shader->uploadUniform(*p_value, m_name);
		}
	}
};

#endif
