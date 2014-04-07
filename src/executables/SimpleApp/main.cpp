#include <Application/Application.h>

#include <iostream>

int main(){

	Application myApp;

	myApp.configure();
	myApp.initialize();
	myApp.run();
	
	return 0;
}