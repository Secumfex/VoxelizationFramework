#ifndef UTILITYLISTENERS_H
#define UTILITYLISTENERS_H

#include <string>
#include <vector>
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

template < class T >
class IncrementValueListener : public Listener
{
private:
	T* p_value;
	T m_increment;
public:
	IncrementValueListener(T* valuePtr, T increment) {
		m_increment = increment;
		p_value = valuePtr;
	}
	virtual ~IncrementValueListener() {};
	void call() {
		*p_value += m_increment;
	}
};

template < class T >
class DecrementValueListener : public Listener
{
private:
	T* p_value;
	T m_decrement;
public:
	DecrementValueListener(T* valuePtr, T decrement);
	virtual ~DecrementValueListener() {
	}
	void call() {
		*p_value -= m_decrement;
	}
};

template < class T >
class SwitchThroughValuesListener : public Listener
{
private:
	T* p_value;
	std::vector < T& > m_candidates;
	unsigned int m_activeCandidate;
public:
	SwitchThroughValuesListener(T* valuePtr,
			std::vector< T& > candidates) {
		m_candidates = candidates;
		m_activeCandidate = 0;
		p_value = valuePtr;
	}
	virtual ~SwitchThroughValuesListener() {
	}
	void call() {
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
};

#endif
