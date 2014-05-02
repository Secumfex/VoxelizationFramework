#ifndef OBJECT_H
#define OBJECT_H

#include <Resources/Model.h>
#include <Resources/Material.h>

class Object
{
private:
	Model* m_model;
	Material* m_material;
public:
	Object( Model* model = 0, Material* material = 0);
	~Object();

	void setMaterial( Material* material );
	void setModel( Model* model );

	Model* getModel();
	Material* getMaterial();
};

#endif