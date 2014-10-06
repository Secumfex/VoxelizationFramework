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

DebugPrintBooleanListener::DebugPrintBooleanListener(bool* booleanPtr, std::string message) {
	p_boolean = booleanPtr;
	m_message = message;
}

DebugPrintBooleanListener::~DebugPrintBooleanListener() {
}

void DebugPrintBooleanListener::call() {
	DEBUGLOG->log(m_message, *p_boolean);
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

ConditionalProxyListener::ConditionalProxyListener(Listener* listener,
		bool* boolean, bool invert) {
	p_listener = listener;
	p_boolean = boolean;
	m_invert = invert;
}

ConditionalProxyListener::~ConditionalProxyListener() {
}

void ConditionalProxyListener::call() {
	if ( (m_invert) ? !*p_boolean : *p_boolean )
	{
		p_listener->call();
	}
}

DebugPrintListener::DebugPrintListener(std::string message) {
	m_message = message;
}

DebugPrintListener::~DebugPrintListener() {
}

void DebugPrintListener::call() {
	DEBUGLOG->log(m_message);
}

SubjectListener::SubjectListener() {
}

SubjectListener::~SubjectListener() {
}

void SubjectListener::addListener(Listener* listener) {
	attach(listener, "CALL");
}

void SubjectListener::call() {
	Subject::call("CALL");
}
