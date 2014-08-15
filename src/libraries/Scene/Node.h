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
	virtual ~Node();

	void setParent(Node* parent);
	Node* getParent();
	void addChild(Node* node);
	void removeChild( Node* node );

	void setObject(Object* object);
	Object* getObject();

	std::vector< Node* > getChildren();
	glm::mat4 getModelMatrix();
	glm::mat4* getModelMatrixPtr();
	glm::mat4 getAccumulatedModelMatrix();
	virtual void setModelMatrix(glm::mat4 modelMatrix);

	virtual void multiply(glm::mat4 transform);	// multiplies the transform matrix from left
	virtual void translate(glm::vec3 translate);
	virtual void scale(glm::vec3 scale);
	virtual void rotate(float angle, glm::vec3 axis);

	Node* findObjectNode(Object* object);
};
#endif
