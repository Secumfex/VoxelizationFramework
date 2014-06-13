#ifndef SUBJECTLISTENERPATTERN_H
#define SUBJECTLISTENERPATTERN_H

#include <map>
#include <list>
#include <string>

class Listener;
class Subject {
protected:
	std::map< std::string, std::list<Listener* > > m_listeners;
public:
	Subject();
	~Subject();

	void attach( Listener* listener, std::string interface );
	void detach( Listener* listener, std::string interface );
	void detach( Listener* listener);
	void call( std::string interface );
};

class Listener{
protected:
	Subject* m_subject;
public:
	Listener();
	virtual ~Listener();

	virtual void call() = 0;
	void detach();

	void setSubject( Subject* subject);
	Subject* getSubject();
};

#endif
