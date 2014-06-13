#ifndef APPLICATION_H
#define APPLICATION_H

#include "Utility/DebugLog.h"
#include "Rendering/RenderManager.h"
#include "Input/InputManager.h"
#include "Resources/ResourceManager.h"
#include "Scene/SceneManager.h"
#include "WindowManager.h"
#include "Utility/UtilityListeners.h"
#include "Utility/Timer.h"
#include "Utility/SubjectListenerPattern.h"

class Application : public Subject{
protected:
	std::string m_name;

	/*Functionality*/
	InputManager 	m_inputManager;
	RenderManager 	m_renderManager;
	ResourceManager m_resourceManager;
	SceneManager 	m_sceneManager;
	WindowManager 	m_windowManager;

	/*Utility*/
	bool 			m_terminate;
	GLFWTimer		m_cycleTimer;
public:
	Application();
	~Application();

	/*Public Methods*/
	void configure();
	virtual void postConfigure();

	void initialize();
	virtual void postInitialize();

	void run();

	/*Getter & Setter*/
	const DebugLog& getLog();

	RenderManager& getRenderManager();
	ResourceManager& getResourceManager();
	SceneManager& getSceneManager();
	WindowManager& getWindowManager();
	InputManager& getInputManager();
};

#endif
