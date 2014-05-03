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
	Node(Node* parent = 0);
	~Node();

	Node* getParent();
	void addChild(Node* node);

	std::vector< Node* > getChildren();
	glm::mat4 getModelMatrix();
	void setModelMatrix(glm::mat4 modelMatrix);
};

#endif