#include "Aimbot.h"
#include "../Vars.h"
#include "AimbotHitscan/AimbotHitscan.h"
#include "AimbotMelee/AimbotMelee.h"
#include "ProjectileAim/projectile.h"

bool CAimbot::ShouldRun(CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon)
{
	// Don't run while freecam is active
	if (G::FreecamActive) { return false; }

	// Don't run if aimbot is disabled
	if (!Vars::Aimbot::Global::Active.Value) { return false; }

	// Don't run if we are frozen in place.
	if (G::Frozen) { return false; }

	if (!pLocal->IsAlive()
		|| pLocal->IsTaunting()
		|| pLocal->IsBonked()
		|| pLocal->GetFeignDeathReady()
		|| pLocal->IsCloaked()
		|| pLocal->IsInBumperKart()
		|| pLocal->IsAGhost())
	{
		return false;
	}

	switch (G::CurItemDefIndex)
	{
	case Soldier_m_RocketJumper:
	case Demoman_s_StickyJumper: return false;
	default: break;
	}

	switch (pWeapon->GetWeaponID())
	{
	case TF_WEAPON_PDA:
	case TF_WEAPON_PDA_ENGINEER_BUILD:
	case TF_WEAPON_PDA_ENGINEER_DESTROY:
	case TF_WEAPON_PDA_SPY:
	case TF_WEAPON_LUNCHBOX:
	case TF_WEAPON_PDA_SPY_BUILD:
	case TF_WEAPON_BUILDER:
	case TF_WEAPON_INVIS:
	case TF_WEAPON_BUFF_ITEM:
		{
			return false;
		}

	default: break;
	}

	return true;
}

void CAimbot::Run(CUserCmd* pCmd)
{
	G::CurrentTargetIdx = 0;
	G::PredictedPos = Vec3();
	G::HitscanRunning = false;
	G::HitscanSilentActive = false;
	G::ProjectileSilentActive = false;
	G::AimPos = Vec3();

	const auto pLocal = g_EntityCache.GetLocal();
	const auto pWeapon = g_EntityCache.GetWeapon();
	if (!pLocal || !pWeapon) { return; }

	if (!ShouldRun(pLocal, pWeapon)) { return; }

	switch (G::CurWeaponType)
	{
	case EWeaponType::HITSCAN:
		{
			F::AimbotHitscan.Run(pLocal, pWeapon, pCmd);
			break;
		}
	case EWeaponType::MELEE:
		{
			F::AimbotMelee.Run(pLocal, pWeapon, pCmd);
			break;
		}
	case EWeaponType::PROJECTILE:
	{
		F::AimbotProjectile.Run(pLocal,pWeapon,pCmd);
		break;
	}

	default: break;
	}
}
