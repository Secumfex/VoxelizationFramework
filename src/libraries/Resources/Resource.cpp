#include "Resources/Resource.h"

Resource::Resource() {
	// TODO Auto-generated constructor stub

}

Resource::~Resource() {
	// TODO Auto-generated destructor stub
}

void Resource::setPath(std::string path)
{
	m_path = path;
}

std::string Resource::getPath()
{
	return m_path;
}

