#ifndef LIPSYNC_H
#define LIPSYNC_H

#include "LargeObject.h"

struct ByteCode;

class CLipTrack
{
public:
	virtual void Create(ByteCode* pByteCode) = 0;
	virtual int UpdateKFrame(float param_1) { IMPLEMENTATION_GUARD(); }
	virtual int GetOutputWidth() {}
	virtual int GetOutputType(int index) {}
	virtual float* GetOutputPtr(int index) {}
};

class CLipDataTrack : public CLipTrack
{
public:
	virtual void Create(ByteCode* pByteCode);

	float* field_0x4;
	int field_0x8;
};

struct CTrackMultiSubObj
{
	int field_0x0;
	float field_0x4;
};

class CTrackMultiChannel : public CLipDataTrack
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual int UpdateKFrame(float param_1);
	virtual int GetOutputWidth();
	virtual int GetOutputType(int index);
	virtual float* GetOutputPtr(int index);

	CTrackMultiSubObj* aSubObj;

	int outputWidth;
	int* aOutputTypes;
	float* aOutputPtrs;
};

class CTrackMonoChannel : public CLipDataTrack
{
public:
	virtual void Create(ByteCode* pByteCode) { IMPLEMENTATION_GUARD(); }
};

class CTrackVector : public CLipDataTrack
{
public:
	virtual void Create(ByteCode* pByteCode) { IMPLEMENTATION_GUARD(); }
};

class CTrackRndTickMonoChannel : public CTrackMonoChannel
{
public:
	virtual void Create(ByteCode* pByteCode) { IMPLEMENTATION_GUARD(); }
};

class CTrackRndSwitchMonoChannel : public CTrackMonoChannel
{
public:
	virtual void Create(ByteCode* pByteCode) { IMPLEMENTATION_GUARD(); }
};

class CTrackRndImpulseMonoChannel : public CTrackMonoChannel
{
public:
	virtual void Create(ByteCode* pByteCode) { IMPLEMENTATION_GUARD(); }
};

class CTrackRndCos : public CTrackMonoChannel
{
public:
	virtual void Create(ByteCode* pByteCode) { IMPLEMENTATION_GUARD(); }
};

class CTrackRndVarMonoChannel : public CTrackMonoChannel
{
public:
	virtual void Create(ByteCode* pByteCode) { IMPLEMENTATION_GUARD(); }
};

class CKFrameTrackReader
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Init();
	virtual void Manage(float param_1);

	float GetValue(int index);

	float field_0x4;
	float field_0xc;
	int nbTracks;
	CLipTrack** aTracks;
	float* field_0x14[18];
};

class CLipTrackManager : public CObjectManager
{
public:
	CLipTrackManager();

	virtual void Level_Init();
	virtual void Level_Term();

	virtual void Level_ClearAll();

	virtual void Level_Manage();
	virtual void Level_ManagePaused();

	int InstallFromMem(char* pFileData);
	void Remove(CKFrameTrackReader* pTrackReader);

	int nbLipTracks;
	CKFrameTrackReader* aLipTracks[0x80];
};

#endif // LIPSYNC_H
