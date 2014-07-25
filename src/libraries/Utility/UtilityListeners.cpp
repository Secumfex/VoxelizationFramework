#include <Utility/UtilityListeners.h>

InvertBooleanListener::InvertBooleanListener(bool* boolean)
{
	p_boolean = boolean;
}

InvertBooleanListener::~InvertBooleanListener() {
}

void InvertBooleanListener::call()
{
	*p_boolean = !(*p_boolean);
}

#include "Utility/DebugLog.h"

DebugPrintDoubleListener::DebugPrintDoubleListener(double* doublePtr, std::string message) {
	p_double = doublePtr;
	m_message = message;
}

DebugPrintDoubleListener::~DebugPrintDoubleListener() {
}

void DebugPrintDoubleListener::call() {
	DEBUGLOG->log(m_message, *p_double);
}

DebugPrintVec4Listener::DebugPrintVec4Listener(glm::vec4* vectorPtr,
		std::string message) {
	p_vector = vectorPtr;
	m_message = message;
}

DebugPrintVec4Listener::~DebugPrintVec4Listener() {
}

void DebugPrintVec4Listener::call() {
	DEBUGLOG->log( m_message, *p_vector );
}
