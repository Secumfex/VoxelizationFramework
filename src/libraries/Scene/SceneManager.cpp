#include "SceneManager.h"

SceneManager::SceneManager()
{

}

SceneManager::~SceneManager()
{
	
}

void SceneManager::setActiveScene(Scene* scene)
{
	m_activeScene = scene;
}
Scene* SceneManager::getActiveScene()
{
	return m_activeScene;
}

void SceneManager::update(float d_t)
{
	if(m_activeScene)
	{
		m_activeScene->update(d_t);
	}
}
