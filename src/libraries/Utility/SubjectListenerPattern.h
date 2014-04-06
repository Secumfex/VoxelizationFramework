#ifndef SUBJECTLISTENERPATTERN_H
#define SUBJECTLISTENERPATTERN_H

class Subject {
protected:
	std::vector<Listener* > m_listeners; 
public:
	Subject();
	~Subject();

	void attachListener( Listener* listener );
	void detachListener( Listener* listener );
	void callListeners();
};

class Listener{
protected:
	Subject* m_subject;
public:
	Listener();
	~Listener();

	virtual void call() = 0;
	void detach();

	void setSubject( Subject* subject);
	Subject* getSubject();
};

#endif