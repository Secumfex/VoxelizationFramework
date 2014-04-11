#ifndef RESOURCE_H_
#define RESOURCE_H_

#include <string>

class Resource {
protected:
	std::string m_path;
public:
	Resource();
	virtual ~Resource();
	void setPath(std::string path);
	std::string getPath();
};

#endif
