#ifndef NODE_H
#define NODE_H

#include <glm/glm.hpp>
#include <vector>

class Node
{
private:
	Node* m_parent;
	std::vector< Node* > m_children;
	glm::mat4 m_modelMatrix;
public:
	Node();
	~Node();
};

#endif