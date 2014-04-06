#ifndef DEBUGLOG_H
#define DEBUGLOG_H

#include <iostream>
#include <string>
#include <vector>

class DebugLog
{
private:
	std::vector< std::string > m_log;
	bool m_autoPrint;
public:
	DebugLog(bool autoPrint = false);
	~DebugLog();
	void log(std::string msg);
	void print() const;
	void printLast() const;
	void clear();

	void setAutoPrint(bool to);
};
#endif