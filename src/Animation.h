#ifndef ANIMATION_H
#define ANIMATION_H

#include "Types.h"
#include "AnmManager.h"
#include "EdenLib/edAnim/AnmSkeleton.h"

#define ANIMATION_LOG(level, format, ...) MY_LOG_CATEGORY("Animation", level, format, ##__VA_ARGS__)

#define STATE_ANIM_NONE		0
#define STATE_ANIM_PLAYING	1
#define STATE_ANIM_MORPHING	2

struct edAnmAnim;

struct edAnmStage {
	edAnmStage();

	int animMode;
	undefined4 field_0x4;
	float field_0x8;
	edF32MATRIX4* pFrameMatrixData;
	edF32MATRIX3* pConstantMatrixData;
	struct AnimMatrix* pAnimMatrix;
	struct edANM_WRTS* pRelativeTransformMatrixBuffer;
	edAnmSkeleton anmSkeleton;
	edANM_HDR* pKeyData;
	int field_0x24;
	undefined4 bLoop;
	float field_0x2c;
	float field_0x30;
	float field_0x34;
	float field_0x38;
	float animTime;
	float field_0x40;
	undefined field_0x44;
	undefined field_0x45;
	undefined field_0x46;
	undefined field_0x47;
	undefined field_0x48;
	undefined field_0x49;
	undefined field_0x4a;
	undefined field_0x4b;
	bool field_0x4c;
	bool field_0x4d;
	undefined field_0x4e;
	undefined field_0x4f;

	void SetActor(edANM_SKELETON* pSkeleton);
	void SetDestinationWRTS(edANM_WRTS* pMatrixBuffer, int count);
	void BlendToDestWRTS(float alpha, edF32MATRIX4* m0, edF32MATRIX4* m1);
	void ToonWRTSToGlobalMatrices(uchar mode);
	static bool ComputeAnimParams(float param_1, float param_2, float param_3, float* param_4, bool param_5, int loopType);
	void SetAnim(edANM_HDR* pKeyData);
	void SetAnimLoop(edANM_HDR* pKeyData);
	void SetTimeAsRatio(float ratio);
	void SetTime(float time);
	void AnimToWRTS();
	void AnimBlendToWRTS(float param_1);

};

union edANM_HDR_Internal {
	float asTime;
	int asKey;
};

struct edANM_HDR {
	int count_0x0;
	edANM_HDR_Internal field_0x4;
	edANM_HDR_Internal keyIndex_0x8;
	float pData[];
};

static_assert(sizeof(edANM_HDR) == 0xc, "Invalid edANM_HDR size");

struct edAnmMacroBlendN {
	edAnmMacroBlendN() {}
	edAnmMacroBlendN(edANM_HDR* pInHdr)
		: pHdr(pInHdr) {}

	edANM_HDR* pHdr;
};

struct AnimDesc {
	int animType;
	edANM_HDR* pHdrA;
	uint flags;
	int maxKey_0xc;
	edANM_HDR* pHdrB;
	int origAnimType;
	edAnmMacroAnimator state;
	float morphDuration;
	int animMode;
};


struct edAnmStateDesc {
	int field_0x0;
	int animType;
	edANM_HDR* pHdrA;
	int flags;
	int maxKey_0x10;
	edANM_HDR* pHdrB;
	int origAnimType;
};

struct edAnmLayer {
	void Reset();
	void AnimateDT(float deltaTime);
	bool MorphingDT(float playTime);
	bool PlayingDT(float playTime);
	void SetRawAnim(edANM_HDR* pHdr, uint flags, uint type);
	bool MorphingInitDT(edAnmStateDesc* pNewAnimation);
	bool MorphingStartDT();

	void SetAnim(edAnmStateDesc* pDesc);
	int field_0x0;
	float field_0x4;
	int animPlayState;
	undefined4 field_0xc;
	AnimDesc currentAnimDesc;
	AnimDesc nextAnimDesc;
	int field_0xb8;
	float field_0xbc;
	AnimationCallback pFunction_0xc0;
	struct CActor* pActor;
	edANM_HDR** pAnimManagerKeyData;
	uint field_0xcc;
	float playRate;
	float field_0xd4;
};

struct edAnmMetaAnimator {
	int layerCount;
	edAnmLayer* aAnimData;

	void SetLayerProperties(int count);
	void SetLayerResourceArray(int index, edANM_HDR** pAnimManagerKeyData);
	float GetLayerAnimTime(int animIndex, int param_3);
	void AnimateDT(float deltaTime);

	float GetAnimStateDescLength(edAnmStateDesc* pOutDesc, int lengthMode);
};

struct edAnmBinMetaAnimator : public edAnmMetaAnimator {
	int* pAnimKeyEntryData;
	void SetAnimStatesBinary(void* pAnimKeyEntryData);
	void SetLayerMacroAnimCallback(int index, AnimationCallback callback);
	void SetLayerMacroAnimUserParams(int index, CActor* pActor);
	void SetLayerTimeWarper(float value, int index);
	void SetAnim(int animType, int origAnimType);
	int GetAnimType_00242330(int animIndex);
	int GetAnimEventTrackID(int index);
};

class CAnimation {
public:
	CAnimation();

	bool UpdateCurSkeleton(CActor* pActor);
	void Create(CActor* pActor, uint count, edAnmLayer* pAnimData, int layerCount);
	void StopEventTrack(int state);
	void Manage(float deltaTime, CActor* pActor, int bHasFlag, int bPlayingAnimation);
	void PlayAnim(CActor* pActor, int animType, int origAnimType);
	void Reset(CActor* pActor);
	BoneData* FindReggedBone(int bone);
	void RegisterBone(uint boneId);
	void UnRegisterBone(uint boneId);

	float GetAnimLength(int param_2, int lengthMode);

	edF32MATRIX4* GetCurBoneMatrix(uint boneId);

	struct FrameMatrixData_8 {
		uint field_0x0;
		int field_0x4;
	};

	edAnmBinMetaAnimator anmBinMetaAnimator;
	edAnmSkeleton anmSkeleton;
	BoneData* pBoneData;
	float aTrackData[4];
	edF32MATRIX3* pAnimMatrix;
	undefined4 field_0x28;
	uint count_0x2c;
	int currentAnimType_0x30;
	FrameMatrixData_8 field_0x34[4];
	int count_0x54;
};

extern edAnmStage TheAnimStage;

#endif // ANIMATION_H
