#ifndef FRONTEND_H
#define FRONTEND_H

#include "Types.h"
#include "LargeObject.h"

class CFrontendSamplePlayer
{
public:
	long PlaySample(float, int, int);
};

class CInterface
{
public:
	virtual bool Activate(int bActive) { return true; }
	virtual bool CanActivate() { return true; }
	virtual bool IsActive() { return true; }
	virtual bool Manage() { return true; }

	virtual void Draw() = 0;
	virtual void Reset() = 0;
	virtual void SetValue(float value) = 0;
	virtual float GetValue() = 0;
};

class CMagicInterface : public CInterface
{
public:
	// CInterface
	virtual bool Activate(int bActive);
	virtual bool CanActivate();
	virtual bool IsActive();
	virtual bool Manage();
	virtual void Draw();
	virtual void Reset() {}
	virtual void SetValue(float value);
	virtual float GetValue();

	float GetValueMax();
	void SetValueMax(float max);

	void SetTransit(float transit);
	float GetTransit();

	void FUN_001dcd70();
	void FUN_001dcda0(float param_1);

	void SetHasMagicInteractionAround(int bHasMagicAround);
	int HasMagicInteractionAround();

	float GetField_0x10();

	int bActive;
	int bHasMagicAround;
	float field_0xc;
	float field_0x10;
	float value;
	float valueMax;
	float transit;
};

class CMoneyInterface : public CInterface
{
public:
	// CInterface
	virtual bool Manage();
	virtual void Draw() {}
	virtual void Reset() {}
	virtual void SetValue(float value) {}
	virtual float GetValue();
};

class CLifeInterface : public CInterface
{
public:
	CLifeInterface();

	int priority;
	float currentValue;
	float valueMax;
	int field_0x10;

	// CInterface
	virtual bool Manage();
	virtual void Draw() {}
	virtual void Reset() {}
	virtual float GetValue();
	virtual void SetValue(float value);

	float GetValueMax();
	void SetValueMax(float max);

	void SetPriority(int newPriority);
	float GetPriority();

	float GetPercent();
};

class CFrontend : public CObjectManager
{
public:
	static ed_3D_Scene* _scene_handle;

	CFrontend();

	// CObjectManager
	virtual void Level_ClearAll();

	static float GetTime();

	bool ComputeSceneCoordinate(float z, edF32VECTOR4* pWorldPosition, edF32VECTOR2* pRect);

	struct ed_viewport* pViewport;
};

#endif // FRONTEND_H