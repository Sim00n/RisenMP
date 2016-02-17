#pragma once
#include "Module.h"

class Client
{
public:
	Client(void);
	void Pulse();
	~Client(void);

	static Client *Instance;
};