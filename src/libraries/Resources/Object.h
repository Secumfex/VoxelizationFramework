#ifndef OBJECT_H
#define OBJECT_H

#include <Resources/Model.h>
#include <Resources/Material.h>
#include <Rendering/Renderable.h>

class Object : public Renderable
{
protected:
	Model* m_model;
	Material* m_material;
public:
	Object( Model* model = 0, Material* material = 0);
	~Object();

	void setMaterial( Material* material );
	void setModel( Model* model );

	Model* getModel();
	Material* getMaterial();

	virtual void render();
	virtual void uploadUniforms(Shader* shader);
};

#endif
