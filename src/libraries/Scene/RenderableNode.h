/*
 * RenderableNode.h
 *
 *  Created on: 05.05.2014
 *      Author: Arend
 */

#ifndef RENDERABLENODE_H_
#define RENDERABLENODE_H_

#include <Rendering/Renderable.h>
#include <Scene/Node.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

class Shader;

class RenderableNode: public Renderable, public Node {
public:
	virtual ~RenderableNode();
	RenderableNode(Node* parent = 0);

	void render();
	void uploadUniforms(Shader* shader);
};

#endif
