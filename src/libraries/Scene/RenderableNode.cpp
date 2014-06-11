#include "Scene/RenderableNode.h"

RenderableNode::RenderableNode(Node* parent)
  : Node( parent )
{

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
