#pragma once
#include "../../SDK/SDK.h"
#include "../Commands/Commands.h"



class CMisc {
	void AccurateMovement(CUserCmd* pCmd, CBaseEntity* pLocal);
	void AutoJump(CUserCmd* pCmd, CBaseEntity* pLocal);
	void AutoStrafe(CUserCmd* pCmd, CBaseEntity* pLocal, Vector& OriginalView);
	void AntiBackstab(CBaseEntity* pLocal, CUserCmd* pCmd);
	void CheatsBypass();
	void Teleport(const CUserCmd* pCmd);
	bool next_lby_update(CUserCmd* cmd);
	void AutoPeek(CUserCmd* pCmd, CBaseEntity* pLocal);
	void FastStop(CUserCmd* pCmd, CBaseEntity* pLocal);
	int m_switch = 1;
	float m_old_yaw = 0;
public:
	void AutoSwitch(CUserCmd* pCmd, CBaseEntity* pLocal);
	void Run(CUserCmd* pCmd);
	void RunLate(CUserCmd* pCmd);
	Vec3 PeekReturnPos;
private:
	std::pair<int, std::pair<CBaseEntity*, bool>> pWaiting = { 0, {nullptr, false} };

};

ADD_FEATURE(CMisc, Misc)

