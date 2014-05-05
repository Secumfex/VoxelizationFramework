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
		m_object->render();
	}
}

#include <Rendering/Shader.h>

void RenderableNode::uploadUniforms(Shader* shader)
{
	if (shader)
	{
		shader->uploadUniform( getAccumulatedModelMatrix() , "uniformModel");
		if (m_object)
		{
			m_object->uploadUniforms(shader);
		}
	}
}
