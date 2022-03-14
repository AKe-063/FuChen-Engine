#pragma once
#include "FSingleton.h"

class FSemaphore : public FSingleton<FSemaphore>
{
public:
	FSemaphore();
	~FSemaphore();

private:

};