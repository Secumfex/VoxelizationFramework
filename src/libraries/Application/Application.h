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

	/*Static Configuration*/
	static int 	static_newWindowWidth;
	static int 	static_newWindowHeight;
	static bool static_autoPrint;
	static int  static_fpsRefreshFrames;

	Application();
	virtual ~Application();

	/*Public Methods*/
	void configure();
	virtual void postConfigure();

	void initialize();
	virtual void postInitialize();

	void run();

	virtual void programCycle();

	/*Getter & Setter*/
	const DebugLog& getLog();

	RenderManager& getRenderManager();
	ResourceManager& getResourceManager();
	SceneManager& getSceneManager();
	WindowManager& getWindowManager();
	InputManager& getInputManager();
};

#endif
