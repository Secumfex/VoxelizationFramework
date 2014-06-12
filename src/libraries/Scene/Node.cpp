#include <Scene/Node.h>

#include <glm/gtc/matrix_transform.hpp>

Node::Node(Node* parent)
{
	m_parent = parent;
	if (parent)
	{
		parent->addChild(this);
	}

	m_object = 0;
	m_children.clear();
}

Node::~Node()
{

}

void Node::addChild(Node* node)
{
	m_children.push_back(node);
}

std::vector<Node*> Node::getChildren()
{
	return m_children;
}

Node* Node::getParent()
{
	return m_parent;
}

glm::mat4 Node::getModelMatrix()
{
	return m_modelMatrix;
}

glm::mat4 Node::getAccumulatedModelMatrix()
{
	if ( m_parent && m_parent != this)
	{
		return m_parent->getAccumulatedModelMatrix() * m_modelMatrix;
	}
	else
	{
		return m_modelMatrix;
	}
}

void Node::setModelMatrix(glm::mat4 modelMatrix)
{
	m_modelMatrix = modelMatrix;
}

void Node::multiply(glm::mat4 transform)
{
	m_modelMatrix = transform * m_modelMatrix;
}
void Node::translate(glm::vec3 translate)
{
	m_modelMatrix = glm::translate( glm::mat4(1.0f), translate) * m_modelMatrix;
}
void Node::scale(glm::vec3 scale)
{
	m_modelMatrix = glm::scale( glm::mat4(1.0f), scale ) * m_modelMatrix;
}
void Node::rotate(float angle, glm::vec3 axis)
{
	m_modelMatrix = glm::rotate( glm::mat4(1.0f), angle, axis ) * m_modelMatrix;
}

void Node::setParent(Node* parent)
{
	m_parent = parent;
	if (parent)
	{
		parent->addChild(this);
	}
}

Object* Node::getObject() {
	return m_object;
}

Node* Node::findObjectNode(Object* object)
{
	if (m_object == object)	// this is the Node
	{
		return this;
	}
	else					// might be at a child node
	{
		for (unsigned int i = 0; i < m_children.size(); i++)
		{
			Node* foundNode = m_children[i]->findObjectNode( object );

			if ( foundNode != 0 ){
				return foundNode;
			}
		}
	}

	return 0;	// neither this, nor child Node
}

void Node::setObject(Object* object)
{
	m_object = object;
}

