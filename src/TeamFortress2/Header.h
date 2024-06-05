#pragma once

#include "TeamFortress2/SDK/SDK.h"



class weapon_info
{
public:
    float crit_bucket{};
    unsigned int weapon_seed{};
    unsigned unknown1{};
    unsigned unknown2{};
    bool unknown3{};
    float m_flCritTime{};
    int crit_attempts{};
    int crit_count{};
    float observed_crit_chance{};
    bool unknown7{};
    int weapon_mode{};
    int weapon_data{};
    weapon_info()
    {
    }
    void Load(CBaseCombatWeapon* weapon)
    {
        crit_bucket = *(float*)((uintptr_t)weapon + 2644);
        weapon_seed = *(unsigned int*)((uintptr_t)weapon + 2908);
        unknown1 = *(unsigned int*)((uintptr_t)weapon + 2896);
        unknown2 = *(unsigned int*)((uintptr_t)weapon + 2900);
        unknown3 = *(bool*)((uintptr_t)weapon + 2871);
        m_flCritTime = *(float*)((uintptr_t)weapon + 2912);
        crit_attempts = *(int*)((uintptr_t)weapon + 2648);
        crit_count = *(int*)((uintptr_t)weapon + 2652);
        observed_crit_chance = *(float*)((uintptr_t)weapon + 0xC18);
        unknown7 = *(bool*)((uintptr_t)weapon + 2872);
        weapon_data = *(int*)((uintptr_t)weapon + 2865);
        // No need to restore
        weapon_mode = *(int*)((uintptr_t)weapon + 0xb08);
        weapon_data = *(int*)((uintptr_t)weapon + 0xb14);
    }
    weapon_info(CBaseCombatWeapon* weapon)
    {
        Load(weapon);
    }
    void restore_data(CBaseCombatWeapon* weapon)
    {
        *(float*)((uintptr_t)weapon + 2644) = crit_bucket;
        *(unsigned int*)((uintptr_t)weapon + 2908) = weapon_seed;
        *(unsigned int*)((uintptr_t)weapon + 2896) = unknown1;
        *(unsigned int*)((uintptr_t)weapon + 2900) = unknown2;
        *(bool*)((uintptr_t)weapon + 2871) = unknown3;
        *(float*)((uintptr_t)weapon + 2912) = m_flCritTime;
        *(int*)((uintptr_t)weapon + 2648) = crit_attempts;
        *(int*)((uintptr_t)weapon + 2652) = crit_count;
        *(float*)((uintptr_t)weapon + 0xC18) = observed_crit_chance;
        *(bool*)((uintptr_t)weapon + 2872) = unknown7;
        *(int*)((uintptr_t)weapon + 2865) = weapon_data;
    }
    bool operator==(const weapon_info& B) const
    {
        return crit_bucket == B.crit_bucket && weapon_seed == B.weapon_seed && unknown1 == B.unknown1 && unknown2 == B.unknown2 && unknown3 == B.unknown3 && m_flCritTime == B.m_flCritTime && crit_attempts == B.crit_attempts && crit_count == B.crit_count && observed_crit_chance == B.observed_crit_chance && unknown7 == B.unknown7;
    }
    bool operator!=(const weapon_info& B) const
    {
        return !(*this == B);
    }
};

inline WeaponData_t* GetWeaponData(CBaseCombatWeapon* weapon)
{
    weapon_info info(weapon);
    int WeaponData = info.weapon_data;
    int WeaponMode = info.weapon_mode;
    return (WeaponData_t*)(WeaponData + sizeof(WeaponData_t) * WeaponMode + 1784);
}