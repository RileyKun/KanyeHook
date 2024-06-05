#include "Misc.h"
#include "../Vars.h"

#include "../../Utils/Timer/Timer.hpp"
#include "../Aimbot/AimbotGlobal/AimbotGlobal.h"
#include "../Commands/Commands.h"
#include "../AntiHack/AntiAim.h"


#define CheckIfNonValidNumber(x) (fpclassify(x) == FP_INFINITE || fpclassify(x) == FP_NAN || fpclassify(x) == FP_SUBNORMAL)


extern int attackStringW;
extern int attackStringH;

void CMisc::Run(CUserCmd* pCmd)
{
	Vector OriginalView;
	if (const auto& pLocal = g_EntityCache.GetLocal())
	{
		const auto& pWeapon = pLocal->GetActiveWeapon();
		AutoJump(pCmd, pLocal);
		AutoStrafe(pCmd, pLocal, OriginalView);
		AntiBackstab(pLocal, pCmd);
		AutoSwitch(pCmd, pLocal);
		//if (I::EngineClient->IsInGame() && I::EngineClient->GetLocalPlayer() && !g_EntityCache.GetLocal()->IsAlive()) {
		//	auto kv = new KeyValues("MVM_Revive_Response");
		//	kv->SetInt("accepted", 1);
		//	I::EngineClient->ServerCmdKeyValues(kv);
		//
		//	KeyValues* kver = new KeyValues("AutoBalanceVolunteerReply");
		//	kver->SetBool("response", false);
		//	I::EngineClient->ServerCmdKeyValues(kver);
		//}

		AutoPeek(pCmd, pLocal);



	//	NoSpread(pCmd, pWeapon);
	}

	CheatsBypass();
	Teleport(pCmd);


}

FORCEINLINE float DotProduc(const Vector& a, const Vector& b)
{

	return (a.x * b.x + a.y * b.y + a.z * b.z);
}



void CMisc::RunLate(CUserCmd* pCmd)
{
	if (const auto& pLocal = g_EntityCache.GetLocal())
	{
		AccurateMovement(pCmd, pLocal);
	  FastStop(pCmd, pLocal);
	}
}

void CMisc::AntiBackstab(CBaseEntity* pLocal, CUserCmd* pCmd)
{
	G::AvoidingBackstab = false;
	Vec3 vTargetPos;

	if (!pLocal->IsAlive() || pLocal->IsStunned() || pLocal->IsInBumperKart() || pLocal->IsAGhost())
	{
		return;
	}

	const Vec3 vLocalPos = pLocal->GetWorldSpaceCenter();
	CBaseEntity* target = nullptr;
	bool IsBack = false;


	float Timer;


	for (const auto& pEnemy : g_EntityCache.GetGroup(EGroupType::PLAYERS_ENEMIES))
	{
		if (!pEnemy || !pEnemy->IsAlive() || pEnemy->GetClassNum() != CLASS_SPY || pEnemy->IsCloaked() || pEnemy->IsAGhost())
		{
			continue;
		}

		Vec3 vEnemyPos = pEnemy->GetWorldSpaceCenter();
		if (!Utils::VisPos(pLocal, pEnemy, vLocalPos, vEnemyPos)) { continue; }

		// Get a vector from owner origin to target origin
		Vector vecToTarget;
		vecToTarget = pEnemy->GetWorldSpaceCenter() - pLocal->GetShootPos();
		vecToTarget.z = 0.0f;
		vecToTarget.NormalizeInPlace();

		// Get owner forward view vector
		Vector vecOwnerForward;
		Math::AngleVectors(Math::CalcAngle(pEnemy->GetShootPos(), pLocal->GetWorldSpaceCenter()), &vecOwnerForward, NULL, NULL);
		vecOwnerForward.z = 0.0f;
		vecOwnerForward.NormalizeInPlace();

		// Get target forward view vector
		Vector vecTargetForward;
		Math::AngleVectors(pEnemy->GetEyeAngles(), &vecTargetForward, NULL, NULL);
		vecTargetForward.z = 0.0f;
		vecTargetForward.NormalizeInPlace();

		// Make sure owner is behind, facing and aiming at target's back
		float flPosVsTargetViewDot = DotProduc(vecToTarget, vecTargetForward);	// Behind?
		float flPosVsOwnerViewDot = DotProduc(vecToTarget, vecOwnerForward);		// Facing?
		float flViewAnglesDot = DotProduc(vecTargetForward, vecOwnerForward);	// Facestab?

		if (!target && vLocalPos.DistTo(vEnemyPos) < 180.f)
		{
			target = pEnemy;
			vTargetPos = target->GetWorldSpaceCenter();
		}
		else if (vLocalPos.DistTo(vEnemyPos) < vLocalPos.DistTo(vTargetPos) && vLocalPos.DistTo(vEnemyPos) < 180.f)
		{
			target = pEnemy;
			vTargetPos = target->GetWorldSpaceCenter();
		}

		IsBack = (flPosVsTargetViewDot > 0.f && flPosVsOwnerViewDot > 0.5 && flViewAnglesDot > -0.3f);

		if (IsBack)
		{
			Timer = 67;
		}
	}

	
	if (target && Timer)
	{
		Timer--;
		vTargetPos = target->GetWorldSpaceCenter();
		const Vec3 vAngleToSpy = Math::CalcAngle(vLocalPos, vTargetPos);
		G::AvoidingBackstab = true;
		Utils::FixMovement(pCmd, vAngleToSpy);
		pCmd->viewangles = vAngleToSpy;
	}
}

void CMisc::CheatsBypass()
{


	static bool cheatset = false;
	ConVar* sv_cheats = g_ConVars.FindVar("sv_cheats");
	if (Vars::Misc::CheatsBypass.Value && sv_cheats)
	{
		sv_cheats->SetValue(1);
		cheatset = true;
	}
	else
	{
		if (cheatset)
		{
			sv_cheats->SetValue(0);
			cheatset = false;
		}
	}


}

void CMisc::Teleport(const CUserCmd* pCmd)
{
	// Stupid
	static KeyHelper tpKey{ &Vars::Misc::CL_Move::TeleportKey.Value };
	if (tpKey.Down())
	{
		if (Vars::Misc::CL_Move::TeleportMode.Value == 0 && G::TickShiftQueue == 0 && G::ShiftedTicks > 0)
		{
			
			G::TickShiftQueue = Vars::Misc::CL_Move::DTTicks.Value;
		} 
		
	}
	
}

void CMisc::AccurateMovement(CUserCmd* pCmd, CBaseEntity* pLocal)
{
	if (!Vars::Misc::AccurateMovement.Value)
	{
		return;
	}

	if (!pLocal->IsAlive()
		|| pLocal->IsSwimming()
		|| pLocal->IsInBumperKart()
		|| pLocal->IsAGhost()
		|| !pLocal->OnSolid())
	{
		return;
	}

	if (pLocal->GetMoveType() == MOVETYPE_NOCLIP
		|| pLocal->GetMoveType() == MOVETYPE_LADDER
		|| pLocal->GetMoveType() == MOVETYPE_OBSERVER)
	{
		return;
	}

	if (pCmd->buttons & (IN_JUMP | IN_MOVELEFT | IN_MOVERIGHT | IN_FORWARD | IN_BACK))
	{
		return;
	}

	const float Speed = pLocal->GetVecVelocity().Length2D();
	const float SpeedLimit = Vars::Debug::DebugBool.Value ? 2.f : 10.f;	//	does some fucky stuff

	if (Speed > 0)
	{
		switch (Vars::Misc::AccurateMovement.Value) {
		case 1: {
			Vec3 direction = pLocal->GetVecVelocity().toAngle();
			direction.y = pCmd->viewangles.y - direction.y;
			const Vec3 negatedDirection = direction.fromAngle() * -Speed;
			pCmd->forwardmove = negatedDirection.x;
			pCmd->sidemove = negatedDirection.y;
		
			break;
		}
		case 2: {
			G::ShouldStop = true;
			break;
		}
		}
	}
}

void CMisc::AutoJump(CUserCmd* pCmd, CBaseEntity* pLocal)
{
	if (!Vars::Misc::AutoJump.Value
		|| !pLocal->IsAlive()
		|| pLocal->IsSwimming()
		|| pLocal->IsInBumperKart()
		|| pLocal->IsAGhost())
	{
		return;
	}

	if (pLocal->GetMoveType() == MOVETYPE_NOCLIP
		|| pLocal->GetMoveType() == MOVETYPE_LADDER
		|| pLocal->GetMoveType() == MOVETYPE_OBSERVER)
	{
		return;
	}


	static bool s_bState = false;
	static Timer cmdTimer{};
	if (pCmd->buttons & IN_JUMP)
	{
		

		if (!s_bState && !pLocal->OnSolid())
		{
			pCmd->buttons &= ~IN_JUMP;
		}
		else if (s_bState)
		{
			s_bState = false;
		}
	}
	else if (!s_bState)
	{
		s_bState = true;
	}


}

inline std::vector<Vector> GenerateSpherePoints(const Vector& center, float radius, int numSamples)
{
	std::vector<Vector> points;
	points.reserve(numSamples);

	const float goldenAngle = 2.39996323f;  // precompute the golden angle

	for (int i = 0; i < numSamples; ++i)
	{
		const float inclination = acosf(1.0f - 2.0f * i / (numSamples - 1));  // compute the inclination angle
		const float azimuth = goldenAngle * i;  // compute the azimuth angle using the golden angle

		const float sin_inc = sinf(inclination);
		const float cos_inc = cosf(inclination);
		const float sin_azi = sinf(azimuth);
		const float cos_azi = cosf(azimuth);

		// compute the sample point on the unit sphere
		const Vector sample = Vector(cos_azi * sin_inc, sin_azi * sin_inc, cos_inc) * radius;

		// translate the point to the desired center
		points.push_back(center + sample);
	}

	return points;
}



void CMisc::AutoSwitch(CUserCmd* pCmd, CBaseEntity* pLocal)
{

	if (!Vars::Misc::CL_Move::Fast.Value)
	{
		return;
	}

	if (Vars::AntiHack::AntiAim::Active.Value)
	{
		return;
	}

	if (G::Recharging || G::RechargeQueued || G::Frozen)
	{
		return;
	}

	if (!pLocal->IsAlive() || pLocal->IsSwimming() || pLocal->IsAGhost() || !pLocal->OnSolid() || G::IsAttacking)
	{
		return;
	}

	if (pLocal->IsCharging())
	{	//	demoman charge
		return;
	}

	if (pLocal->GetMoveType() == MOVETYPE_NOCLIP
		|| pLocal->GetMoveType() == MOVETYPE_LADDER
		|| pLocal->GetMoveType() == MOVETYPE_OBSERVER)
	{
		return;
	}

	const int maxSpeed = std::min(pLocal->GetMaxSpeed() * (pCmd->forwardmove < 0 ? .9f : 1.f) - 1, 510.f); //	get our max speed, then if we are going backwards, reduce it.
	const float curSpeed = pLocal->GetVecVelocity().Length2D();

	if (curSpeed > maxSpeed)
	{
		return;	//	no need to accelerate if we are moving at our max speed
	}

	if (pLocal->GetClassNum() == ETFClass::CLASS_HEAVY && pCmd->buttons & IN_ATTACK2 && pLocal->IsDucking())
	{
		return;
	}

	if (pCmd->buttons & (IN_MOVELEFT | IN_MOVERIGHT | IN_FORWARD | IN_BACK))
	{
		const Vec3 vecMove(pCmd->forwardmove, pCmd->sidemove, 0.0f);
		const float flLength = vecMove.Length();
		Vec3 angMoveReverse;
		Math::VectorAngles(vecMove * -1.f, angMoveReverse);
		pCmd->viewangles.x = 89;
		pCmd->forwardmove = -flLength;
		pCmd->sidemove = 0.0f;
		pCmd->viewangles.y = fmodf(pCmd->viewangles.y - angMoveReverse.y, 360.0f);	//	this doesn't have to be clamped inbetween 180 and -180 because the engine automatically fixes it.
		pCmd->viewangles.z = 270.f;
	}
}

void CMisc::AutoStrafe(CUserCmd* pCmd, CBaseEntity* pLocal, Vector& OriginalView)
{
	if (!Vars::Misc::AutoStrafe.Value)
	{
		return;
	}



	if (!pLocal->IsAlive()
		|| pLocal->IsSwimming()
		|| pLocal->IsInBumperKart()
		|| pLocal->IsAGhost()
		|| pLocal->OnSolid())
	{
		return;
	}

	if (pLocal->GetMoveType() == MOVETYPE_NOCLIP
		|| pLocal->GetMoveType() == MOVETYPE_LADDER
		|| pLocal->GetMoveType() == MOVETYPE_OBSERVER)
	{
		return;
	}

	static auto cl_sidespeed = g_ConVars.FindVar("cl_sidespeed");
	if (!cl_sidespeed || !cl_sidespeed->GetFloat())
	{
		return;
	}
	

	static bool wasJumping = false;
	const bool isJumping = pCmd->buttons & IN_JUMP;

	switch (Vars::Misc::AutoStrafe.Value)
	{
	default:
		break;
	case 1:
	{
	
		break;
	}
	case 2:
	{
		static Timer cmdTimer{};

		static bool WasJumping = false;
		bool IsJumping = pCmd->buttons & IN_JUMP;

		if (!pLocal || !pLocal->IsAlive() || pLocal->GetWaterLevel() > 1)
			return;

		if (pLocal->GetMoveType() == MOVETYPE_NOCLIP
			|| pLocal->GetMoveType() == MOVETYPE_LADDER
			|| pLocal->GetMoveType() == MOVETYPE_OBSERVER)
		{
			return;
		}

		if (Vars::Misc::DuckJump.Value)
		{
			if (pCmd->buttons & IN_DUCK)
			{

			}
			else
			{
				if (pLocal->m_flFallVelocity() < 0)
				{
					if (pLocal->IsDucking() && pLocal->m_flDucktime() < 1001)
						pCmd->buttons &= ~IN_DUCK;
					else
						pCmd->buttons |= IN_DUCK;
				}
			}
		}

		if (!(pLocal->GetFlags() & FL_ONGROUND) && (!IsJumping || WasJumping)) {
			if (pLocal->GetFlags() & (1 << 11))
				return;

			float SideMove = pCmd->sidemove;
			float ForwardMove = pCmd->forwardmove;

			Vector Forward = {}, Right = {};
			Math::AngleVectors(pCmd->viewangles, &Forward, &Right, nullptr);

			Forward.z = Right.z = 0.0f;

			Forward.Normalize();
			Right.Normalize();

			Vector WishDir = {};
			Math::VectorAngles(
				{ (Forward.x * ForwardMove) + (Right.x * SideMove),
				(Forward.y * ForwardMove) +
				(Right.y * SideMove), 0.0f },
				WishDir);

			Vector CurDir = {};
			Math::VectorAngles(pLocal->GetVelocity(), CurDir);

			float DirDelta = Math::NormalizeAngle(WishDir.y - CurDir.y);
			float TurnScale = Math::RemapValClamped((25 / 100.f), 0.0f, 1.0f, 0.9f, 1.0f);
			float Rotation = DEG2RAD((DirDelta > 0.0f ? -90.0f : 90.f) + (DirDelta * TurnScale));

			float CosRot = cosf(Rotation);
			float SinRot = sinf(Rotation);

			pCmd->forwardmove = (CosRot * ForwardMove) - (SinRot * SideMove);
			pCmd->sidemove = (SinRot * ForwardMove) + (CosRot * SideMove);
		}

		WasJumping = IsJumping;
		break;
	}
	}

};

void CMisc::FastStop(CUserCmd* pCmd, CBaseEntity* pLocal)
{
	if (pLocal && pLocal->IsAlive() && !pLocal->IsTaunting() && !pLocal->IsStunned() && pLocal->GetVelocity().Length2D() > 0.0f) {
		if (Vars::Misc::CL_Move::AntiWarp.Value)
		{
			return;
		}
		else
		{
			if (G::DT)
			{
				G::ShouldStop = true;
			}
		}
	}
}


bool CanAttack(CBaseEntity* pLocal, const Vec3& pPos)
{
	if (const auto pWeapon = pLocal->GetActiveWeapon())
	{
		if (!G::WeaponCanHeadShot && pLocal->IsScoped()) { return false; }
		if (!pWeapon->CanShoot(G::LastUserCmd,pLocal)) { return false; }
		
		for (const auto& target : g_EntityCache.GetGroup(EGroupType::PLAYERS_ENEMIES))
		{
			if (!target->IsAlive()) { continue; }
			if (F::AimbotGlobal.ShouldIgnore(target)) { continue; }

			if (Utils::VisPos(pLocal, target, pPos, target->GetHitboxPos(HITBOX_HEAD)))
			{
				return true;
			}
		}
	}

	return false;
}

bool Vis(CBaseEntity* pLocal, const Vec3& pPos)
{
	if (const auto pWeapon = pLocal->GetActiveWeapon())
	{
		for (const auto& target : g_EntityCache.GetGroup(EGroupType::PLAYERS_ENEMIES))
		{
			if (!target->IsAlive()) { continue; }
			if (F::AimbotGlobal.ShouldIgnore(target)) { continue; }

			if (Utils::VisPos(pLocal, target, pPos, target->GetWorldSpaceCenter()))
			{
				return true;
			}
		}
	}

	return false;
}

__inline Vector ComputeMoves(const CUserCmd* pCmd, CBaseEntity* pLocal, Vec3& a, Vec3& b)
{
	const Vec3 diff = (b - a);
	if (diff.Length() == 0.0f)
	{
		return { 0.0f, 0.0f, 0.0f };
	}
	const float x = diff.x;
	const float y = diff.y;
	const Vec3 vSilent(x, y, 0);
	Vec3 ang;
	Math::VectorAngles(vSilent, ang);
	const float yaw = DEG2RAD(ang.y - pCmd->viewangles.y);
	const float pitch = DEG2RAD(ang.x - pCmd->viewangles.x);
	Vec3 move = { cos(yaw) * 450.0f, -sin(yaw) * 450.0f, -cos(pitch) * 450.0f };

	// Only apply upmove in water
	if (!(I::EngineTrace->GetPointContents(pLocal->GetEyePosition()) & CONTENTS_WATER))
	{
		move.z = pCmd->upmove;
	}
	return move;
}


__inline void WalkTos(CUserCmd* pCmd, CBaseEntity* pLocal, Vec3& a, Vec3& b, float scale)
{
	// Calculate how to get to a vector
	const auto result = ComputeMoves(pCmd, pLocal, a, b);

	// Push our move to usercmd
	pCmd->forwardmove = result.x * scale;
	pCmd->sidemove = result.y * scale;
	pCmd->upmove = result.z * scale;
}


void CMisc::AutoPeek(CUserCmd* pCmd, CBaseEntity* pLocal)
{
	if (!G::WeaponCanAttack || !GetAsyncKeyState(Vars::Misc::CL_Move::AutoPeekKey.Value))
	{
		return;
	}


	if (pLocal->GetClassNum() != CLASS_SOLDIER || !pLocal->OnSolid() || pLocal->IsDucking() || (pLocal->GetHealth() < 60)) // health check is meh, you could check the damage of the launcher, and find the damage at distance from explosion, but that's a lot of work, and it will just be ~40 anyway.
	{
		return;
	}

	if (const auto& pWeapon = g_EntityCache.GetWeapon())
	{
		if (pWeapon->IsInReload())
		{
			pCmd->buttons |= IN_ATTACK;
			return;
		}
		if (pCmd->buttons & IN_ATTACK)
		{
			pCmd->buttons &= ~IN_ATTACK;
		}

		if (G::CurItemDefIndex == Soldier_m_TheBeggarsBazooka
			|| G::CurItemDefIndex == Soldier_m_TheCowMangler5000
			|| pWeapon->GetSlot() != SLOT_PRIMARY)
		{
			return;
		}

		if (pLocal->GetViewOffset().z < 60.05f)
		{
			pCmd->buttons |= IN_ATTACK | IN_JUMP;

			const Vec3 vVelocity = pLocal->GetVelocity();
			Vec3 vAngles = { vVelocity.IsZero() ? 89.0f : 40.0f, Math::VelocityToAngles(vVelocity).y - 180.0f, 0.0f };

			if (G::CurItemDefIndex != Soldier_m_TheOriginal && !vVelocity.IsZero())
			{
				Vec3 vForward = {}, vRight = {}, vUp = {};
				Math::AngleVectors(vAngles, &vForward, &vRight, &vUp);
				Math::VectorAngles((vForward * 23.5f) + (vRight * -5.6f) + (vUp * -3.0f), vAngles);
			}

			Math::ClampAngles(vAngles);
			pCmd->viewangles = vAngles;
		}
		else
		{
			pCmd->buttons |= IN_DUCK;
		}
	}
}
