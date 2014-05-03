#include <Scene/SceneGraph.h>

SceneGraph::SceneGraph()
{

}

SceneGraph::~SceneGraph()
{

}

Node* SceneGraph::findObjectNode( Object* object )
{
	return m_rootNode.findObjectNode( object );
}

Node* SceneGraph::getRootNode()
{
	return &m_rootNode;
}