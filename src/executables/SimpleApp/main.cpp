#include <Application/Application.h>

#include <iostream>

class ObjectLoadingApp : public Application
{
	void postInitialize()
	{
		DEBUGLOG->log("Loading some objects...");
	}
};

int main(){

	ObjectLoadingApp myApp;

	myApp.configure();
	myApp.initialize();
	myApp.run();
	
	return 0;
}