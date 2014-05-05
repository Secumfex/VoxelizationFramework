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

void Node::translate(glm::mat4 translate)
{
	m_modelMatrix = translate * m_modelMatrix;
}
void Node::scale(glm::mat4 scale)
{
	m_modelMatrix = scale * m_modelMatrix;
}
void Node::rotate(glm::mat4 rotate)
{
	m_modelMatrix = rotate * m_modelMatrix;
}

void Node::setParent(Node* parent)
{
	m_parent = parent;
	if (parent)
	{
		parent->addChild(this);
	}
}

Node* Node::findObjectNode(Object* object)
{
	if (m_object == object)
	{
		return this;
	}
	else
	{
		for (unsigned int i = 0; 0 < m_children.size(); i++)
		{
			if ( m_children[i]->findObjectNode( object ) )
			{
				return m_children[i];
			}
		}
	}
	return 0;
}

void Node::setObject(Object* object)
{
	m_object = object;
}

