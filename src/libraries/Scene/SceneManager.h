#ifndef SCENEMANAGER_H
#define SCENEMANAGER_H

#include "Scene/Scene.h"
#include "Utility/Updatable.h"

class SceneManager : public Updatable
{
protected:
	Scene* m_activeScene;
public:
	SceneManager();
	~SceneManager();

	void setActiveScene(Scene* scene);
	Scene* getActiveScene();

	void update(float d_t = 0.1f);
};

#endif
