#ifndef DEBUGLOG_H
#define DEBUGLOG_H

#include <iostream>
#include <string>
#include <vector>

class DebugLog
{
private:
	std::vector< std::string > m_log;
	int  m_indent;
	bool m_autoPrint;
	inline std::string createIndent() const;
public:
	DebugLog(bool autoPrint = false);
	~DebugLog();
	void log(std::string msg);
	void indent();
	void outdent();
	void print() const;
	void printLast() const;
	void clear();

	void setAutoPrint(bool to);
};
#endif