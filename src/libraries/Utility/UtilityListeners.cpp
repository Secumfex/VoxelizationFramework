#include "UtilityListeners.h"

InvertBooleanListener::InvertBooleanListener(bool* boolean)
{
	p_boolean = boolean;
}

void InvertBooleanListener::call()
{
	*p_boolean = !(*p_boolean);
}