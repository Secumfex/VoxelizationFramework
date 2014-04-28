#ifndef SCENEGRAPH_H
#define SCENEGRAPH_H

#include "Scene/Node.h"

class SceneGraph
{
private:
	Node m_rootNode;
public:
	SceneGraph();
	~SceneGraph();
};

#endif