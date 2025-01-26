#ifndef FRONTEND_H
#define FRONTEND_H

#include "Types.h"
#include "LargeObject.h"

class CFrontend : public CObjectManager
{
public:
	// CObjectManager
	virtual void Level_ClearAll();

	static float GetTime();
};

#endif // FRONTEND_H