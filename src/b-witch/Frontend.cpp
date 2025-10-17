#include "Frontend.h"
#include "TimeController.h"
#include "CameraViewManager.h"
#include "LevelScheduleManager.h"

ed_3D_Scene* CFrontend::_scene_handle = (ed_3D_Scene*)0x0;

CFrontend::CFrontend()
{
	this->pViewport = (ed_viewport*)0x0;

	return;
}

void CFrontend::Level_ClearAll()
{
	return;
}

float CFrontend::GetTime()
{
	return Timer::GetTimer()->totalTime;
}

bool CFrontend::ComputeSceneCoordinate(float z, edF32VECTOR4* pWorldPosition, edF32VECTOR2* pScreenCoordinate)
{
	bool bSuccess;
	edF32VECTOR2 position;

	position.x = (pScreenCoordinate->x - 0.5f) * 2.0f;
	position.y = -((pScreenCoordinate->y - 0.5f) * 2.0f);

	bSuccess = false;
	if ((CScene::ptable.g_CameraManager_0045167c)->pFrontendCamera_0x4e4 != (CCamera*)0x0) {
		bSuccess = ed3DComputeScreenCoordinate(z, pWorldPosition, &position, CFrontend::_scene_handle);
	}

	return bSuccess;
}

long CFrontendSamplePlayer::PlaySample(float, float*, int, int)
{
	IMPLEMENTATION_GUARD_AUDIO();
	return 0;
}

bool CMagicInterface::Activate(int bActive)
{
	bool bSuccess;

	bSuccess = false;

	if (bActive == 0) {
		this->bActive = 0;
		bSuccess = true;
	}
	else {
		if (0.0f < this->value) {
			bSuccess = true;
			this->bActive = 1;
		}
	}

	return bSuccess;
}

bool CMagicInterface::CanActivate()
{
	bool bSuccess;

	bSuccess = 0.0f < this->value;
	if (bSuccess) {
		bSuccess = this->bActive == 0;
	}

	return bSuccess;
}

bool CMagicInterface::IsActive()
{
	return this->bActive;
}

bool CMagicInterface::Manage()
{
	bool bSuccess;

	bSuccess = CScene::_pinstance->IsFadeTermActive();
	return bSuccess == false;
}

void CMagicInterface::Draw()
{
	float fVar1;

	if (this->bActive != 0) {
		fVar1 = this->value - Timer::GetTimer()->cutsceneDeltaTime;
		this->value = fVar1;
		if (fVar1 <= 0.0f) {
			this->value = 0.0f;
			this->bActive = 0;
		}
	}

	return;
}

void CMagicInterface::SetValue(float value)
{
	this->value = value;
	return;
}

float CMagicInterface::GetValue()
{
	return this->value;
}

float CMagicInterface::GetValueMax()
{
	return this->valueMax;
}

void CMagicInterface::SetValueMax(float max)
{
	this->valueMax = max;
	return;
}

void CMagicInterface::SetTransit(float transit)
{
	this->transit = transit;
	return;
}

float CMagicInterface::GetTransit()
{
	return this->transit;
}

void CMagicInterface::FUN_001dcd70()
{
	this->field_0x10 = this->field_0xc;
	this->field_0xc = 3.402823e+38f;

	return;
}

void CMagicInterface::FUN_001dcda0(float param_1)
{
	if (param_1 < this->field_0xc) {
		this->field_0xc = param_1;
	}

	return;
}

void CMagicInterface::SetHasMagicInteractionAround(int bHasMagicAround)
{
	this->bHasMagicAround = bHasMagicAround;
	return;
}

int CMagicInterface::HasMagicInteractionAround()
{
	return this->bHasMagicAround;
}

float CMagicInterface::GetField_0x10()
{
	return this->field_0x10;
}

bool CMoneyInterface::Manage()
{
	bool bVar1;

	bVar1 = CScene::_pinstance->IsFadeTermActive();
	return bVar1 == false;
}

float CMoneyInterface::GetValue()
{
	return (float)CLevelScheduler::_gGameNfo.nbMoney;
}

CLifeInterface::CLifeInterface()
{
	this->priority = 2;
	this->currentValue = 1.0f;
	this->valueMax = 1.0f;
	this->field_0x10 = 0;
	return;
}

bool CLifeInterface::Manage()
{
	bool bVar1;

	bVar1 = CScene::_pinstance->IsFadeTermActive();
	bVar1 = bVar1 != false;

	if (!bVar1) {
		bVar1 = CScene::_pinstance->IsCutsceneFadeActive();
		bVar1 = bVar1 != false;
	}

	if (!bVar1) {
		bVar1 = CScene::_pinstance->IsResetFadeActive();
		bVar1 = bVar1 != false;
	}

	return (bVar1 ^ 1);
}

float CLifeInterface::GetValue()
{
	return this->currentValue;
}

void CLifeInterface::SetValue(float value)
{
	this->currentValue = value;
}

float CLifeInterface::GetValueMax()
{
	return this->valueMax;
}

void CLifeInterface::SetValueMax(float max)
{
	ACTOR_LOG(LogLevel::Info, "CLifeInterface::SetValueMax Setting max life to {}", max);
	this->valueMax = max;
	return;
}

void CLifeInterface::SetPriority(int newPriority)
{
	this->priority = newPriority;
	return;
}

float CLifeInterface::GetPriority()
{
	return this->priority;
}

float CLifeInterface::GetPercent()
{
	return GetValue() / this->valueMax;
}
