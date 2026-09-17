#ifndef SHARED_LIGHTS_H
#define SHARED_LIGHTS_H

#include "Light.h"
#include "MathOps.h"

template<class T, int count>
class CSharedLights
{
public:
	void Init(float fallout0, float fallout1, uint ambientColor, uint color, uint param_6)
	{
		float fVar1;
		float fVar2;
		float fVar3;
		int* pCVar4;
		T* pCVar5;
		CLight* pLight;
		int iVar6;
		CLightManager* pLightManager;

		pLightManager = CScene::ptable.g_LightManager_004516b0;
		if (this->field_0x0 == 0) {
			iVar6 = 0;
			pCVar4 = this->field_0x60;
			pCVar5 = this->aLightArray;
			do {
				pCVar5->colour_0x4.r = 0;
				pCVar5->colour_0x4.g = 0;
				pLight = pCVar5;
				pCVar5->colour_0x4.b = pCVar5->colour_0x4.b & 0xf0;
				pCVar5->colour_0x4.b = pCVar5->colour_0x4.b & 0xf;
				pCVar5->colour_0x4.a = 0;
				pCVar5->colour_0x4 = (_rgba)0x0;
				pCVar5->field_0x8 = 0;

				// I don't know how this is meant to decompile, so for now just limiting it to the type used (I expect some inlining going on 
				// in the dissasmbly).
				static_assert(std::is_same<T, CLightOmni>::value, "CSharedLights can only be used with CLightOmni");
				reinterpret_cast<CLightOmni*>(pLight)->Create();
	
				pCVar5->baseShape.position = gF32Vertex4Zero;
				pCVar5->baseShape.fallout0 = fallout0;
				pCVar5->baseShape.fallout1 = fallout1;

				pCVar5->colorModel.color.x = (float)(color & 0xff);
				pCVar5->colorModel.color.y = (float)(color >> 8 & 0xff);
				pCVar5->colorModel.color.z = (float)(color >> 0x10 & 0xff);
				pCVar5->colorModel.color.w = 0.0f;
				pCVar5->colorModel.ambientColor.x = (float)(ambientColor & 0xff);
				pCVar5->colorModel.ambientColor.y = (float)(ambientColor >> 8 & 0xff);
				pCVar5->colorModel.ambientColor.z = (float)(ambientColor >> 0x10 & 0xff);
				pCVar5->colorModel.ambientColor.w = 0.0f;
				pCVar5->colorModel.field_0x20.x = (float)(param_6 & 0xff);
				pCVar5->colorModel.field_0x20.y = (float)(param_6 >> 8 & 0xff);
				pCVar5->colorModel.field_0x20.z = (float)(param_6 >> 0x10 & 0xff);
				pCVar5->colorModel.field_0x20.w = 0.0f;
				pLightManager->Reference(pLight, -1, false, false, -1);
				pLight->Inactivate();
				iVar6 = iVar6 + 1;
				*pCVar4 = 0;
				pCVar5 = pCVar5 + 1;
				pCVar4 = pCVar4 + 1;
			} while (iVar6 < count);

			this->field_0x0 = 1;
		}

		return;
	}

	void Term() { field_0x0 = 0; }

	undefined4 field_0x0;

	T aLightArray[count];
	int field_0x60[count];
};

#endif // SHARED_LIGHTS_H
