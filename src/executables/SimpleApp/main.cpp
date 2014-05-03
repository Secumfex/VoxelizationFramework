#include <Application/Application.h>

#include <iostream>

class ObjectLoadingApp : public Application
{
	void postInitialize()
	{
		DEBUGLOG->log("Loading some objects");
		DEBUGLOG->indent();
		
			DEBUGLOG->log("Loading cube dae file");
			DEBUGLOG->indent();
			std::vector< Object* > daeCube = m_resourceManager.loadObjectsFromFile(RESOURCES_PATH "/cube.dae");
			DEBUGLOG->outdent();

			DEBUGLOG->log("Loading cube obj file");
			DEBUGLOG->indent();
			std::vector< Object* > objCube =  m_resourceManager.loadObjectsFromFile( RESOURCES_PATH "/cube.obj" );
			DEBUGLOG->outdent();

		DEBUGLOG->log("Loading some objects complete");
		DEBUGLOG->outdent();

		DEBUGLOG->log("Adding objects to a scene instance");
		DEBUGLOG->indent();
			Scene* scene = new Scene();
			scene->addObjects( daeCube );
			scene->addObjects( objCube );
		DEBUGLOG->outdent();
		
		DEBUGLOG->log("Setting scene instance as active scene ");
		m_sceneManager.setActiveScene(scene);	
	}
};

int main(){

	ObjectLoadingApp myApp;

	myApp.configure();
	myApp.initialize();
	myApp.run();

	
	return 0;
}