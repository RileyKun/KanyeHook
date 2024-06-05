#include "AutoDetonate.h"

#include "../../Vars.h"

//credits to KGB
class CEntitySphereQuery {
public:
	CEntitySphereQuery(const Vec3& center, const float radius, const int flagMask = 0,
		const int partitionMask = PARTITION_CLIENT_NON_STATIC_EDICTS)
	{
		static DWORD dwAddress = g_Pattern.Find(_(L"client.dll"), _(L"55 8B EC 83 EC 14 D9 45 0C"));
		reinterpret_cast<void(__thiscall*)(void*, const Vec3&, float, int, int)>(dwAddress)(
			this, center, radius, flagMask, partitionMask);
	}

	CBaseEntity* GetCurrentEntity()
	{
		return (m_nListIndex < m_nListCount) ? m_pList[m_nListIndex] : nullptr;
	}

	void NextEntity()
	{
		m_nListIndex++;
	}

private:
	int m_nListIndex, m_nListCount;
	CBaseEntity* m_pList[MAX_SPHERE_QUERY];
};

bool CAutoDetonate::CheckDetonation(CBaseEntity* pLocal, const std::vector<CBaseEntity*>& entityGroup, float radius)
{
	for (const auto& pExplosive : entityGroup)
	{
		CBaseEntity* pTarget;

		// Iterate through entities in sphere radius
		for (CEntitySphereQuery sphere(pExplosive->GetAbsOrigin(), pExplosive->GetTouched() ? 150 : 100);
			(pTarget = sphere.GetCurrentEntity()) != nullptr;
			sphere.NextEntity())
		{
			if (!pTarget || pTarget == pLocal || !pTarget->IsAlive() || pTarget->GetTeamNum() == pLocal->
				GetTeamNum())
			{
				continue;
			}

			const bool bIsPlayer = pTarget->IsPlayer() && Vars::Triggerbot::Detonate::DetonateOnPlayer.Value;
			const bool bIsBuilding = pTarget->IsBuilding() && Vars::Triggerbot::Detonate::DetonateOnBuilding.Value;
			bool CanDestroySticky = (G::CurItemDefIndex == Demoman_s_TheQuickiebombLauncher || G::CurItemDefIndex == Demoman_s_TheScottishResistance && Vars::Triggerbot::Detonate::DetonateOnSticky.Value);
			if (bIsPlayer || bIsBuilding || pTarget->GetPipebombType() == TYPE_STICKY && CanDestroySticky)
			{
				if (bIsPlayer && F::AutoGlobal.ShouldIgnore(pTarget)) { continue; }

				CGameTrace trace = {};
				CTraceFilterWorldAndPropsOnly traceFilter = {};
				Utils::Trace(pExplosive->GetAbsOrigin(), pTarget->GetAbsOrigin(), MASK_SOLID_BRUSHONLY, &traceFilter,
					&trace);

				if (trace.flFraction >= 0.99f || trace.entity == pTarget)
				{
					return true;
				}
			}
		}
	}

	return false;
}

void CAutoDetonate::Run(CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon, CUserCmd* pCmd)
{
	if (!Vars::Triggerbot::Detonate::Active.Value) { return; }

	bool shouldDetonate = false;

	// Check sticky detonation
	if (Vars::Triggerbot::Detonate::Stickies.Value
		&& CheckDetonation(pLocal, g_EntityCache.GetGroup(EGroupType::LOCAL_STICKIES), 110.0f))
	{
		shouldDetonate = true;
	}

	// Check flare detonation
	if (Vars::Triggerbot::Detonate::Flares.Value
		&& CheckDetonation(pLocal, g_EntityCache.GetGroup(EGroupType::LOCAL_FLARES), 85.0f * Vars::Triggerbot::Detonate::RadiusScale.Value))
	{
		shouldDetonate = true;
	}

	if (shouldDetonate)
	{
		pCmd->buttons |= IN_ATTACK2;
	}
}
