#include "AntiAim.h"
#include "../Vars.h"
#include "../../Utils/Timer/Timer.hpp"
#include "../Visuals/Visuals.h"
#include "FakeLag/FakeLag.h"
#include "../Visuals/FakeAngleManager/FakeAng.h"

int edgeToEdgeOn = 0;
float lastRealAngle = -90.f;
float lastFakeAngle = 90.f;
bool wasHit = false;

float quick_normalize(float degree, const float min, const float max) {
	while (degree < min)
		degree += max - min;
	while (degree > max)
		degree -= max - min;

	return degree;
}
bool trace_to_exit_short(Vector& point, Vector& dir, const float step_size, float max_distance)
{
	float flDistance = 0;

	while (flDistance <= max_distance)
	{
		flDistance += step_size;

		point += dir * flDistance;

		if ((I::EngineTrace->GetPointContents(point) & MASK_SOLID) == 0)
		{
			// found first free point
			return true;
		}
	}

	return false;
}

float get_thickness(Vector& start, Vector& end) {
	Vector dir = end - start;
	Vector step = start;
	dir /= dir.Length();
	CTraceFilterWorldAndPropsOnly filter;
	CGameTrace trace;
	Ray_t ray;
	float thickness = 0;
	while (true) {
		ray.Init(step, end);
		I::EngineTrace->TraceRay(ray, MASK_SOLID, &filter, &trace);

		if (!trace.DidHit())
			break;

		const Vector lastStep = trace.vEndPos;
		step = trace.vEndPos;

		if ((end - start).Length() <= (step - start).Length())
			break;

		if (!trace_to_exit_short(step, dir, 5, 90))
			return FLT_MAX;

		thickness += (step - lastStep).Length();
	}
	return thickness;
}

void CAntiAim::FixMovement(CUserCmd* pCmd, const Vec3& vOldAngles) {
	//better movement fix roll and pitch above 90 and -90 l0l
	static auto cl_forwardspeed = g_ConVars.FindVar("cl_forwardspeed");
	static auto cl_sidespeed = g_ConVars.FindVar("cl_sidespeed");
	if (!cl_sidespeed || !cl_forwardspeed)
	{
		return;
	}

	const float flMaxForwardSpeed = cl_forwardspeed->GetFloat();
	const float flMaxSideSpeed = cl_sidespeed->GetFloat();
	Vector vecForward = {}, vecRight = {}, vecUp = {};
	Math::AngleVectors(vOldAngles, &vecForward, &vecRight, &vecUp);
	vecForward.z = vecRight.z = vecUp.x = vecUp.y = 0.f;
	vecForward.NormalizeInPlace();
	vecRight.NormalizeInPlace();
	vecUp.NormalizeInPlace();
	Vector vecOldForward = {}, vecOldRight = {}, vecOldUp = {};
	Math::AngleVectors(pCmd->viewangles, &vecOldForward, &vecOldRight, &vecOldUp);
	vecOldForward.z = vecOldRight.z = vecOldUp.x = vecOldUp.y = 0.f; // these can all have 3 vectors can they not?
	vecOldForward.NormalizeInPlace();
	vecOldRight.NormalizeInPlace();
	vecOldUp.NormalizeInPlace();
	const float flPitchForward = vecForward.x * pCmd->forwardmove; //	chunky
	const float flYawForward = vecForward.y * pCmd->forwardmove; //	chunky
	const float flPitchSide = vecRight.x * pCmd->sidemove; //	chunky
	const float flYawSide = vecRight.y * pCmd->sidemove; //	chunky
	const float flRollUp = vecUp.z * pCmd->sidemove; //	chunky
	const float x = vecOldForward.x * flPitchSide + vecOldForward.y * flYawSide + vecOldForward.x * flPitchForward + vecOldForward.y * flYawForward + vecOldForward.z * flRollUp;
	const float y = vecOldRight.x * flPitchSide + vecOldRight.y * flYawSide + vecOldRight.x * flPitchForward + vecOldRight.y * flYawForward + vecOldRight.z * flRollUp;
	pCmd->forwardmove = std::clamp(x, -flMaxForwardSpeed, flMaxForwardSpeed);
	pCmd->sidemove = std::clamp(y, -flMaxSideSpeed, flMaxSideSpeed);
}

void CAntiAim::GetEdge(CUserCmd* pCmd)
{
	std::vector<angle_data> points;



	const auto local = g_EntityCache.GetLocal();



	const auto local_position = local->GetEyePosition();
	std::vector<float> scanned = {};

	for (auto i = 0; i <= 64; i++)
	{
		auto p_entity = dynamic_cast <CBaseEntity*> (I::ClientEntityList->GetClientEntity(i));
		if (p_entity == nullptr) continue;
		if (p_entity == local) continue;
		if (!p_entity->IsAlive()) continue;
		if (p_entity->GetTeamNum() == local->GetTeamNum()) continue;

		if (!p_entity->IsPlayer()) continue;

		const auto view = Math::CalcAngle(local_position, p_entity->GetEyePosition());

		std::vector<angle_data> angs;

		for (auto y = 1; y < 4; y++)
		{
			auto ang = quick_normalize((y * 90) + view.y, -180.f, 180.f);
			auto found = false; // check if we already have a similar angle

			for (auto i2 : scanned)
				if (abs(quick_normalize(i2 - ang, -180.f, 180.f)) < 20.f)
					found = true;

			if (found)
				continue;

			points.emplace_back(ang, -1.f);
			scanned.push_back(ang);
		}
		//points.push_back(base_angle_data(view.y, angs)); // base yaws and angle data (base yaw needed for lby breaking etc)
	}

	for (auto i = 0; i <= 64; i++)
	{
		auto p_entity = dynamic_cast <CBaseEntity*> (I::ClientEntityList->GetClientEntity(i));
		if (p_entity == nullptr) continue;
		if (p_entity == local) continue;
		if (!p_entity->IsAlive()) continue;
		if (p_entity->GetTeamNum() == local->GetTeamNum()) continue;
		if (!p_entity->IsPlayer()) continue;

		auto found = false;
		auto points_copy = points; // copy data so that we compair it to the original later to find the lowest thickness
		auto enemy_eyes = p_entity->GetEyePosition();

		for (auto& z : points_copy) // now we get the thickness for all of the data
		{
			const Vector tmp(10, z.angle, 0.0f);
			Vector head;
			Math::AngleVectors(tmp, &head);
			head *= ((16.0f + 3.0f) + ((16.0f + 3.0f) * sin(DEG2RAD(10.0f)))) + 7.0f;
			head += local_position;
			const auto local_thickness = get_thickness(head, enemy_eyes); // i really need my source for this bit, i forgot how it works entirely Autowall :: GetThickness1 (head, hacks.m_local_player, p_entity);
			z.thickness = local_thickness;

			if (local_thickness != 0) // if theres a thickness of 0 dont use this data
			{
				found = true;
			}
		}

		if (!found) // dont use
			continue;

		for (auto z = 0; points_copy.size() > z; z++)
			if (points_copy[z].thickness < points[z].thickness || points[z].thickness == -1) // find the lowest thickness so that we can hide our head best for all entities
				points[z].thickness = points_copy[z].thickness;

	}
	float best = 0;
	for (auto& i : points)
	if ((i.thickness > best || i.thickness == -1) && i.thickness != 0) // find the best hiding spot (highest thickness)
	{
		best = i.thickness;
		pCmd->viewangles.y = i.angle;
	
	}

}

float GetBaseYaw(int iMode, CBaseEntity* pLocal, CUserCmd* pCmd)
{
	//	0 offset, 1 at player, 2 at player + offset

	switch (iMode)
	{
	case 0: { return pCmd->viewangles.y; }
	case 1:
	{
		float flSmallestAngleTo = 0.f; float flSmallestFovTo = 360.f;
		for (CBaseEntity* pEnemy : g_EntityCache.GetGroup(EGroupType::PLAYERS_ENEMIES)) {
			if (!pEnemy || !pEnemy->IsAlive() || pEnemy->GetDormant()) { continue; }
			const Vec3 vAngleTo = Math::CalcAngle(pLocal->GetAbsOrigin(), pEnemy->GetAbsOrigin());
			const float flFOVTo = Math::CalcFov(I::EngineClient->GetViewAngles(), vAngleTo);

			if (flFOVTo < flSmallestFovTo) { flSmallestAngleTo = vAngleTo.y; flSmallestFovTo = flFOVTo; }
		}
		return flSmallestAngleTo;
	}
	case 2:
	{
		float flSmallestAngleTo = 0.f; float flSmallestFovTo = 360.f;
		for (CBaseEntity* pEnemy : g_EntityCache.GetGroup(EGroupType::PLAYERS_ENEMIES))
		{
			if (!pEnemy || !pEnemy->IsAlive() || pEnemy->GetDormant()) { continue; }	//	is enemy valid
			PlayerInfo_t pInfo{ };
			if (I::EngineClient->GetPlayerInfo(pEnemy->GetIndex(), &pInfo))
			{
				if (G::IsIgnored(pInfo.friendsID)) { continue; }
			}
			const Vec3 vAngleTo = Math::CalcAngle(pLocal->GetAbsOrigin(), pEnemy->GetAbsOrigin());
			const float flFOVTo = Math::CalcFov(I::EngineClient->GetViewAngles(), vAngleTo);

			if (flFOVTo < flSmallestFovTo) { flSmallestAngleTo = vAngleTo.y; flSmallestFovTo = flFOVTo; }
		}
		return (flSmallestFovTo == 360.f ? pCmd->viewangles.y + (iMode == 2) : flSmallestAngleTo + (iMode == 2));
	}
	}
	return pCmd->viewangles.y;
}

float CAntiAim::CalculateCustomRealPitch(float WishPitch, bool FakeDown) {
	return FakeDown ? 2160 + WishPitch : -2160 + WishPitch;
}

float get(CUserCmd* ucmd) {
	if (const auto& pLocal = g_EntityCache.GetLocal()) {

		if (!pLocal)
			return false;
		static int g_tick = 0;
		static CUserCmd* g_pLastCmd = nullptr;
		if (!g_pLastCmd || g_pLastCmd->hasbeenpredicted) {
			g_tick = pLocal->m_nTickBase();
		}
		else {
			// Required because prediction only runs on frames, not ticks
			// So if your framerate goes below tickrate, m_nTickBase won't update every tick
			++g_tick;
		}
		g_pLastCmd = ucmd;
		float curtime = g_tick * I::GlobalVars->interval_per_tick;
		return curtime;
	}
}

bool next(CUserCmd* cmd)
{
	if (const auto& pLocal = g_EntityCache.GetLocal()) {

		if (!pLocal)
			return false;

		float curtime = get(cmd);

		float m_flNextBodyUpdate = 0;
		// walking, delay next update by .22s.
		if (pLocal->GetVelocity().Length2D() > 0.5f)
			m_flNextBodyUpdate = curtime + 0.22f;
		else if (pLocal->GetVelocity().Length2D() < 0.5 && curtime > m_flNextBodyUpdate)
			m_flNextBodyUpdate = curtime + 1.1f;


		if (curtime >= m_flNextBodyUpdate)
		{
			return true;
		}

		return false;

	}
	return false;
}


float CAntiAim::GetReal(CUserCmd* pCmd, int nIndex, bool* pSendPacket)
{
	if (const auto& pLocal = g_EntityCache.GetLocal()) {

		static Timer cmdTimer{};
		switch (nIndex) {
		case 1: //forward
		{
			pCmd->viewangles.y = 0;

			break;
		}
		case 2:
		{
			pCmd->viewangles.y = bInvert ? 90 : -90;
			break;
		}
		case 3:
		{
			pCmd->viewangles.y = bInvert ? -90 : 90;
			break;
		}
		case 4: //back
		{
			pCmd->viewangles.y = next(pCmd) ? 180 : 0;
			break;
		}
		case 5: //spin
		{

			pCmd->viewangles.y = fmod(I::GlobalVars->curtime * 720 * Vars::AntiHack::AntiAim::SpinSpeed.Value, 720);
			break;
		}
		case 6: //edge
		{		static bool hi = false;
			float first = bInvert ? -178 : 178;
			float second = bInvert ? 1 : -1;

			pCmd->viewangles.y = hi ? first : second;
			hi = !hi;
			break;
		}
		case 7: //custom
		{
			static bool counter = false;
			static int counters = 0;
			if (counters == 5)
			{
				counters = 0;
				counter = !counter;
			}
			counters++;
			if (counter)
			{
				pCmd->viewangles.y = bInvert ? Vars::AntiHack::AntiAim::first.Value : Vars::AntiHack::AntiAim::FakeFirst.Value;
			}
			else
				pCmd->viewangles.y = bInvert ? Vars::AntiHack::AntiAim::second.Value : Vars::AntiHack::AntiAim::FakeSecond.Value;
			break;
		}

		}
	}

	return pCmd->viewangles.y;

}


/*
// walking, delay next update by .22s.
if (m_serverAnimState->m_flSpeed > 0.1f)
m_flNextBodyUpdate = gpGlobals->curtime + 0.22f;

// calculate delta.
float delta = std::abs(Math::NormalizeAngle(m_angRealAngle - local->m_flLowerBodyYawTarget()));

// standing, update every 1.1s.
else if (delta > 35.f && gpGlobals->curtime > m_flNextBodyUpdate)

m_flNextBodyUpdate = gpGlobals->curtime + 1.1f;
*/

static timer wait_perf{ };

float CAntiAim::GetFake(CUserCmd* pCmd, int nIndex, bool* pSendPacket)
{


	float retnAngle = 0.f;
	static Timer cmdTimer{};
	switch (nIndex) {
	case 1:
	{
		retnAngle = 0;
		break;
	}
	case 2:
	{
		retnAngle = bInvert ? 90 : -90;
		break;
	}
	case 3:
	{
		retnAngle = bInvert ? -90 : 90;
		break;
	}
	case 4:
	{
		if (!wait_perf.test_and_set(220))
		{
			retnAngle = bInvert ? -90 : 90;
		}
		else
		{
			retnAngle = bInvert ? 90 : -90;
		}
		break;
	}
	case 5:
	{
		retnAngle = fmod(I::GlobalVars->curtime * 720 * Vars::AntiHack::AntiAim::SpinSpeed.Value, 720);
		break;
	}
	case 6:
	{
		static bool hi = false;
		float first = bInvert ? -178 : 178;
		float second = bInvert ? 1 : -1;

		retnAngle = hi ? first : second;
		hi = !hi;
		break;
	}
	case 7:
	{
		static bool counter = false;
		static int counters = 0;
		if (counters == 5)
		{
			counters = 0;
			counter = !counter;
		}
		counters++;
		if (counter)
		{
			retnAngle = bInvert ? Vars::AntiHack::AntiAim::FakeFirst.Value : Vars::AntiHack::AntiAim::first.Value;

		}
		else
			retnAngle = bInvert ? Vars::AntiHack::AntiAim::FakeSecond.Value : Vars::AntiHack::AntiAim::second.Value;
		break;
	}

	}
	return retnAngle;


}

float CAntiAim::GetAnglePairPitch(int nIndex, CUserCmd* pCmd) {
	float retnAngles;

	switch (nIndex) {
	case 1: //fake fake up
	{
		retnAngles = -91;
		G::FakeViewAngles.x = -91;
		break;
	}
	case 2: //fake fake down
	{
		retnAngles = 91;
		G::FakeViewAngles.x = 91;
		break;
	}
	case 3:	//fake jitter "fakeup"
	{
		if (Vars::AntiHack::AntiAim::Roll.Value)
		{
			static bool flip = false;
			retnAngles = flip ? 271.f : -91.f;
			G::FakeViewAngles.x = flip ? 91 : -91;
			if (I::GlobalVars->tickcount % 3 != 0) {	//	dumb hack
				flip = !flip;
			}
		}
		else
		{
			retnAngles = 271.f;
			G::FakeViewAngles.x = 91;
		}
		break;
	}
	case 4:	//fake jitter "fakedown"
	{
		if (Vars::AntiHack::AntiAim::Roll.Value)
		{
			static bool flip = false;
			retnAngles = flip ? -271.f : 91.f;
			G::FakeViewAngles.x = flip ? -91 : 91;
			if (I::GlobalVars->tickcount % 3 != 0) {	//	dumb hack
				flip = !flip;
			}
		}
		else
		{
			retnAngles = -271.f;
			G::FakeViewAngles.x = -91;
		}
		break;
	}
	}
	return retnAngles;
}


bool IsOverlapping(float a, float b, float epsilon = 45.f)
{
	return std::abs(a - b) < epsilon;
}
 
void CAntiAim::Run(CUserCmd* pCmd, bool* pSendPacket)
{
	G::AAActive = false;
	G::RealViewAngles = G::ViewAngles;
	G::FakeViewAngles = G::ViewAngles;

	static KeyHelper rechargeKey{ &Vars::Misc::CL_Move::RechargeKey.Value };

	if (!Vars::AntiHack::AntiAim::Active.Value) { return; }

	if (const auto& pLocal = g_EntityCache.GetLocal()) {
		if (!pLocal->IsAlive()
			|| pLocal->IsTaunting()
			|| pLocal->IsInBumperKart()
			|| pLocal->IsAGhost()) {
			return;
		}

		const auto& pWeapon = g_EntityCache.GetWeapon();

		bool edge = (Vars::AntiHack::AntiAim::YawFake.Value == 6 || Vars::AntiHack::AntiAim::YawReal.Value == 6);

		static KeyHelper kInvert{ &Vars::AntiHack::AntiAim::ToggleKey.Value };
		bInvert = (kInvert.Pressed() ? !bInvert : bInvert);

		float flBaseYaw = 0;
		flBaseYaw = GetBaseYaw(0, pLocal, pCmd);

		if (pLocal->GetMoveType() == MOVETYPE_NOCLIP
			|| pLocal->GetMoveType() == MOVETYPE_LADDER
			|| pLocal->GetMoveType() == MOVETYPE_OBSERVER)
		{
			return;
		}
		static bool pos = false;


		if (pCmd->buttons & IN_ATTACK)
		{
			const float angPair = GetAnglePairPitch(Vars::AntiHack::AntiAim::Pitch.Value, pCmd);
			static bool flip = false;

			if (Vars::AntiHack::AntiAim::Pitch.Value == 2)
			{
				pCmd->viewangles.x = CalculateCustomRealPitch(pCmd->viewangles.x, true);
				G::RealViewAngles.x = angPair;
			}

			if (Vars::AntiHack::AntiAim::Pitch.Value == 4 && Vars::AntiHack::AntiAim::Roll.Value)
			{
				static bool flip = false;

				pCmd->viewangles.x = CalculateCustomRealPitch(pCmd->viewangles.x, flip ? true : false);
				G::RealViewAngles.x = angPair;
				if (I::GlobalVars->tickcount % 3 != 0) {
					flip = !flip;
				}
			}

			if (Vars::AntiHack::AntiAim::Pitch.Value == 3 && Vars::AntiHack::AntiAim::Roll.Value)
			{
				static bool flip = false;

				pCmd->viewangles.x = CalculateCustomRealPitch(pCmd->viewangles.x, flip ? false : true);
				G::RealViewAngles.x = angPair;
				if (I::GlobalVars->tickcount % 3 != 0) {
					flip = !flip;
				}
			}

			if (Vars::AntiHack::AntiAim::Pitch.Value == 1)
			{
				pCmd->viewangles.x = CalculateCustomRealPitch(pCmd->viewangles.x, false);
				G::RealViewAngles.x = angPair;
			}

			if (Vars::AntiHack::AntiAim::Pitch.Value == 3 && !Vars::AntiHack::AntiAim::Roll.Value)
			{
				pCmd->viewangles.x = CalculateCustomRealPitch(pCmd->viewangles.x, true);
				G::RealViewAngles.x = angPair;
			}

			if (Vars::AntiHack::AntiAim::Pitch.Value == 4 && !Vars::AntiHack::AntiAim::Roll.Value)
			{
				pCmd->viewangles.x = CalculateCustomRealPitch(pCmd->viewangles.x, false);
				G::RealViewAngles.x = angPair;
			}
		}


		if (G::IsAttacking)
		{
			return;
		}

		const Vec3 vOldAngles = pCmd->viewangles;
		const float fOldSideMove = pCmd->sidemove;
		const float fOldForwardMove = pCmd->forwardmove;

		if (Vars::AntiHack::AntiAim::Pitch.Value || G::RechargeQueued && !G::IsChoking || G::Recharging && (G::ShiftedTicks < 24) || rechargeKey.Down() && !G::RechargeQueued && (G::ShiftedTicks < 24)) {
			const float angPair = GetAnglePairPitch(Vars::AntiHack::AntiAim::Pitch.Value, pCmd);
			pCmd->viewangles.x = angPair;
			G::RealViewAngles.x = angPair;
		}

		if (Vars::Misc::CL_Move::Fakelag.Value)
		{
			auto distance = pLocal->GetVelocity().Length2D() * 0.015;
			int choked_ticks = std::ceilf(64 / distance);
			float shit = Vars::Misc::CL_Move::RetainFakelag.Value ? Vars::Misc::CL_Move::FakelagValue.Value - G::ShiftedTicks : std::min(choked_ticks, 22 - G::ShiftedTicks);

			if (const auto& pWeapon = g_EntityCache.GetWeapon())
			{
				if (pLocal->GetMoveType() != MOVETYPE_WALK || (pWeapon->GetWeaponID() == TF_WEAPON_MINIGUN && pCmd->buttons & IN_ATTACK2) && pLocal->OnSolid())
				{
					shit = 5;
				}
			}

			if (G::ShiftedTicks < 24 - shit)
			{
				if (I::ClientState->chokedcommands == shit)
				{
					const float angOffseter = flBaseYaw + GetFake(pCmd, Vars::AntiHack::AntiAim::YawFake.Value, pSendPacket);
					pCmd->viewangles.y = angOffseter;
					G::FakeViewAngles = pCmd->viewangles;
				}
				else
				{
					const float angOffset = flBaseYaw + GetReal(pCmd, Vars::AntiHack::AntiAim::YawReal.Value, pSendPacket);
					pCmd->viewangles.y = angOffset;
					G::RealViewAngles = pCmd->viewangles;
				}
			}
		}
		else
		{

			if (I::GlobalVars->tickcount % 3 != 0)
			{
				*pSendPacket = false;

				const float angOffset = flBaseYaw + GetReal(pCmd, Vars::AntiHack::AntiAim::YawReal.Value, pSendPacket);
				pCmd->viewangles.y = angOffset;
				G::RealViewAngles = pCmd->viewangles;
			}
			else
			{

				*pSendPacket = true;

				const float angOffseter = flBaseYaw + GetFake(pCmd, Vars::AntiHack::AntiAim::YawFake.Value, pSendPacket);
				pCmd->viewangles.y = angOffseter;
				G::FakeViewAngles = pCmd->viewangles;
			}
		}


		bPacketFlip = *pSendPacket;

		G::AAActive = true;
		FixMovement(pCmd, vOldAngles);
	}
}



                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 