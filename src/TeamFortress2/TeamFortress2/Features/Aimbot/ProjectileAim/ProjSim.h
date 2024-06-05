#pragma once
#include "../../../SDK/SDK.h"


struct projectile_info_t {
	projectile_type_t m_type = { };
	Vector m_position = Vector(), m_angle = Vector();
	float m_speed = 0.0f, m_gravity_modifier = 0.0f;
	bool m_no_spin = false;
};

class c_projectile_sim {
public:
	bool init(const projectile_info_t& info);
	void run_simulation(CBaseEntity* local, CBaseCombatWeapon* weapon, const Vector& angles);
	bool get_info(CBaseEntity* player, CBaseCombatWeapon* weapon, const Vector& angles, projectile_info_t& info);
	void run_tick();
	Vec3 get_origin();
};

ADD_FEATURE(c_projectile_sim, projectile_sim)
