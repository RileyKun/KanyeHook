#pragma once
#include "../../SDK/SDK.h"

struct ResolveMode {
	int m_Pitch = 4; // Default to AUTO
	int m_Yaw = 6;
};

struct ResolverData {
	float LastShot = 0.f;
	float LastHit = 0.f;
	int Mode = 0;
	bool RequiresUpdate = false;
};

struct BruteData {
	int BruteNum = 0;
	int HitStreak = 0;
};

class CResolver
{
private:

public:
	void UpdateSniperDots();
	float ResolveSniperDot(CBaseEntity* pOwner);
	std::unordered_map<CBaseEntity*, CBaseEntity*> SniperDotMap;
	bool ShouldAutoResolve();
	void Run();

	void Update(CUserCmd* pCmd);
	void OnPlayerHurt(CGameEvent* pEvent);

	std::unordered_map<int, ResolveMode> ResolvePlayers;
	std::unordered_map<int, ResolverData> ResolveData;
};

ADD_FEATURE(CResolver, Resolver)