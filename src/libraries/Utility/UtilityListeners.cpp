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


template<class T>
 IncrementValueListener<T>::IncrementValueListener(T* valuePtr, T increment) {
	m_increment = increment;
	p_value = valuePtr;
}

template<class T>
IncrementValueListener<T>::~IncrementValueListener() {
}

template<class T>
 void IncrementValueListener<T>::call() {
	*p_value += m_increment;
}

template<class T>
DecrementValueListener<T>::DecrementValueListener(T* valuePtr, T decrement) {
	p_value = valuePtr;
	m_decrement = decrement;
}

template<class T>
DecrementValueListener<T>::~DecrementValueListener() {
}

template<class T>
void DecrementValueListener<T>::call() {
	*p_value -= m_decrement;
}

template<class T>
 SwitchThroughValuesListener<T>::SwitchThroughValuesListener(T* valuePtr,
		std::vector< T& > candidates) {
	m_candidates = candidates;
	m_activeCandidate = 0;
	p_value = valuePtr;
}

template<class T>
 SwitchThroughValuesListener<T>::~SwitchThroughValuesListener() {
}

template<class T>
 void SwitchThroughValuesListener<T>::call() {
	if (m_candidates.empty())
	{
		return;
	}

	if (m_activeCandidate < m_candidates.size())
	{
		m_activeCandidate++;

	}
	else
	{
		m_activeCandidate = 0;
	}

	*p_value = m_candidates[ m_activeCandidate ];
}
