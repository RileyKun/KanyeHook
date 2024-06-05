#pragma once

#include "../BaseEntity/BaseEntity.h"


#ifndef TICKS_TO_TIME
#define TICKS_TO_TIME( t )	( I::GlobalVars->interval_per_tick * ( t ) )
#endif

#define OFFSET( name, type, offset ) inline type& name( ) { \
	return *reinterpret_cast< type* >( reinterpret_cast< uint32_t >( this ) + offset ); \
}

inline auto crit()
{
	static auto ret = g_Pattern.Find(_(L"client.dll"), _(L"53 57 6A ? 68 ? ? ? ? 68 ? ? ? ? 6A ? 8B F9 E8 ? ? ? ? 50 E8 ? ? ? ? 8B D8 83 C4 ? 85 DB 0F 84"));
	return ret;
}

inline DWORD calcisattackcriticaloffset = 0;

class CBaseCombatWeapon : public CBaseEntity
{
public: //Netvars
	M_DYNVARGET(Clip1, int, this, _("DT_BaseCombatWeapon"), _("LocalWeaponData"), _("m_iClip1"))
		M_DYNVARGET(Clip2, int, this, _("DT_BaseCombatWeapon"), _("LocalWeaponData"), _("m_iClip2"))
		M_DYNVARGET(nViewModelIndex, int, this, _("DT_BaseCombatWeapon"), _("LocalWeaponData"), _("m_nViewModelIndex"))
		M_DYNVARGET(iViewModelIndex, int, this, _("DT_BaseCombatWeapon"), _("m_iViewModelIndex"))
		M_DYNVARGET(ItemDefIndex, int, this, _("DT_EconEntity"), _("m_AttributeManager"), _("m_Item"), _("m_iItemDefinitionIndex"))
		M_DYNVARGET(ChargeBeginTime, float, this, _("DT_WeaponPipebombLauncher"), _("PipebombLauncherLocalData"), _("m_flChargeBeginTime"))
		M_DYNVARGET(ChargeDamage, float, this, _("DT_TFSniperRifle"), _("SniperRifleLocalData"), _("m_flChargedDamage"))
		M_DYNVARGET(LastFireTime, float, this, _("DT_TFWeaponBase"), _("LocalActiveTFWeaponData"), _("m_flLastFireTime"))
		M_DYNVARGET(NextSecondaryAttack, float, this, _("DT_BaseCombatWeapon"), _("LocalActiveWeaponData"), _("m_flNextSecondaryAttack"))
		M_DYNVARGET(NextPrimaryAttack, float, this, _("DT_BaseCombatWeapon"), _("LocalActiveWeaponData"), _("m_flNextPrimaryAttack"))
		M_DYNVARGET(ChargeResistType, int, this, _("DT_WeaponMedigun"), _("m_nChargeResistType"))
		M_DYNVARGET(ReloadMode, int, this, _("DT_TFWeaponBase"), _("m_iReloadMode"))
		M_DYNVARGET(DetonateTime, float, this, _("DT_WeaponGrenadeLauncher"), _("m_flDetonateTime"))
		NETVAR(ObservedCritChance, float, "CTFWeaponBase", "m_flObservedCritChance");
	    M_DYNVARGET(LastCritCheckTime, float, this, "DT_TFWeaponBase", "LocalActiveTFWeaponData", "m_flLastCritCheckTime")
	
		M_DYNVARGET(InvisCompleteTime, float, this, "DT_TFPlayer", "m_Shared", "m_flInvisChangeCompleteTime")
		M_OFFSETGET(UberCharge, float, 0xC6C) //DT_WeaponMedigun -> NonLocalTFWeaponMedigundata -> m_flChargeLevel
		//M_OFFSETGET(HealingTarget, int, 0xC48) //DT_WeaponMedigun -> m_hHealingTarget
		M_OFFSETGET(Healing, int, 0xC51) //DT_WeaponMedigun -> m_bHealing
		NETVAR(m_iPrimaryAmmoType, int, "CBaseCombatWeapon", "m_iPrimaryAmmoType");
	    NETVAR(m_iWeaponState, int, "CTFMinigun", "m_iWeaponState");
	    NETVAR(m_bCritShot, bool, "CTFMinigun", "m_bCritShot");
		NETVAR(m_iSeed, int, "CTEFireBullets", "m_iSeed");
		NETVAR(m_flSpread, float, "CTEFireBullets", "m_flSpread");

public: //Virtuals
	M_VIRTUALGET(WeaponID, int, this, int(__thiscall*)(void*), 381)
		M_VIRTUALGET(Slot, int, this, int(__thiscall*)(void*), 330)
		M_VIRTUALGET(DamageType, int, this, int(__thiscall*)(void*), 340)
		M_VIRTUALGET(FinishReload, void, this, void(__thiscall*)(void*), 275)
		M_VIRTUALGET(BulletSpread, Vec3&, this, Vec3& (__thiscall*)(void*), 286)


public: //Everything else, lol
	__inline float GetSmackTime() {
		//credits to KGB
		static auto dwOffset = g_NetVars.get_offset("DT_TFWeaponBase", "m_nInspectStage") + 0x1C;
		return *reinterpret_cast<float*>(this + dwOffset);
	}

	OFFSET(m_last_crit_check_time, float, 0xb54);
	OFFSET(m_last_rapid_fire_crit_check_time, float, 0xb60);
	M_OFFSETPURE(CritTokenBucket, float, 0xA54)
	M_OFFSETPURE(CritChecks, int, 0xA58)
	M_OFFSETPURE(CritSeedRequests, int, 0xA5C)
	M_OFFSETPURE(WeaponMode, int, 0xB24)
	M_OFFSETPURE(CritTime, float, 0xB50)
	M_OFFSETPURE(LastCritCheckFrame, int, 0xB58)
	M_OFFSETPURE(LastRapidfireCritCheckTime, float, 0xB60)
	M_OFFSETPURE(CritShot, bool, 0xB36)
	M_OFFSETPURE(RandomSeed, int, 0xB5C)

	bool IsRapidFireCrits()
	{
		switch (GetItemDefIndex())
		{
		case Scout_s_PrettyBoysPocketPistol:
			return true;
		}

		switch (GetWeaponID())
		{
		case TF_WEAPON_MINIGUN:
		case TF_WEAPON_SYRINGEGUN_MEDIC:
		case TF_WEAPON_FLAMETHROWER:
		case TF_WEAPON_PISTOL:
		case TF_WEAPON_PISTOL_SCOUT:
		case TF_WEAPON_SMG:
		case TF_WEAPON_FLAREGUN:
			return true;
		}

		switch (GetItemDefIndex())
		{
		case Scout_m_TheShortstop:
		case Scout_s_ScoutsPistol:
		case Scout_s_PistolR:
		case Scout_s_PrettyBoysPocketPistol:
			return true;
		}

		return false;
	}

	inline int& m_iWeaponMode() {
		static int offset = 716;
		return *reinterpret_cast<int*>(reinterpret_cast<DWORD>(this) + offset);
	}


	__inline bool AmbassadorCanHeadshot()
	{
		if (GetItemDefIndex() == Spy_m_TheAmbassador || GetItemDefIndex() == Spy_m_FestiveAmbassador)
		{
			if ((I::GlobalVars->curtime - GetLastFireTime()) <= 1.0)
			{
				return false;
			}
		}
		return true;
	}


	inline void SetObservedCritChance(float crit_chance)
	{
		static auto offset = g_NetVars.get_offset("CTFWeaponBase", "m_flObservedCritChance");
		*reinterpret_cast<float*>(reinterpret_cast<DWORD>(this) + offset) = crit_chance;
	}

	inline void SetWeaponMode(int mode)
	{
		*reinterpret_cast<int*>(reinterpret_cast<DWORD>(this) + 716) = mode;

	}


	inline const char* GetName()
	{
		return GetVFunc<const char* (__thiscall*)(void*)>(this, 331)(this);
	}

	__inline float LastCritCheckTime()
	{
		DYNVAR_RETURN(float, this, "DT_TFWeaponBase", "LocalActiveTFWeaponData", "m_flLastCritCheckTime");
	}

	__inline float GetInvisPercentage()
	{
		const float invisTime = I::Cvar->FindVar("tf_spy_invis_time")->GetFloat();
		const float GetInvisPercent = Math::RemapValClamped(GetInvisCompleteTime() - I::GlobalVars->curtime, invisTime, 0.0f, 0.0f, 100.0f);
		return GetInvisPercent;
	}

	int& m_iCurrentSeed() {
		return *reinterpret_cast<int*>(reinterpret_cast<DWORD>(this) + 0xB5C);
	}
	__inline CAttributeList* GetAttributeList() {
		static auto dwOff = g_NetVars.get_offset(_("DT_EconEntity"), _("m_AttributeManager"), _("m_AttributeList"));
		return reinterpret_cast<CAttributeList*>(this + dwOff);
	}

	__inline void SetItemDefIndex(const int nIndex) {
		static auto dwOff = g_NetVars.get_offset(_("DT_EconEntity"), _("m_AttributeManager"), _("m_Item"), _("m_iItemDefinitionIndex"));
		*reinterpret_cast<int*>(this + dwOff) = nIndex;
	}

	__inline CBaseEntity* GetHealingTarget() {
		return I::ClientEntityList->GetClientEntityFromHandle(GetHealingTargetHandle());
	}

	__inline int GetHealingTargetHandle() {
		return *reinterpret_cast<int*>(this + 0xC48);
	}

	__inline WeaponData_t GetWeaponData() {
		static int offset = g_Pattern.Find(_(L"client.dll"), _(L"55 8B EC 66 8B ? ? 66 3B 05 ? ? ? ? 73"));
		static auto get_tf_weapon_data_fn = reinterpret_cast<CTFWeaponInfo * (__cdecl*)(int)>(offset);
		return get_tf_weapon_data_fn(GetWeaponID())->m_WeaponData[0];
	}

	__inline bool CanFireCriticalShot() {
		static int CanFireCriticalShotOffset = g_Pattern.Find(_(L"client.dll"), _(L"6A 00 68 ? ? ? ? 68 ? ? ? ? 6A 00 E8 ? ? ? ? 50 E8 ? ? ? ? 83 C4 14 C3"));
		static auto CanFireCriticalShotFN = reinterpret_cast<bool* (__cdecl*)(CBaseCombatWeapon*)>(CanFireCriticalShotOffset);
		return CanFireCriticalShotFN(this);
	}

	__inline CTFWeaponInfo* GetTFWeaponInfo() {
		static int GetTFWeaponInfoFNOffset = g_Pattern.Find(L"client.dll", L"55 8B EC FF 75 08 E8 ? ? ? ? 83 C4 04 85 C0 75 02 5D C3 56 50 E8 ? ? ? ? 83 C4 04 0F B7 F0 E8 ? ? ? ? 66 3B F0 75 05 33 C0 5E 5D C3");
		static auto GetTFWeaponInfoFN = reinterpret_cast<CTFWeaponInfo * (__cdecl*)(int)>(GetTFWeaponInfoFNOffset);
		return GetTFWeaponInfoFN(GetWeaponID());
	}

	__inline float GetSwingRange(CBaseEntity* pLocal) {
		return static_cast<float>(GetVFunc<int(__thiscall*)(CBaseEntity*)>(this, 455)(pLocal));
	}

	__inline float GetWeaponSpread() {
		static auto GetWeaponSpreadFn = reinterpret_cast<float(__thiscall*)(decltype(this))>(g_Pattern.Find(_(L"client.dll"), _(L"55 8B EC 83 EC 08 56 8B F1 57 6A 01 6A 00 8B 96 ? ? ? ? 8B 86 ? ? ? ? C1 E2 06 56 68 ? ? ? ? 51")));
		return GetWeaponSpreadFn(this);
	}

	/*__inline bool WillCrit() {
		static auto dwCalcIsAttackCritical = g_Pattern.Find(_(L"client.dll"), _(L"55 8B EC 83 EC 18 56 57 6A 00 68 ? ? ? ? 68 ? ? ? ? 6A 00 8B F9 E8 ? ? ? ? 50 E8 ? ? ? ? 8B F0 83 C4 14 89 75 EC"));
		return reinterpret_cast<bool(__thiscall*)(decltype(this))>(dwCalcIsAttackCritical);
	}*/

	/*__inline bool CalcIsAttackCritical() {
		typedef bool(__thiscall* OriginalFn)(CBaseCombatWeapon*);
		static DWORD dwFunc = g_Pattern.Find(_(L"client.dll"), _(L"55 8B EC 83 EC 18 56 57 6A 00 68 ? ? ? ? 68 ? ? ? ? 6A 00 8B F9 E8 ? ? ? ? 50 E8 ? ? ? ? 8B F0 83 C4 14 89 75 EC"));
		return ((OriginalFn)dwFunc)(this);
	}*/

	__inline bool DoSwingTrace(CGameTrace& Trace) {
		return GetVFunc<int(__thiscall*)(CGameTrace&)>(this, 454)(Trace);
	}

	__inline int LookupAttachment(const char* pAttachmentName)
	{
		const auto pRend = Renderable();
		return GetVFunc<int(__thiscall*)(void*, const char*)>(pRend, 35)(pRend, pAttachmentName);
	}

	__inline bool GetAttachment(int number, Vec3& origin) {
		return GetVFunc<bool(__thiscall*)(void*, int, Vec3&)>(this, 71)(this, number, origin);
	}


	__inline bool CanFireCriticalShot(const bool bHeadShot) {
		bool bResult = false;
		if (const auto& pOwner = I::ClientEntityList->GetClientEntityFromHandle(GethOwner())) {
			const int nOldFov = pOwner->GetFov(); pOwner->SetFov(70);
			bResult = GetVFunc<bool(__thiscall*)(decltype(this), bool, CBaseEntity*)>(this, 425)(this, bHeadShot, nullptr);
			pOwner->SetFov(nOldFov);
		} return bResult;
	}

	__inline bool CanFireRandomCriticalShot(const float flCritChance) {
		return GetVFunc<bool(__thiscall*)(decltype(this), float)>(this, 424)(this, flCritChance);
	}


	__inline bool CanWeaponHeadShot()
	{
		return ((GetDamageType() & DMG_USE_HITLOCATIONS) && CanFireCriticalShot(true)); //credits to bertti
	}

	float GetCurTime(CUserCmd* ucmd, CBaseEntity* pLocal) {
		static int g_tick = 0;
		static CUserCmd* g_pLastCmd = nullptr;
		if (!g_pLastCmd || g_pLastCmd->hasbeenpredicted) {
			g_tick = pLocal->m_nTickBase();
		}
		else {
			// Required because prediction only runs on frames, not ticks
			// So if your framerate goes below tickrate, m_nTickBase won't update every tick
			++g_tick;
		}
		g_pLastCmd = ucmd;
		float curtime = g_tick * I::GlobalVars->interval_per_tick;
		return curtime;
	}

	__inline bool CanShoot(CUserCmd* ucmd, CBaseEntity* pLocal)
	{
		if (!pLocal->IsAlive() || pLocal->IsTaunting() || pLocal->IsBonked() || pLocal->IsAGhost() || pLocal->IsInBumperKart() || pLocal->m_fFlags() & FL_FROZEN)
			return false;

		if (pLocal->GetClassNum() == CLASS_SPY)
		{
			{ //DR
				static float flTimer = 0.0f;

				if (pLocal->GetFeignDeathReady())
				{
					flTimer = 0.0f;
					return false;
				}
				else
				{
					if (!flTimer)
						flTimer = I::GlobalVars->curtime;

					if (flTimer > I::GlobalVars->curtime)
						flTimer = 0.0f;

					if ((I::GlobalVars->curtime - flTimer) < 0.4f)
						return false;
				}
			}

			{ //Invis
				static float flTimer = 0.0f;

				if (pLocal->IsCloaked())
				{
					flTimer = 0.0f;
					return false;
				}
				else
				{
					if (!flTimer)
						flTimer = I::GlobalVars->curtime;

					if (flTimer > I::GlobalVars->curtime)
						flTimer = 0.0f;

					if ((I::GlobalVars->curtime - flTimer) < 2.0f)
						return false;
				}
			}
		}

		float flCurTime = GetCurTime(ucmd, pLocal);

		return GetNextPrimaryAttack() <= flCurTime && pLocal->GetNextAttack() <= flCurTime;
	}

	__inline bool CanSecondaryAttack(CBaseEntity* pLocal)
	{
		if (!pLocal->IsAlive() || pLocal->IsTaunting() || pLocal->IsBonked() || pLocal->IsAGhost() || pLocal->IsInBumperKart())
			return false;

		float flCurTime = static_cast<float>(pLocal->GetTickBase()) * I::GlobalVars->interval_per_tick;

		return GetNextSecondaryAttack() <= flCurTime && pLocal->GetNextAttack() <= flCurTime;
	}

	__inline bool IsInReload()
	{
		static DWORD dwNextPrimaryAttack = g_NetVars.get_offset("DT_BaseCombatWeapon", "LocalActiveWeaponData", "m_flNextPrimaryAttack");
		bool m_bInReload = *reinterpret_cast<bool*>(this + (dwNextPrimaryAttack + 0xC));
		int m_iReloadMode = *reinterpret_cast<int*>(this + 0xB28);
		return (m_bInReload || m_iReloadMode != 0);
	}

	__inline void GetSpreadAngles(Vec3& vOut) {
		static auto GetSpreadAnglesFn = reinterpret_cast<void(__thiscall*)(decltype(this), Vec3&)>(g_Pattern.Find(_(L"client.dll"), _(L"55 8B EC 83 EC 18 56 57 6A 00 68 ? ? ? ? 68 ? ? ? ? 6A 00 8B F9 E8 ? ? ? ? 50 E8 ? ? ? ? 8B F0 83 C4 14 85 F6 74 10 8B 06 8B CE 8B 80 ? ? ? ? FF D0 84 C0 75 02")));
		GetSpreadAnglesFn(this, vOut);
	}

	__inline void GetProjectileFireSetup(CBaseEntity* pPlayer, Vec3 vOffset, Vec3* vSrc, Vec3* vForward, bool bHitTeam, float flEndDist) {
		static auto FN = reinterpret_cast<void(__thiscall*)(CBaseEntity*, CBaseEntity*, Vec3, Vec3*, Vec3*, bool, float)>(g_Pattern.Find(_(L"client.dll"), _(L"53 8B DC 83 EC ? 83 E4 ? 83 C4 ? 55 8B 6B ? 89 6C ? ? 8B EC 81 EC ? ? ? ? 56 8B F1 57 8B 06 8B 80 ? ? ? ? FF D0 84 C0")));
		FN(this, pPlayer, vOffset, vSrc, vForward, bHitTeam, flEndDist);
	}

	__inline bool IsRapidFire()
	{
		const bool ret = GetWeaponData().m_bUseRapidFireCrits;
		return ret || this->GetClientClass()->m_ClassID == static_cast<int>(ETFClassID::CTFMinigun);
	}

	__inline bool WillCrit()
	{
		return this->GetSlot() == SLOT_MELEE ? this->CalcIsAttackCriticalHelperMelee() : this->CalcIsAttackCriticalHelper();
	}

	__inline bool CalcIsAttackCritical()
	{
	
		typedef bool(__thiscall* fn)(void*);
		return reinterpret_cast<fn>(crit())(this);
	}

	__inline bool CalcIsAttackCriticalHelper()
	{
		using FN = bool(__thiscall*)(CBaseCombatWeapon*);
		static FN pCalcIsAttackCriticalHelper = reinterpret_cast<FN>(g_Pattern.Find(_(L"client.dll"), _(L"55 8B EC 83 EC 18 56 57 6A 00 68 ? ? ? ? 68 ? ? ? ? 6A 00 8B F9 E8 ? ? ? ? 50 E8 ? ? ? ? 8B F0 83 C4 14 89 75 EC")));
		if (!pCalcIsAttackCriticalHelper)
		{
			pCalcIsAttackCriticalHelper = (FN)calcisattackcriticaloffset;
			return false;
		}
		return pCalcIsAttackCriticalHelper(this);
	}

	__inline bool CalcIsAttackCriticalHelperMelee()
	{
		using FN = bool(__thiscall*)(CBaseCombatWeapon*);
		static FN pCalcIsAttackCriticalHelper = reinterpret_cast<FN>(g_Pattern.Find(_(L"client.dll"), _(L"55 8B EC A1 ? ? ? ? 83 EC 08 83 78 30 00 57")));
		return pCalcIsAttackCriticalHelper(this);
	}

	__inline bool CalcIsAttackCriticalHelperNoCrits(CBaseEntity* pWeapon)
	{
		typedef bool (*fn_t)(CBaseEntity*);
		return GetVFunc<fn_t>(pWeapon, 463, 0)(pWeapon);
	}

	//__inline bool CanFireCriticalShot(CBaseEntity* pWeapon)		// this does not fucking work no matter what i do and i have no idea why :DDD
	//{
	//	typedef bool (*fn_t)(CBaseEntity*, bool, CBaseEntity*);
	//	return GetVFunc<fn_t>(this, 491)(pWeapon, false, nullptr);
	//}

	__inline Vec3 GetSpreadAngles() {
		Vec3 vOut; GetSpreadAngles(vOut); return vOut;
	}

	__inline int GetMinigunState() {
		return *reinterpret_cast<int*>(this + 0xC48);
	}

	__inline bool IsReadyToFire()
	{
		static float lastFire = 0, nextAttack = 0;

		if (lastFire != GetLastFireTime())
		{
			lastFire = GetLastFireTime();
			nextAttack = GetNextPrimaryAttack();
		}

		if (GetClip1() == 0)
			return false;
		return (nextAttack <= (TICKS_TO_TIME(I::ClientEntityList->GetClientEntity(I::EngineClient->GetLocalPlayer())->GetTickBase())));
	}

	__inline bool IsFlipped()
	{
		static auto cl_flipviewmodels = I::Cvar->FindVar("cl_flipviewmodels");
		return cl_flipviewmodels->GetBool();
	}

	CHudTexture* GetWeaponIcon();
};

class CTFWeaponInvis : public CBaseCombatWeapon
{
public:
	__inline bool HasFeignDeath() {
		return static_cast<bool>(GetVFunc<bool(__thiscall*)(CTFWeaponInvis*)>(this, 522));
	}
};