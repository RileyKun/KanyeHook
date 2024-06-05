#include "AutoBlast.h"

#include "../AutoGlobal/AutoGlobal.h"
#include "../../Vars.h"
#include "../../Backtrack/Backtrack.h"

void CAutoAirblast::Run(CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon, CUserCmd* pCmd)
{
	if (!Vars::Triggerbot::Blast::Active.Value || !G::WeaponCanSecondaryAttack)
	{
		return;
	}

	id = pWeapon->GetWeaponID();

	if (id != TF_WEAPON_FLAMETHROWER && id != TF_WEAPON_FLAME_BALL)
	{
		return;
	}

	if (G::CurItemDefIndex == Pyro_m_ThePhlogistinator)
	{
		return;
	}

	if (Vars::Triggerbot::Blast::DisableOnAttack.Value && pCmd->buttons & IN_ATTACK)
		return;

	if (const auto& pNet = I::EngineClient->GetNetChannelInfo())
	{
		const Vec3 vEyePos = pLocal->GetEyePosition();
		const Vec3 WorldCenter = pLocal->GetWorldSpaceCenter();
		float flLatency = (pNet->GetLatency(FLOW_OUTGOING));

		// pretty sure the game shows the predicted position of projectiles so accounting for incoming ping seems useless.
		bool bShouldBlast = false;

		for (const auto& pProjectile : g_EntityCache.GetGroup(EGroupType::WORLD_PROJECTILES))
		{
			if (pProjectile->GetTeamNum() == pLocal->GetTeamNum())
			{
				continue; //Ignore team's projectiles
			}

			switch (pProjectile->GetClassID())
			{
			case ETFClassID::CTFGrenadePipebombProjectile:
			{
				break;
			}

			case ETFClassID::CTFProjectile_Arrow:
			{
				if (pProjectile->GetVelocity().IsZero())
				{
					continue; //Ignore arrows with no velocity / not moving
				}
				break;
			}

			default: break;
			}
			
	

			Vec3 vPredicted = (pProjectile->GetAbsOrigin() + pProjectile->GetVelocity().Scale(flLatency));

		

			if (vEyePos.DistTo(vPredicted) <= 256.0f && Utils::VisPos(pLocal, pProjectile, vEyePos, vPredicted))
			{
				G::SilentTime = true;
				G::IsAttacking = true;
				pCmd->viewangles = Math::CalcAngle(vEyePos, vPredicted);
				pCmd->buttons |= IN_ATTACK2;
				break;
			}
		}

	}
}
