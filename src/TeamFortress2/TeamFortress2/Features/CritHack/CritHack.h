#pragma once
#include "../../SDK/SDK.h"

class CCritHack
{
private:
	bool AreRandomCritsEnabled();
	bool IsEnabled();
	bool ShouldCrit();
	bool NoRandomCrits(CBaseCombatWeapon* pWeapon);
	std::pair<float, float> critMultInfo(CBaseCombatWeapon* wep);
	float getWithdrawMult(CBaseCombatWeapon* pWeapon);
	float getWithdrawAmount(CBaseCombatWeapon* pWeapon);
	bool IsAttacking(const CUserCmd* pCmd, CBaseCombatWeapon* pWeapon);
	void ScanForCrits(const CUserCmd* pCmd, int loops = 10);
	int LastGoodCritTick(const CUserCmd* pCmd);
	int damageUntilToCrit(CBaseCombatWeapon* wep);
	std::vector<int> CritTicks{};
	int LastGoodSkipTick(const CUserCmd* pCmd);
	float AddedPerShot = 0.f;
	float critmultx3 = 0.f;
	UINT32 decrypt_or_encrypt_seed(CBaseEntity* local, CBaseCombatWeapon* weapon, UINT32 seed);
	float TakenPerCrit = 0.0f;
	int cached_damage = 0;
	int crit_damage = 0;
	int melee_damage = 0;
	int round_damage = 0;
	bool is_out_of_sync = false;
	int ShotsToFill = 0;
	float boost_damage = 0.f;
	float LastBucket = -1.f;
	int LastCritTick = -1;
	std::vector<int> skip{};
	bool is_pure_crit_command(const INT32 command_number);
	int LastWeapon = 0;
	bool calling_crithelper = false;
	int shots_until_crit = 0;

	int crit_damage_till_unban = 0;
	int32_t wish_random_seed = 0;

	struct stats_t
	{
		float flCritBucket;	// 0xA54
		int iNumAttacks;	// 0xA58
		int iNumCrits;		// 0xA5C
	};

public:
	void Run(CUserCmd* pCmd);
	void Draw();
	bool CritHandler();
	void Events(CGameEvent* event, const FNV1A_t uNameHash);
	float CritBucketBP = 0;
	int IndicatorW;
	int IndicatorH;
	bool ProtectData = false;
};


ADD_FEATURE(CCritHack, CritHack)