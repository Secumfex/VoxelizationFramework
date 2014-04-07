#include "Application.h"

#include <sstream>

Application::Application()
{
	m_name = "Application";
	m_terminate = false;

	m_log.setAutoPrint(true);

	m_log.log( "Constructing Application" );

	m_log.indent();
	m_log.log( "Initializing GLFW libraries");

	if(!glfwInit()){
		m_log.log ("ERROR : GLFW failed to initialize");
	}

	m_log.log("Creating GLFW window");

	GLFWwindow* window = m_windowManager.createWindow(800,600);
	if(!window){
		m_log.log ("ERROR : GLFW Window failed to initialize");
	}
	m_log.log("Initializing GLEW libraries");

	glewInit();
	
	m_log.indent();
		std::stringstream ss;
	// print out some info about the graphics drivers
		ss << "OpenGL version: " <<  glGetString(GL_VERSION);
		m_log.log( ss.str() );
		ss.str(std::string());

		ss << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION);
		m_log.log( ss.str());
		ss.str(std::string());

		ss << "Vendor: " << glGetString(GL_VENDOR);
		m_log.log( ss.str());
		ss.str(std::string());

		ss << "Renderer: " << glGetString(GL_RENDERER);
		m_log.log( ss.str());
		ss.str(std::string());
	m_log.outdent();

	m_log.log("Constructing Application complete");

	m_log.outdent();
}


Application::~Application()
{
	m_log.log("Terminating Application");
	m_log.indent();

	m_log.log("Destroying GLFW windows");
	m_windowManager.closeWindows();

	m_log.log("Terminating GLFW");
	glfwTerminate();

	m_log.log("Terminating Application complete");
	m_log.outdent();
}


void Application::configure()
{
	m_log.log("Configuring Application ");
	m_log.indent();

	m_log.log("Binding global Window Manager pointer");
	m_windowManager.bindAsGlobalWindowManager();

	m_log.log("Binding global Input Manager pointer");
	m_inputManager.bindAsGlobalInputManager();

	m_log.log("Binding GLFW Callbacks");
	GLFWwindow* window = m_windowManager.getActiveWindow();

	m_log.indent();
	
		m_log.log("Binding WindowShouldCloseCallback...");
		glfwSetWindowCloseCallback(window, WindowManager::windowCloseCallback);

		m_log.log("Binding KeyCallback...");
		glfwSetKeyCallback(window, InputManager::keyCallback);

		m_log.log("Binding MouseButtonCallback...");
		glfwSetMouseButtonCallback(window, InputManager::mouseButtonCallback);

	m_log.outdent();

	m_log.log("Configuring Application complete");
	m_log.outdent();
}


void Application::initialize()
{

}


void Application::run()
{
	m_log.log("Entering Application main loop ");
	Listener* terminateListener = new InvertBooleanListener(&m_terminate);
	m_windowManager.attachListenerOnWindowClose(terminateListener);

	while (!m_terminate)
	{
		glfwSwapBuffers(m_windowManager.getActiveWindow());
        glfwPollEvents();
	}
}

const DebugLog& Application::getLog()
{
	return &m_log;
}