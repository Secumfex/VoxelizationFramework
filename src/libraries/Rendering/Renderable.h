#ifndef RENDERABLE_H
#define RENDERABLE_H

class Shader;

class Renderable
{
public:
	virtual void render() = 0;
	virtual void uploadUniforms(Shader* shader) = 0;
};

#endif
