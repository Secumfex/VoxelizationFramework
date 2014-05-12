#include "Application.h"

#include <sstream>

Application::Application()
{
	m_name = "Application";
	m_terminate = false;

	DEBUGLOG->setAutoPrint(true);

	DEBUGLOG->log( "Constructing Application" );

	DEBUGLOG->indent();
	DEBUGLOG->log( "Initializing GLFW libraries");

	if(!glfwInit()){
		DEBUGLOG->log ("ERROR : GLFW failed to initialize");
	}

	DEBUGLOG->log("Creating GLFW window");

	GLFWwindow* window = m_windowManager.createWindow(800,600);
	if(!window){
		DEBUGLOG->log ("ERROR : GLFW Window failed to initialize");
	}
	DEBUGLOG->log("Initializing GLEW libraries");

	glewInit();
	
	DEBUGLOG->indent();
		std::stringstream ss;
	// print out some info about the graphics drivers
		ss << "OpenGL version: " <<  glGetString(GL_VERSION);
		DEBUGLOG->log( ss.str() );
		ss.str(std::string());

		ss << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION);
		DEBUGLOG->log( ss.str());
		ss.str(std::string());

		ss << "Vendor: " << glGetString(GL_VENDOR);
		DEBUGLOG->log( ss.str());
		ss.str(std::string());

		ss << "Renderer: " << glGetString(GL_RENDERER);
		DEBUGLOG->log( ss.str());
		ss.str(std::string());
	DEBUGLOG->outdent();

	DEBUGLOG->log("Constructing Application complete");

	DEBUGLOG->outdent();
}


Application::~Application()
{
	DEBUGLOG->log("Terminating Application");
	DEBUGLOG->indent();

	DEBUGLOG->log("Destroying GLFW windows");
	m_windowManager.closeWindows();

	DEBUGLOG->log("Terminating GLFW");
	glfwTerminate();

	DEBUGLOG->log("Terminating Application complete");
	DEBUGLOG->outdent();
}


void Application::configure()
{
	DEBUGLOG->log("Configuring Application ");
	DEBUGLOG->indent();

	DEBUGLOG->log("Binding global Window Manager pointer");
	m_windowManager.bindAsGlobalWindowManager();

	DEBUGLOG->log("Binding global Input Manager pointer");
	m_inputManager.bindAsGlobalInputManager();

	DEBUGLOG->log("Binding GLFW Callbacks");
	GLFWwindow* window = m_windowManager.getActiveWindow();

	DEBUGLOG->indent();
	
		DEBUGLOG->log("Binding WindowShouldCloseCallback...");
		glfwSetWindowCloseCallback(window, WindowManager::windowCloseCallback);

		DEBUGLOG->log("Binding KeyCallback...");
		glfwSetKeyCallback(window, InputManager::keyCallback);

		DEBUGLOG->log("Binding MouseButtonCallback...");
		glfwSetMouseButtonCallback(window, InputManager::mouseButtonCallback);

	DEBUGLOG->outdent();

	DEBUGLOG->log("Binding GLFW window to RenderManager");
	m_renderManager.setActiveWindow(window);

	postConfigure();

	DEBUGLOG->log("Configuring Application complete");
	DEBUGLOG->outdent();
}

void Application::postConfigure()
{

}


void Application::initialize()
{
	DEBUGLOG->log("Initializing Application ");
	DEBUGLOG->indent();

	postInitialize();

	DEBUGLOG->log("Initializing Application complete ");
	DEBUGLOG->outdent();
}

void Application::postInitialize()
{
	
}

void Application::run()
{
	DEBUGLOG->log("Entering Application main loop ");
	DEBUGLOG->indent();
	Listener* terminateListener  = new InvertBooleanListener(&m_terminate);
	Listener* terminateListener2 = new InvertBooleanListener(&m_terminate);
	m_windowManager.attachListenerOnWindowClose(terminateListener);
	m_inputManager. attachListenerOnKeyPress(terminateListener2, GLFW_KEY_ESCAPE, GLFW_PRESS);

	while (!m_terminate)
	{
		m_sceneManager.update( 0.005f );	//TODO replace with actual frame time
		m_renderManager.render();

		glfwSwapBuffers(m_windowManager.getActiveWindow());
        glfwPollEvents();
	}

	DEBUGLOG->log("Quitting Application main loop ");
	DEBUGLOG->outdent();
}

const DebugLog& Application::getLog()
{
	return DEBUGLOG;
}

RenderManager& Application::getRenderManager()
{
	return m_renderManager;
}

InputManager& Application::getInputManager()
{
	return m_inputManager;
}

WindowManager& Application::getWindowManager()
{
	return m_windowManager;
}

ResourceManager& Application::getResourceManager()
{
	return m_resourceManager;
}

SceneManager& Application::getSceneManager()
{
	return m_sceneManager;
}
