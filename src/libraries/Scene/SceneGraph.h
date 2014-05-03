#ifndef SCENEGRAPH_H
#define SCENEGRAPH_H

#include "Scene/Node.h"

class Object;

class SceneGraph
{
private:
	Node m_rootNode;
public:
	SceneGraph();
	~SceneGraph();

	Node* getRootNode();
	Node* findObjectNode( Object* object);
};

#endif