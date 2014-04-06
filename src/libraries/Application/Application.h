#ifndef APPLICATION_H
#define APPLICATION_H

#include "Utility/DebugLog.h"
#include "Rendering/RenderManager.h"
#include "Input/InputManager.h"
#include "Resources/ResourceManager.h"
#include "Scene/SceneManager.h"

class Application {
protected:
	std::string m_name;

	/*Functionality*/
	InputManager 	m_inputManager;
	RenderManager 	m_renderManager;
	ResourceManager m_resourceManager;
	SceneManager 	m_sceneManager;

	/*Utility*/
	DebugLog 		m_log;
	bool 			m_terminate;
public:
	Application();
	~Application();

	/*Public Methods*/
	void configure();
	void initialize();
	void run();	

	/*Getter & Setter*/
	const DebugLog& getLog();
};

#endif