#include "Application.h"

Application::Application()
{
	m_name = "Application";
	m_terminate = false;

	m_log.setAutoPrint(true);
	m_log.log( "Application starting" );
}


Application::~Application()
{

}


void Application::configure()
{

}


void Application::initialize()
{

}


void Application::run()
{

}

const DebugLog& Application::getLog()
{
	return &m_log;
}