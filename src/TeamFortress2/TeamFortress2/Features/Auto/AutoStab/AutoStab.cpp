#include "AutoStab.h"

#include "../../Vars.h"
#include "../../Backtrack/Backtrack.h"
#include "../../Visuals/Visuals.h"

FORCEINLINE float DotProduct(const Vector& a, const Vector& b)
{

	return (a.x * b.x + a.y * b.y + a.z * b.z);
}

bool CAutoStab::CanBackstab(const Vec3& vSrc, const Vec3& vDst, Vec3 vWSCDelta)
{
	Vector vecToTarget;
	vecToTarget = vWSCDelta;
	vecToTarget.z = 0.0f;
	vecToTarget.NormalizeInPlace();

	// Get owner forward view vector
	Vector vecOwnerForward;
	Math::AngleVectors(vSrc, &vecOwnerForward, NULL, NULL);
	vecOwnerForward.z = 0.0f;
	vecOwnerForward.NormalizeInPlace();

	// Get target forward view vector
	Vector vecTargetForward;
	Math::AngleVectors(vDst, &vecTargetForward, NULL, NULL);
	vecTargetForward.z = 0.0f;
	vecTargetForward.NormalizeInPlace();

	// Make sure owner is behind, facing and aiming at target's back
	float flPosVsTargetViewDot = DotProduct(vecToTarget, vecTargetForward);	// Behind?
	float flPosVsOwnerViewDot = DotProduct(vecToTarget, vecOwnerForward);		// Facing?
	float flViewAnglesDot = DotProduct(vecTargetForward, vecOwnerForward);	// Facestab?

	return (flPosVsTargetViewDot > 0.f && flPosVsOwnerViewDot > 0.5 && flViewAnglesDot > -0.3f);
}

bool CAutoStab::TraceMelee(CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon, const Vec3& vViewAngles,
	CBaseEntity** pEntityOut)
{
	float flRange = 48.0f;

	if (flRange <= 0.0f)
	{
		return false;
	}

	auto vForward = Vec3();
	Math::AngleVectors(vViewAngles, &vForward);
	Vec3 vTraceStart = pLocal->GetShootPos();
	Vec3 vTraceEnd = (vTraceStart + (vForward * flRange));

	CGameTrace Trace = {};
	CTraceFilterHitscan Filter = {};
	Filter.pSkip = pLocal;
	Utils::TraceHull(vTraceStart, vTraceEnd, { -18.0f, -18.0f, -18.0f }, { 18.0f, 18.0f, 18.0f }, MASK_SOLID, &Filter, &Trace);
	if (IsEntityValid(pLocal, Trace.entity))
	{
		if (pEntityOut && !*pEntityOut)
		{
			*pEntityOut = Trace.entity;
		}

		return true;
	}

	return false;
}

bool CAutoStab::IsEntityValid(CBaseEntity* pLocal, CBaseEntity* pEntity)
{
	if (!pEntity || !pEntity->IsAlive() || pEntity->GetTeamNum() == pLocal->GetTeamNum() || !pEntity->IsPlayer())
		return false;

	if (F::AutoGlobal.ShouldIgnore(pEntity)) { return false; }

	return true;
}

void CAutoStab::RunLegit(CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon, CUserCmd* pCmd)
{
	CBaseEntity* pEnemy = nullptr;

	if (!TraceMelee(pLocal, pWeapon, pCmd->viewangles, &pEnemy))
	{
		return;
	}

	if (Vars::Triggerbot::Stab::IgnRazor.Value && pEnemy->GetClassNum() == CLASS_SNIPER &&
		pEnemy->GetWeaponFromSlot(SLOT_SECONDARY)->GetItemDefIndex() == Sniper_s_TheRazorback)
	{
		return;
	}

	if (!CanBackstab(pCmd->viewangles, pEnemy->GetEyeAngles(),
		(pEnemy->GetWorldSpaceCenter() - pLocal->GetWorldSpaceCenter())))
	{
		return;
	}

	pCmd->buttons |= IN_ATTACK;
	m_bShouldDisguise = true;

	if (Vars::Misc::DisableInterpolation.Value)
	{
		pCmd->tick_count = TIME_TO_TICKS(pEnemy->GetSimulationTime() + G::LerpTime);
	}
}

void CAutoStab::RunRage(CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon, CUserCmd* pCmd)
{
	for (const auto& pEnemy : g_EntityCache.GetGroup(EGroupType::PLAYERS_ENEMIES))
	{
		const auto& pEnemyWeapon = pEnemy->GetWeaponFromSlot(SLOT_SECONDARY);
		if (Vars::Triggerbot::Stab::IgnRazor.Value && pEnemy->GetClassNum() == CLASS_SNIPER &&
			pEnemyWeapon && pEnemyWeapon->GetItemDefIndex() == Sniper_s_TheRazorback)
		{
			continue;
		}

		if (!IsEntityValid(pLocal, pEnemy)) { continue; }

		CBaseEntity* pTraceEnemy = nullptr;

		Vec3 vAngleTo = Math::CalcAngle(pLocal->GetShootPos(), pEnemy->GetHitboxPos(HITBOX_PELVIS));

		const auto& pRecords = F::Backtrack.GetPlayerRecords(pEnemy->GetIndex());
		const bool bBacktrackable = pRecords != nullptr && Vars::Backtrack::Enabled.Value;

		Vec3 vOriginalPos = pEnemy->GetAbsOrigin();
		Vec3 vOriginalEyeAngles = pEnemy->GetEyeAngles();
		Vec3 vShootPos = pLocal->GetShootPos();

		if (bBacktrackable)
		{

			for (const auto& pTick : *pRecords)
			{

				pEnemy->SetAbsOrigin(pTick.AbsOrigin);	//	set these 4 CalcNearestPoint
				pEnemy->SetEyeAngles(pTick.EyeAngles);

				/* Default stab */
				if (!TraceMelee(pLocal, pWeapon, Math::CalcAngle(pLocal->GetShootPos(), pTick.WorldSpaceCenter), &pTraceEnemy) || pTraceEnemy != pEnemy)
				{
					continue;
				}

				if (!CanBackstab(vAngleTo, pTick.EyeAngles, (pTick.WorldSpaceCenter - pLocal->GetWorldSpaceCenter())))
				{
					continue;
				}

				// Silent backstab
				if (Vars::Triggerbot::Stab::Silent.Value)
				{
					Utils::FixMovement(pCmd, vAngleTo);
					G::SilentTime = true;
				}

				pCmd->viewangles = Math::CalcAngle(pLocal->GetShootPos(), pTick.WorldSpaceCenter);
				pCmd->buttons |= IN_ATTACK;
				m_bShouldDisguise = true;

				pCmd->tick_count = TIME_TO_TICKS(pTick.SimulationTime + TICKS_TO_TIME(TIME_TO_TICKS(G::LerpTime)));

				pEnemy->SetAbsOrigin(vOriginalPos);
				pEnemy->SetEyeAngles(vOriginalEyeAngles);
				return;
			}

			pEnemy->SetAbsOrigin(vOriginalPos);
			pEnemy->SetEyeAngles(vOriginalEyeAngles);
		}
		else
		{
			/* Default stab */
			if (!TraceMelee(pLocal, pWeapon, vAngleTo, &pTraceEnemy) || pTraceEnemy != pEnemy)
			{
				continue;
			}

			if (!CanBackstab(vAngleTo, pEnemy->GetEyeAngles(),
				(pEnemy->GetWorldSpaceCenter() - pLocal->GetWorldSpaceCenter())))
			{
				continue;
			}

			if (Vars::Triggerbot::Stab::Silent.Value)
			{
				Utils::FixMovement(pCmd, vAngleTo);
				G::SilentTime = true;
			}

			pCmd->viewangles = vAngleTo;
			pCmd->buttons |= IN_ATTACK;
			m_bShouldDisguise = true;


			Vec3 vForward = {};
			Math::AngleVectors(pCmd->viewangles, &vForward);
			const Vec3 vTraceStart = pLocal->GetShootPos();
			const Vec3 vTraceEnd = (vTraceStart + (vForward * 8192.0f));

			CGameTrace trace = {};
			CTraceFilterHitscan filter = {};
			filter.pSkip = pLocal;



			Utils::Trace(vTraceStart, vTraceEnd, MASK_SHOT, &filter, &trace);
			const int iAttachment = pWeapon->LookupAttachment("muzzle");
			pWeapon->GetAttachment(iAttachment, trace.vStartPos);


			if (pCmd->buttons & IN_ATTACK)
			{
				I::DebugOverlay->ClearAllOverlays();
				const model_t* model = pEnemy->GetModel();
				const studiohdr_t* hdr = I::ModelInfoClient->GetStudioModel(model);
				const mstudiohitboxset_t* set = hdr->GetHitboxSet(pEnemy->GetHitboxSet());
				const mstudiobbox_t* bbox = set->hitbox(HITBOX_PELVIS);

				matrix3x4 rotMatrix;
				Math::AngleMatrix(bbox->angle, rotMatrix);

				matrix3x4 matrix;
				matrix3x4 boneees[128];
				pEnemy->SetupBones(boneees, 128, BONE_USED_BY_ANYTHING, 0);
				Math::ConcatTransforms(boneees[bbox->bone], rotMatrix, matrix);

				Vec3 bboxAngle;
				Math::MatrixAngles(matrix, bboxAngle);

				Vec3 matrixOrigin;
				Math::GetMatrixOrigin(matrix, matrixOrigin);

				I::DebugOverlay->AddBoxOverlay2(matrixOrigin, bbox->bbmin, bbox->bbmax, bboxAngle, { 0,0,0,0 }, { 255,0,0,255 }, 4);
				F::Visuals.DrawHitboxMatrix(pEnemy, { 0,0,0,0 }, { 255, 255, 255, 255 }, 4);
				I::DebugOverlay->AddLineOverlay(trace.vStartPos, pEnemy->GetHitboxPos(HITBOX_PELVIS), 255, 255, 255, true, 4);
			}

			if (Vars::Misc::DisableInterpolation.Value)
			{
				pCmd->tick_count = TIME_TO_TICKS(pEnemy->GetSimulationTime() + G::LerpTime);
			}

			return;
		}
	}
}

void CAutoStab::Run(CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon, CUserCmd* pCmd)
{
	if (!Vars::Triggerbot::Stab::Active.Value || !G::WeaponCanAttack || pWeapon->GetWeaponID() != TF_WEAPON_KNIFE)
	{
		return;
	}

	if (Vars::Triggerbot::Stab::RageMode.Value)
	{
		RunRage(pLocal, pWeapon, pCmd);
	}
	else
	{
		RunLegit(pLocal, pWeapon, pCmd);
	}

	if (pCmd->buttons & IN_ATTACK)
	{
		G::IsAttacking = true;
	}

	G::AutoBackstabRunning = true;
}