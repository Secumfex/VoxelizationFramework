#include "DebugLog.h"

DebugLog::DebugLog(bool autoPrint)
{
	m_autoPrint = autoPrint;
}


DebugLog::~DebugLog()
{

}


void DebugLog::log(std::string msg)
{
	m_log.push_back(msg);
	if (m_autoPrint)
	{
		printLast();
	}
}


void DebugLog::print() const{
	for (unsigned int i = 0; i < m_log.size(); i++)
	{
		std::cout << m_log[i] << std::endl;
	}
}

void DebugLog::printLast() const{
	std::cout << m_log.back() << std::endl;
}

void DebugLog::clear()
{
	m_log.clear();
}

void DebugLog::setAutoPrint(bool to)
{
	m_autoPrint = to;
}