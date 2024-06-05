#include "AimbotMelee.h"
#include "../../Vars.h"

#include "../../Backtrack/Backtrack.h"

int  CAimbotMelee::GetRange(CBaseCombatWeapon* pWeapon, CBaseEntity* pLocal)
{

	if (pWeapon->GetWeaponID() == TF_WEAPON_SWORD)
	{
		return 72;
	}

	return 48;
}

bool CAimbotMelee::CanMeleeHit(CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon, const Vec3& vecViewAngles, int nTargetIndex)
{
	// Setup a volume for the melee weapon to be swung - approx size, so all melee behave the same.
	static Vector vecSwingMinsBase(-18, -18, -18);
	static Vector vecSwingMaxsBase(18, 18, 18);

	float fBoundsScale = 1.0f;
	Utils::ATTRIB_HOOK_FLOAT(fBoundsScale, "melee_bounds_multiplier", pWeapon, 0, 1);
	Vector vecSwingMins = vecSwingMinsBase * fBoundsScale;
	Vector vecSwingMaxs = vecSwingMaxsBase * fBoundsScale;

	float fSwingRange = GetRange(pWeapon, pLocal);

	// Scale the range and bounds by the model scale if they're larger
	// Not scaling down the range for smaller models because midgets need all the help they can get
	if (pLocal->m_flModelScale() > 1.0f)
	{
		fSwingRange *= pLocal->m_flModelScale();
		vecSwingMins *= pLocal->m_flModelScale();
		vecSwingMaxs *= pLocal->m_flModelScale();
	}

	Utils::ATTRIB_HOOK_FLOAT(fSwingRange, "melee_range_multiplier", pWeapon, 0, 1);

	Vector vecForward;
	Math::AngleVectors(vecViewAngles, &vecForward);
	Vector vecSwingStart = pLocal->GetShootPos();
	Vector vecSwingEnd = vecSwingStart + vecForward * fSwingRange;
	Ray_t ray;

	ray.Init(vecSwingStart, vecSwingEnd, vecSwingMins, vecSwingMaxs);
	CTraceFilterHitscan filter;
	filter.pSkip = pLocal;
	CGameTrace trace;
	Utils::TraceHull(vecSwingStart, vecSwingEnd, vecSwingMins, vecSwingMaxs, MASK_SHOT_HULL, &filter, &trace);

	if (!(trace.entity && trace.entity->GetIndex() == nTargetIndex))
	{
		const INetChannel* pNetChannel = I::EngineClient->GetNetChannelInfo();

		if (!pNetChannel)
			return false;

		static constexpr float FL_DELAY = 0.35f; //it just works

		float fInterp = G::LerpTime;
		float fLatency = (pNetChannel->GetLatency(FLOW_OUTGOING) + pNetChannel->GetLatency(FLOW_INCOMING));

		float MAX_TIME = FL_DELAY;
		float TIME_STEP = (MAX_TIME / 128.0f);

		for (float fPredTime = 0.0f; fPredTime < 8192; fPredTime += TIME_STEP)
		{
			float fCorrectTime = (fPredTime + fInterp + fLatency);

			if (!Vars::Aimbot::Melee::PredictSwing.Value || pWeapon->GetWeaponID() == TF_WEAPON_KNIFE || pLocal->IsCharging())
			{
				return false;
			}

			if (pLocal->IsOnGround())
				vecSwingStart += ((pLocal->GetVelocity() * fCorrectTime));

			else vecSwingStart += (pLocal->GetVelocity() * fCorrectTime) - (Vec3(0.0f, 0.0f, g_ConVars.sv_gravity->GetFloat()) * 0.5f * fCorrectTime * fCorrectTime);

			Vec3 vecTraceEnd = vecSwingStart + (vecForward * fSwingRange);
			Utils::TraceHull(vecSwingStart, vecTraceEnd, vecSwingMins, vecSwingMaxs, MASK_SHOT_HULL, &filter, &trace);

			return (trace.entity && trace.entity->GetIndex() == nTargetIndex);
		}
	}
	return true;

}

ESortMethod CAimbotMelee::GetSortMethod()
{
	switch (Vars::Aimbot::Melee::SortMethod.Value)
	{
	case 0: return ESortMethod::FOV;
	case 1: return ESortMethod::DISTANCE;
	default: return ESortMethod::UNKNOWN;
	}
}

bool CAimbotMelee::GetTargets(CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon)
{
	const ESortMethod sortMethod = GetSortMethod();

	F::AimbotGlobal.m_vecTargets.clear();

	const Vec3 vLocalPos = pLocal->GetShootPos();
	const Vec3 vLocalAngles = I::EngineClient->GetViewAngles();

	// Players
	if (Vars::Aimbot::Global::AimPlayers.Value)
	{
		const bool bWhipTeam = (pWeapon->GetItemDefIndex() == Soldier_t_TheDisciplinaryAction &&
			Vars::Aimbot::Melee::WhipTeam.Value);

		for (const auto& pTarget : g_EntityCache.GetGroup(
			bWhipTeam ? EGroupType::PLAYERS_ALL : EGroupType::PLAYERS_ENEMIES))
		{
			if (!pTarget->IsAlive() || pTarget->IsAGhost())
			{
				continue;
			}

			if (pTarget == pLocal)
			{
				continue;
			}

			if (F::AimbotGlobal.ShouldIgnore(pTarget, true)) { continue; }


			Vec3 vPos = pTarget->GetHitboxPos(HITBOX_SPINE_0);
			Vec3 vAngleTo = Math::CalcAngle(vLocalPos, vPos);
			const float flFOVTo = Math::CalcFov(vLocalAngles, vAngleTo);

			if ((sortMethod == ESortMethod::FOV || Vars::Aimbot::Melee::RespectFOV.Value) && flFOVTo > Vars::Aimbot::Global::AimFOV.Value)
			{
				continue;
			}

			const float flDistTo = sortMethod == ESortMethod::DISTANCE ? vLocalPos.DistTo(vPos) : 0.0f;

			const uint32_t priorityID = g_EntityCache.GetPR()->GetValid(pTarget->GetIndex()) ? g_EntityCache.GetPR()->GetAccountID(pTarget->GetIndex()) : 0;
			const auto& priority = G::PlayerPriority[priorityID];

			F::AimbotGlobal.m_vecTargets.push_back({ pTarget, ETargetType::PLAYER, vPos, vAngleTo, flFOVTo, flDistTo, -1, false, priority });
		}
	}

	return !F::AimbotGlobal.m_vecTargets.empty();
}

bool CAimbotMelee::VerifyTarget(CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon, Target_t& target)
{

	if (Vars::Aimbot::Melee::RangeCheck.Value)
	{
		if (!CanMeleeHit(pLocal, pWeapon, target.m_vAngleTo, target.m_pEntity->GetIndex()))
		{
			return false;
		}
	}
	else {
		if (!Utils::VisPos(pLocal, target.m_pEntity, pLocal->GetShootPos(), target.m_vPos))
		{
			return false;
		}
	}

	return true;
}

bool CAimbotMelee::GetTarget(CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon, Target_t& Out)
{
	if (!GetTargets(pLocal, pWeapon))
	{
		return false;
	}

	F::AimbotGlobal.SortTargets(GetSortMethod());

	for (auto& target : F::AimbotGlobal.m_vecTargets)
	{
		if (!VerifyTarget(pLocal, pWeapon, target))
		{
			continue;
		}

		Out = target;
		return true;
	}

	return false;
}

void CAimbotMelee::Aim(CUserCmd* pCmd, Vec3& vAngle)
{
	vAngle -= G::PunchAngles;
	switch (Vars::Aimbot::Melee::AimMethod.Value)
	{
	case 0:
	{
		pCmd->viewangles = vAngle;
		I::EngineClient->SetViewAngles(pCmd->viewangles);
		break;
	}
	case 1:
	{
		if (!Vars::Aimbot::Melee::SmoothingAmount.Value) { break; }
		Vec3 vDelta = vAngle - pCmd->viewangles;
		Math::ClampAngles(vDelta);
		pCmd->viewangles += vDelta / Vars::Aimbot::Melee::SmoothingAmount.Value;
		I::EngineClient->SetViewAngles(pCmd->viewangles);
		return;
	}
	case 2:
	{
		if (!G::IsAttacking) { return; }
		G::SilentTime = true;
		Utils::FixMovement(pCmd, vAngle);
		pCmd->viewangles = vAngle;
		return;
	}
	}

	pCmd->viewangles = vAngle;
	I::EngineClient->SetViewAngles(pCmd->viewangles);
	return;
}

bool CAimbotMelee::ShouldSwing(CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon, CUserCmd* pCmd, const Target_t& Target)
{
	if (Vars::Aimbot::Global::Active.Value)
	{
		return true;
	}
}

bool CAimbotMelee::IsAttacking(CUserCmd* pCmd, CBaseCombatWeapon* pWeapon)
{
	return fabsf(pWeapon->GetSmackTime() - I::GlobalVars->curtime) < pWeapon->GetWeaponData().m_flSmackDelay;
}

void CAimbotMelee::Run(CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon, CUserCmd* pCmd)
{
	if (!Vars::Aimbot::Global::Active.Value || G::AutoBackstabRunning || pWeapon->GetWeaponID() == TF_WEAPON_KNIFE)
	{
		return;
	}

	Target_t target = {};

	const bool bShouldAim = F::AimbotGlobal.IsKeyDown();

	if (GetTarget(pLocal, pWeapon, target) && bShouldAim)
	{
		G::CurrentTargetIdx = target.m_pEntity->GetIndex();

		if (Vars::Aimbot::Melee::AimMethod.Value == 2)
		{
			G::AimPos = target.m_vPos;
		}

		if (CanMeleeHit(pLocal, pWeapon, target.m_vAngleTo, target.m_pEntity->GetIndex()) && Vars::Aimbot::Global::Active.Value)
		{
			pCmd->buttons |= IN_ATTACK;
		}

		G::IsAttacking = G::IsAttacking || IsAttacking(pCmd, pWeapon);

		Aim(pCmd, target.m_vAngleTo);

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

		if (G::IsAttacking)
		{
			I::DebugOverlay->ClearAllOverlays();
			const model_t* model = target.m_pEntity->GetModel();
			const studiohdr_t* hdr = I::ModelInfoClient->GetStudioModel(model);
			const mstudiohitboxset_t* set = hdr->GetHitboxSet(target.m_pEntity->GetHitboxSet());
			const mstudiobbox_t* bbox = set->hitbox(HITBOX_SPINE_0);

			matrix3x4 rotMatrix;
			Math::AngleMatrix(bbox->angle, rotMatrix);

			matrix3x4 matrix;
			matrix3x4 boneees[128];
			target.m_pEntity->SetupBones(boneees, 128, BONE_USED_BY_ANYTHING, 0);
			Math::ConcatTransforms(boneees[bbox->bone], rotMatrix, matrix);

			Vec3 bboxAngle;
			Math::MatrixAngles(matrix, bboxAngle);

			Vec3 matrixOrigin;
			Math::GetMatrixOrigin(matrix, matrixOrigin);

			I::DebugOverlay->AddBoxOverlay2(matrixOrigin, bbox->bbmin, bbox->bbmax, bboxAngle, { 0,0,0,0 }, { 255,0,0,255 }, 4);
			F::Visuals.DrawHitboxMatrix(target.m_pEntity, { 0,0,0,0 }, { 255, 255, 255, 255 }, 4);
			I::DebugOverlay->AddLineOverlay(trace.vStartPos, target.m_pEntity->GetHitboxPos(HITBOX_SPINE_0), 255, 255, 255, true, 4);
		}

		if (G::IsAttacking && target.m_TargetType == ETargetType::PLAYER)
		{
			const float simTime = target.ShouldBacktrack ? target.SimTime : target.m_pEntity->GetSimulationTime();
			pCmd->tick_count = TIME_TO_TICKS(simTime + G::LerpTime);
		}
	}
}