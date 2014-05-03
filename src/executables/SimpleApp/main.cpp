#include <Application/Application.h>

#include <iostream>

class ObjectLoadingApp : public Application
{
	void postInitialize()
	{
		DEBUGLOG->log("Loading some objects");
		DEBUGLOG->indent();
		
		DEBUGLOG->log("Loading Cube dae file");
		DEBUGLOG->indent();
		std::vector< Object* > someObjects = m_resourceManager.loadObjectsFromFile(RESOURCES_PATH "/cube.dae");
		DEBUGLOG->outdent();

		DEBUGLOG->log("Loading Cube obj file");
		DEBUGLOG->indent();
		someObjects =  m_resourceManager.loadObjectsFromFile( RESOURCES_PATH "/cube.obj" );
		DEBUGLOG->outdent();

		DEBUGLOG->log("Loading some objects complete");
		DEBUGLOG->outdent();
	}
};

int main(){

	ObjectLoadingApp myApp;

	myApp.configure();
	myApp.initialize();
	myApp.run();
	
	return 0;
}