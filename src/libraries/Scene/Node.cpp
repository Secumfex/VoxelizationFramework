#include <Scene/Node.h>

Node::Node(Node* parent)
{
	m_parent = parent;
	if (parent)
	{
		parent->addChild(this);
	}
}

Node::~Node()
{

}

void Node::addChild(Node* node)
{
	m_children.push_back(node);
}

std::vector<Node*> getChildren()
{
	return m_children;
}

Node* Node::getParent()
{
	return m_parent;
}

Node::getModelMatrix()
{
	return m_modelMatrix();
}

void Node::setModelMatrix(glm::mat4 modelMatrix)
{
	m_modelMatrix = modelMatrix;
}