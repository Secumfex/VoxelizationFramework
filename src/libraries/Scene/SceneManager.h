#ifndef SCENEMANAGER_H
#define SCENEMANAGER_H

#include "Scene/Scene.h"

class SceneManager
{
protected:
	Scene* m_activeScene;
public:
	SceneManager();
	~SceneManager();

	void setActiveScene(Scene* scene);
	Scene* getActiveScene();
};

#endif