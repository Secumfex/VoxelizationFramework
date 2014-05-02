#ifndef DEBUGLOG_H
#define DEBUGLOG_H

#include <iostream>
#include <string>
#include <vector>

#include <Utility/Singleton.h>

class DebugLog : public Singleton<DebugLog>
{
friend class Singleton< DebugLog >;
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

// for convenient access
#define DEBUGLOG DebugLog::getInstance()

#endif

