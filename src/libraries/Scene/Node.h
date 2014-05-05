#ifndef NODE_H
#define NODE_H

#include <glm/glm.hpp>
#include <vector>
#include <Resources/Object.h>

class Node
{
protected:
	Node* m_parent;
	std::vector< Node* > m_children;
	glm::mat4 m_modelMatrix;
	Object* m_object;
public:
	Node(Node* parent = 0);
	~Node();

	void setParent(Node* parent);
	Node* getParent();
	void addChild(Node* node);

	void setObject(Object* object);

	std::vector< Node* > getChildren();
	glm::mat4 getModelMatrix();
	glm::mat4 getAccumulatedModelMatrix();
	void setModelMatrix(glm::mat4 modelMatrix);

	void translate(glm::mat4 translate);
	void scale(glm::mat4 scale);
	void rotate(glm::mat4 rotate);

	Node* findObjectNode(Object* object);
};
#endif
