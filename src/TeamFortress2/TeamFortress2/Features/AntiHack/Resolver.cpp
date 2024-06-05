#include "Resolver.h"

static std::vector YawResolves{ 0.0f, 180.0f, 65.0f, -65.0f, -180.0f };

bool CResolver::ShouldAutoResolve()
{
	if (G::CurWeaponType == EWeaponType::PROJECTILE) { return false; }

	if (const auto& pWeapon = g_EntityCache.GetWeapon())
	{
		if (pWeapon->GetClassID() == ETFClassID::CTFMinigun) { return false; }
	}

	return true;
}

/* Run the resolver and apply the resolved angles */
void CResolver::Run()
{
	if (!Vars::AntiHack::Resolver::Resolver.Value) { return; }

	Vec3 localHead;
	if (const auto& pLocal = g_EntityCache.GetLocal())
	{
		localHead = pLocal->GetEyePosition();
	}

	UpdateSniperDots();

	for (auto i = 1; i <= I::EngineClient->GetMaxClients(); i++)
	{
		CBaseEntity* entity;
		PlayerInfo_t temp{};

		if (!(entity = I::ClientEntityList->GetClientEntity(i))) {
			continue;
		}

		if (entity->GetDormant()) {
			continue;
		}

		if (!I::EngineClient->GetPlayerInfo(i, &temp)) {
			continue;
		}

		if (!entity->GetLifeState() == LIFE_ALIVE) {
			continue;
		}

		if (entity->IsTaunting()) {
			continue;
		}

		const Vector vX = entity->GetEyeAngles();
		auto* m_angEyeAnglesX = reinterpret_cast<float*>(reinterpret_cast<DWORD>(entity) + g_NetVars.
			get_offset("DT_TFPlayer", "tfnonlocaldata", "m_angEyeAngles[0]"));
		auto* m_angEyeAnglesY = reinterpret_cast<float*>(reinterpret_cast<DWORD>(entity) + g_NetVars.
			get_offset("DT_TFPlayer", "tfnonlocaldata", "m_angEyeAngles[1]"));

		auto findResolve = F::Resolver.ResolvePlayers.find(temp.friendsID);
		ResolveMode resolveMode;
		if (findResolve != F::Resolver.ResolvePlayers.end())
		{
			resolveMode = findResolve->second;
		}

		if (const float SniperDotYaw = ResolveSniperDot(entity))
		{
			*m_angEyeAnglesX = SniperDotYaw;
			break;
		}

		if ((entity->GetClassNum() == CLASS_SNIPER || entity->GetClassNum() == CLASS_HEAVY) && (entity->GetEyeAngles().x >= 90.f || entity->GetEyeAngles().x <= 90.f))
		{
			*m_angEyeAnglesX = -89;
		}

		if ((entity->GetClassNum() == CLASS_SCOUT || entity->GetClassNum() == CLASS_SPY || entity->GetClassNum() == CLASS_DEMOMAN || entity->GetClassNum() == CLASS_MEDIC || entity->GetClassNum() == CLASS_PYRO || entity->GetClassNum() == CLASS_ENGINEER || entity->GetClassNum() == CLASS_SOLDIER) && (entity->GetEyeAngles().x >= 90.f || entity->GetEyeAngles().x <= 90.f))
		{
			*m_angEyeAnglesX = 89;
		}

		*m_angEyeAnglesY = YawResolves[ResolveData[temp.friendsID].Mode];
	}
}

/* Update resolver data (for Bruteforce) */
void CResolver::Update(CUserCmd* pCmd)
{
	if (!Vars::AntiHack::Resolver::Resolver.Value) { return; }

	// Log shots
	if (const auto& pLocal = g_EntityCache.GetLocal())
	{
		if (G::CurrentTargetIdx != 0 &&
			(pCmd->buttons & IN_ATTACK || G::IsAttacking) &&
			pLocal->GetActiveWeapon()->CanShoot(pCmd, pLocal))
		{
			PlayerInfo_t temp{};
			const int aimTarget = G::CurrentTargetIdx;

			if (const auto& pTarget = I::ClientEntityList->GetClientEntity(aimTarget))
			{
				if (I::EngineClient->GetPlayerInfo(aimTarget, &temp))
				{
					const auto findResolve = F::Resolver.ResolvePlayers.find(temp.friendsID);
					ResolveMode resolveMode;
					if (findResolve != F::Resolver.ResolvePlayers.end())
					{
						resolveMode = findResolve->second;
					}

					if (resolveMode.m_Yaw == 6)
					{
						ResolveData[temp.friendsID].LastShot = I::EngineClient->Time();
						ResolveData[temp.friendsID].RequiresUpdate = true;
					}
				}
			}
		}
	}

	// Check for misses
	for (auto& data : ResolveData)
	{
		float delay = 1.f;
		if (const auto nc = I::EngineClient->GetNetChannelInfo())
		{
			delay = (nc->GetLatency(FLOW_OUTGOING) + nc->GetLatency(FLOW_INCOMING)) + 0.3f;
		}
		const float time = I::EngineClient->Time();
		const bool shouldCheck = (time - data.second.LastShot) > delay;
		const float timeDiff = data.second.LastHit - data.second.LastShot;

		if (data.second.RequiresUpdate && shouldCheck && timeDiff <= 0.f)
		{
			// Miss
			data.second.Mode += 1;
			while (data.second.Mode >= YawResolves.size())
			{
				data.second.Mode -= YawResolves.size();
			}

			data.second.RequiresUpdate = false;
		}
	}
}

void CResolver::UpdateSniperDots() {
	SniperDotMap.clear();

	// Find sniper dots
	for (int i = I::EngineClient->GetMaxClients() + 1; i <= I::ClientEntityList->GetHighestEntityIndex(); i++)
	{
		if (CBaseEntity* eTarget = I::ClientEntityList->GetClientEntity(i)) {
			if (eTarget->GetClassID() != ETFClassID::CSniperDot)
				continue;

			if (CBaseEntity* pOwner = I::ClientEntityList->GetClientEntityFromHandle(eTarget->m_hOwnerEntity())) {
				SniperDotMap[pOwner] = eTarget;
			}
		}
	}
}

float CResolver::ResolveSniperDot(CBaseEntity* pOwner) {
	if (CBaseEntity* SniperDot = SniperDotMap[pOwner]) {
		const Vec3 DotOrigin = SniperDot->m_vecOrigin();
		const Vec3 EyePosition = pOwner->GetEyePosition();
		const Vec3 delta = DotOrigin - EyePosition;
		Vec3 angles;
		Math::VectorAngles(delta, angles);
		return angles.x;
	}
	return false;
}

/* Called when the someone was damaged. Did we hit? */
void CResolver::OnPlayerHurt(CGameEvent* pEvent)
{
	if (!Vars::AntiHack::Resolver::Resolver.Value) { return; }

	const int victim = I::EngineClient->GetPlayerForUserID(pEvent->GetInt("userid"));
	const int attacker = I::EngineClient->GetPlayerForUserID(pEvent->GetInt("attacker"));
	const bool bCrit = pEvent->GetBool("crit");

	if (attacker == I::EngineClient->GetLocalPlayer()) {
		PlayerInfo_t temp{};

		if (!I::EngineClient->GetPlayerInfo(victim, &temp)) { return; }
		if (ResolveData.find(temp.friendsID) == ResolveData.end()) { return; }
		if (Vars::Aimbot::Hitscan::AimHitbox.Value == 0 && !bCrit) { return; }

		ResolveData[temp.friendsID].LastHit = I::EngineClient->Time();
		// ResolveData[temp.friendsID].RequiresUpdate = false;
	}
}