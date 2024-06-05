#pragma once

#include "../../../SDK/SDK.h"

class CMovementSimulation
{
public:
	CBaseEntity* m_pPlayer = nullptr;
	CMoveData m_MoveData = {};

private:
	void SetupMoveData(CBaseEntity* pPlayer, CMoveData* pMoveData);
private:
	bool m_bOldInPrediction = false;
	bool m_bOldFirstTimePredicted = false;
	bool bFirstRunTick = false;
	float m_flOldFrametime = 0.0f;
	float shit = 0.0f;

public:
	std::map<int, std::deque<Vec3>> m_Velocities;
	bool Initialize(CBaseEntity* pPlayer);
	void Restore();
	bool BetaStrafePred();
	void FillVelocities();
	void RunTick(CMoveData& moveDataOut, Vec3& m_vecAbsOrigin);
	const Vec3& GetOrigin() { return m_MoveData.m_vecAbsOrigin; }
};

ADD_FEATURE(CMovementSimulation, MoveSim)