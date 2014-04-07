#ifndef UTILITYLISTENERS_H
#define UTILITYLISTENERS_H

#include "SubjectListenerPattern.h"

class InvertBooleanListener : public Listener
{
private:
	bool* p_boolean;
public:
	InvertBooleanListener(bool* boolean);
	void call();
};

#endif