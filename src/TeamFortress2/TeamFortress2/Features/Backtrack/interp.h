#pragma once
#include "../../SDK/SDK.h"


class CInterp
{

public:
	float GetLerpTime();
	float GetLowestPossibleLerpTime(int* nUpdateRate);
	int EstimateServerArriveTick();
	bool CanRestoreToSimulationTime(float flSimulationTime, bool* bNeedToAdjustInterp);
	//void GetUsableRecordsForEntity(CBaseEntity* pEntity, std::deque<TickRecord>* vecRecords, float flMinTime, float flMaxTime, bool* bShouldLagFix);
	//bool IsDeltaTooBig(Vector& vPos1, Vector& vPos2);
	//void GetUsableRecordsForIndex(int idx, std::deque<TickRecord>* vecRecords, float flMinTime, float flMaxTime, bool* bShouldLagFix);
};

ADD_FEATURE(CInterp, interp)