#include "../Hooks.h"
#include "../../Features/Aimbot/ProjectileAim/movementsim.h"
#include "../../Features/Visuals/Visuals.h"

#include "../../Features/Menu/Playerlist/Playerlist.h"
#include "../../Features/Backtrack/Backtrack.h"
#include "../../Features/lagcomp/lagcomp.h"
#include "../../Features/Visuals/FakeAngleManager/FakeAng.h"
#include "../../Features/AntiHack/Resolver.h"
#include "../../Features/Aimbot/ProjectileAim/ProjSim.h"
#include "../../Features/Aimbot/ProjectileAim/projectile.h"

#include "../../Features/seedprediction/seed.hpp"

void meme(CBaseEntity* pPlayer, CBaseCombatWeapon* pWeapon, Vector ViewAngles, Vector vecOffset, Vector* vecSrc, QAngle* angForward, float flEndDist /* = 2000 */)
{
	ConVar* cl_flipviewmodels = g_ConVars.cl_flipviewmodels;

	if (!cl_flipviewmodels)
		return;

	if (cl_flipviewmodels->GetBool())
		vecOffset.y *= -1.0f;

	if (pWeapon->GetItemDefIndex() == Soldier_m_TheOriginal)
	{
		vecOffset.y = 0;
	}

	Vector vecForward, vecRight, vecUp;
	Math::AngleVectors(ViewAngles, &vecForward, &vecRight, &vecUp);

	Vector vecShootPos = pPlayer->GetShootPos();

	// Estimate end point
	Vector endPos = vecShootPos + vecForward * flEndDist;

	// Trace forward and find what's in front of us, and aim at that
	CGameTrace tr;

	CTraceFilterWorldAndPropsOnly traceFilter;
	traceFilter.ShouldHitEntity(pPlayer, COLLISION_GROUP_NONE);

	Utils::Trace(vecShootPos, endPos, MASK_SOLID, &traceFilter, &tr);

	// Offset actual start point
	*vecSrc = vecShootPos + (vecForward * vecOffset.x) + (vecRight * vecOffset.y) + (vecUp * vecOffset.z);

	// Find angles that will get us to our desired end point
	// Only use the trace end if it wasn't too close, which results
	// in visually bizarre forward angles
	if (tr.flFraction > 0.1)
	{
		Math::VectorAngles(tr.vEndPos - *vecSrc, *angForward);
	}
	else
	{
		Math::VectorAngles(endPos - *vecSrc, *angForward);
	}
}

#include "../../Features/Aimbot/ProjectileAim/ProjSim.h"




MAKE_HOOK(BaseClientDLL_FrameStageNotify, Utils::GetVFuncPtr(I::BaseClientDLL, 35), void, __fastcall,
		  void* ecx, void* edx, EClientFrameStage curStage)
{
	switch (curStage)
	{
	case EClientFrameStage::FRAME_RENDER_START:
	{
		G::PunchAngles = Vec3();

		if (const auto& pLocal = g_EntityCache.GetLocal())
		{
			if (Vars::Visuals::RemovePunch.Value)
			{
				G::PunchAngles = pLocal->GetPunchAngles();
				//Store punch angles to be compesnsated for in aim
				pLocal->ClearPunchAngle(); //Clear punch angles for visual no-recoil
			}

	
		}

		break;
	}
	}

	Hook.Original<FN>()(ecx, edx, curStage);

	switch (curStage)
	{
		case EClientFrameStage::FRAME_NET_UPDATE_START:
		{
			g_EntityCache.Clear();

			if (const auto& pLocal = g_EntityCache.GetLocal())
			{
			

			}
			break;
		}

		case EClientFrameStage::FRAME_NET_UPDATE_END:
		{
			CUserCmd* pCmd = {};
			g_EntityCache.Fill();
			F::MoveSim.FillVelocities();
			
			for (auto ent : g_EntityCache.GetGroup(EGroupType::PLAYERS_ALL))
			{
				if (!ent || ent == g_EntityCache.GetLocal())
					continue; // local player managed in CPrediction_RunCommand

				auto diff = std::clamp((TIME_TO_TICKS(fabs(ent->GetSimulationTime() - ent->GetOldSimulationTime()))), 0, 22);
			
				float flOldFrameTime = I::GlobalVars->frametime;
				int nOldSequence = ent->m_nSequence();
				float flOldCycle = ent->m_flCycle();
				auto pOldPoseParams = ent->GetPoseParam();
	
				auto Restore = [&]()
					{
						I::GlobalVars->frametime = flOldFrameTime;
						ent->m_nSequence() = nOldSequence;
						ent->m_flCycle() = flOldCycle;
						ent->SetPoseParam(pOldPoseParams);
					};


				for (int i = 0; i < diff; i++)
				{
					G::UpdateAnim = true;
					ent->UpdateAnim();
					G::UpdateAnim = false; Restore();
				}

				I::GlobalVars->frametime = I::Prediction->m_bEnginePaused ? 0.0f : 0.015;

				Restore();
			}


			G::LocalSpectated = false;

			if (const auto& pLocal = g_EntityCache.GetLocal())
			{
				for (const auto& teammate : g_EntityCache.GetGroup(EGroupType::PLAYERS_TEAMMATES))
				{
					if (teammate->IsAlive() || g_EntityCache.IsFriend(teammate->GetIndex()))
					{
						continue;
					}

					const CBaseEntity* pObservedPlayer = I::ClientEntityList->GetClientEntityFromHandle(teammate->GetObserverTarget());

					if (pObservedPlayer == pLocal)
					{
						G::LocalSpectated = true;
						break;
					}
				}
			}


	

			for (const auto& pLocal : g_EntityCache.GetGroup(EGroupType::PLAYERS_ALL))
			{
				//pLocal->GetAnimState()->DoAnimationEvent()


				F::Backtrack.Run(pLocal);
			}


			for (const auto& pLocal : g_EntityCache.GetGroup(EGroupType::PLAYERS_ENEMIES))
			{
		           


				const VelFixRecord record = { pLocal->GetAbsOrigin(), pLocal->m_fFlags(), pLocal->GetSimulationTime() };
				G::VelFixRecords[pLocal] = record;

		

			}

			F::PlayerList.UpdatePlayers();
			break;
		}

	}
}