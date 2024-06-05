#pragma once
#include "../../SDK/SDK.h"
#include <deque>
#pragma warning ( disable : 4091 )

class CIncomingSequence
{
public:
	int InReliableState;
	int SequenceNr;
	float CurTime;

	CIncomingSequence(int inState, int seqNr, float time)
	{
		InReliableState = inState;
		SequenceNr = seqNr;
		CurTime = time;
	}
};

using BoneMatrixes = struct
{
	float BoneMatrix[128][3][4];
};

struct TickRecord
{
	float SimulationTime = -1;
	int TickCount = -1;
	Vec3 HeadPosition = { };
	Vec3 AbsOrigin = { };
	BoneMatrixes BoneMatrix{ };
	model_t* Model = nullptr;
	studiohdr_t* HDR = nullptr;
	int  HitboxSet = 0;
	Vec3 Mins = Vec3();
	Vec3 Maxs = Vec3();
	Vec3 WorldSpaceCenter = { };
	Vec3 EyeAngles = { };
};

enum class BacktrackMode
{
	First, Last, Distance, FOV
};

class CBacktrack
{

	std::optional<TickRecord> GetLastRecord(int entIdx);
	std::optional<TickRecord> GetFirstRecord(int entIdx);

	void UpdateDatagram();
	

	float LatencyRampup = 0.f;
	int LastInSequence = 0;
	std::deque<CIncomingSequence> Sequences;

public:
	void Run(CBaseEntity* Player);
	void UpdateRecords(CBaseEntity* Player);
	float GetLatency();
	void AdjustPing(INetChannel* netChannel);
	void ResetLatency();
	//bool IsGoodTick(float simTime);
	bool IsTickInRange(int tickCount);

	std::deque<TickRecord>* GetPlayerRecords(int entIdx);
	std::optional<TickRecord> GetRecord(int entIdx, BacktrackMode mode);

	bool AllowLatency = false;
	std::deque<TickRecord> Records[67];
};

ADD_FEATURE(CBacktrack, Backtrack)