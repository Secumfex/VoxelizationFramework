#include <Application/Application.h>

#include <iostream>

int main(){

	Application myApp;

	myApp.configure();
	myApp.initialize();
	myApp.run();

	myApp.getLog().print();

	float end;
	std::cin >> end;
	
	return 0;
}