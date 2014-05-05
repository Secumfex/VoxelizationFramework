#include "Scene/RenderableNode.h"

RenderableNode::RenderableNode(Node* parent) {
	m_parent = parent;
	if (parent)
	{
		parent->addChild(this);
	}

	m_object = 0;
}

RenderableNode::~RenderableNode() {
}

void RenderableNode::render()
{
	if(m_object)
	{
		glBindVertexArray(m_object->getModel()->getVAOHandle());
		glDrawElements(GL_TRIANGLES, m_object->getModel()->getNumIndices(), GL_UNSIGNED_INT, 0);
	}
}

#include <Rendering/Shader.h>

void RenderableNode::uploadUniforms(Shader* shader)
{
	if (shader)
	{
		shader->uploadUniform(m_modelMatrix, "uniformModel");
	}
}
