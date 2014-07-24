#include "SubjectListenerPattern.h"

/*		Subject 		*/
Subject::Subject()
{

}

Subject::~Subject()
{
	for ( std::map < std::string, std::list < Listener* > >::iterator i = m_listeners.begin(); i != m_listeners.end(); ++i)
	{
		for (std::list<Listener*>::iterator l = (*i).second.begin(); l != (*i).second.end();++l)
		{
//			delete (*l);
		}
		(*i).second.clear();
	}
}

void Subject::attach( Listener* listener, std::string interface )
{
	listener->setSubject(this);
	m_listeners[interface].push_back( listener );
}

void Subject::detach( Listener* listener, std::string interface )
{
	m_listeners[interface].remove( listener );
}

void Subject::detach( Listener* listener )
{
	for (std::map<std::string, std::list<Listener* > >::iterator interface = m_listeners.begin(); interface != m_listeners.end(); ++interface)
	{
		(*interface).second.remove( listener );
	}
}

void Subject::call ( std::string interface )
{
	for ( std::list <Listener* >::iterator it = m_listeners[interface].begin(); it != m_listeners[interface].end(); )
	{
		Listener* listener = (*it);
		++it;
		listener->call();
	}
}

/*		Listener 	*/

Listener::Listener()
{
	m_subject = 0;
}

Listener::~Listener()
{

}

void Listener::detach()
{
	if (m_subject)
	{
		m_subject->detach(this);
	}
}

void Listener::setSubject(Subject* subject)
{
	m_subject = subject;
}

Subject* Listener::getSubject()
{
	return m_subject;
}
