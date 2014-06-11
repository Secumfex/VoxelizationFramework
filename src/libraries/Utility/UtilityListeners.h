#ifndef UTILITYLISTENERS_H
#define UTILITYLISTENERS_H

#include <string>
#include "SubjectListenerPattern.h"

class InvertBooleanListener : public Listener
{
private:
	bool* p_boolean;
public:
	InvertBooleanListener(bool* boolean);
	virtual ~InvertBooleanListener();
	void call();
};

class DebugPrintDoubleListener : public Listener
{
private:
	double* p_double;
	std::string m_message;
public:
	DebugPrintDoubleListener( double* doublePtr , std::string message);
	virtual ~DebugPrintDoubleListener();
	void call();
};

#endif
