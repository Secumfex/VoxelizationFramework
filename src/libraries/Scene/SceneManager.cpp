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