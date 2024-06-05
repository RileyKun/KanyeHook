#include "ProjSim.h"

i_physics_environment* env{};
c_physics_object* obj{};

//rebuilt without unneeded things
void GetProjectileFireSetupRebuilt(CBaseEntity* player, Vec3 offset, const Vec3& ang_in, Vec3& pos_out, Vec3& ang_out, bool pipes)
{
	static auto cl_flipviewmodels{ g_ConVars.FindVar("cl_flipviewmodels") };

	if (!cl_flipviewmodels)
	{
		return;
	}

	if (cl_flipviewmodels->GetBool())
	{
		offset.y *= -1.0f;
	}

	Vec3 forward{}, right{}, up{};
	Math::AngleVectors(ang_in, &forward, &right, &up);

	auto shoot_pos{ player->GetShootPos() };

	pos_out = shoot_pos + (forward * offset.x) + (right * offset.y) + (up * offset.z);

	if (pipes)
	{
		ang_out = ang_in;
	}

	else
	{
		auto end_pos{ shoot_pos + (forward * 2000.0f) };

		Math::VectorAngles(end_pos - pos_out, ang_out);
	}
}

bool c_projectile_sim::get_info(CBaseEntity* player, CBaseCombatWeapon* weapon, const Vec3& angles, projectile_info_t& out)
{
	if (!player || !weapon)
	{
		return false;
	}

	auto cur_time{ static_cast<float>(player->m_nTickBase()) * TICK_INTERVAL };
	auto ducking{ player->m_fFlags() & FL_DUCKING };

	Vec3 pos{};
	Vec3 ang{};

	switch (weapon->GetWeaponID())
	{
	case TF_WEAPON_ROCKETLAUNCHER:
	case TF_WEAPON_DIRECTHIT:
	case TF_WEAPON_PARTICLE_CANNON:
    {
		if (weapon->GetItemDefIndex() == Soldier_m_TheOriginal)
		{
			GetProjectileFireSetupRebuilt(player, { 23.5f, 0.0f, ducking ? 8.0f : -3.0f }, angles, pos, ang, false);
		}
		else
		{
			GetProjectileFireSetupRebuilt(player, { 23.5f, 12.0f, ducking ? 8.0f : -3.0f }, angles, pos, ang, false);
		}

		out = { TF_PROJECTILE_ROCKET, pos, ang, Utils::ATTRIB_HOOK_FLOAT(1100.0f, "mult_projectile_speed", weapon, 0,1), 0.0f, false };
		return true;
		break;
	}

	case TF_WEAPON_GRENADELAUNCHER:
	{
		GetProjectileFireSetupRebuilt(player, { 16.0f, 8.0f, -6.0f }, angles, pos, ang, true);

		auto is_lochnload{ G::CurItemDefIndex == Demoman_m_TheLochnLoad };
		auto speed{ is_lochnload ? 1490.0f : 1200.0f };

		out = { TF_PROJECTILE_PIPEBOMB, pos, ang, speed, 1.0f, is_lochnload };

		return true;
		break;
	}

	case TF_WEAPON_PIPEBOMBLAUNCHER:
	{
		GetProjectileFireSetupRebuilt(player, { 16.0f, 8.0f, -6.0f }, angles, pos, ang, true);

		auto charge_begin_time{ weapon->GetChargeBeginTime() };
		auto charge{ cur_time - charge_begin_time };
		auto speed{ Math::RemapValClamped(charge, 0.0f, Utils::ATTRIB_HOOK_FLOAT(4.0f, "stickybomb_charge_rate", weapon,0,1), 900.0f, 2400.0f) };

		if (charge_begin_time <= 0.0f)
		{
			speed = 900.0f;
		}

		out = { TF_PROJECTILE_PIPEBOMB_REMOTE, pos, ang, speed, 1.0f, false };

		return true;
		break;
	}

	case TF_WEAPON_CANNON:
	{
		GetProjectileFireSetupRebuilt(player, { 16.0f, 8.0f, -6.0f }, angles, pos, ang, true);

		out = { TF_PROJECTILE_CANNONBALL, pos, ang, 1454.0f, 1.0f, false };

		return true;
		break;
	}

	case TF_WEAPON_FLAREGUN:
	{
		GetProjectileFireSetupRebuilt(player, { 23.5f, 12.0f, ducking ? 8.0f : -3.0f }, angles, pos, ang, false);

		out = { TF_PROJECTILE_FLARE, pos, ang, 2000.0f, 0.3f, true };

		return true;
		break;
	}
	case TF_WEAPON_COMPOUND_BOW:
	{
		GetProjectileFireSetupRebuilt(player, { 23.5f, 8.0f, -3.0f }, angles, pos, ang, false);

		auto charge_begin_time{ weapon->GetChargeBeginTime() };
		auto charge{ cur_time - charge_begin_time };
		auto speed{ Math::RemapValClamped(charge, 0.0f, 1.0f, 1800.0f, 2600.0f) };
		auto grav_mod{ Math::RemapValClamped(charge, 0.0f, 1.0f, 0.5f, 0.1f) };

		if (charge_begin_time <= 0.0f)
		{
			speed = 1800.0f;
			grav_mod = 0.5f;
		}

		out = { TF_PROJECTILE_ARROW, pos, ang, speed, grav_mod, true };

		return true;
		break;
	}

	case TF_WEAPON_CROSSBOW:
	case TF_WEAPON_SHOTGUN_BUILDING_RESCUE:
	{
		GetProjectileFireSetupRebuilt(player, { 23.5f, 8.0f, -3.0f }, angles, pos, ang, false);

		out = { TF_PROJECTILE_ARROW, pos, ang, 2400.0f, 0.2f, true };

		return true;
		break;
	}

	default:
	{
		return false;
	}
	}
}



Vector Size()
{
	const auto& pLocal = g_EntityCache.GetLocal();
	const auto& pWeapon = g_EntityCache.GetWeapon();

	if (!pLocal || !pWeapon) { return Vector(0, 0, 0); }

	switch (pWeapon->GetWeaponID())
	{
	case TF_WEAPON_ROCKETLAUNCHER:
	case TF_WEAPON_DIRECTHIT:
	case TF_WEAPON_PARTICLE_CANNON:
	case TF_WEAPON_COMPOUND_BOW:
	{
		return Vector(0.1, 0.1, 0.1); //0 but crashes so..
	}
	case TF_WEAPON_GRENADELAUNCHER:
	case TF_WEAPON_PIPEBOMBLAUNCHER:
	case TF_WEAPON_CANNON:
	{
		return Vector(4, 4, 4); 
	}
	case TF_WEAPON_CROSSBOW:
	case TF_WEAPON_SHOTGUN_BUILDING_RESCUE:
	case TF_WEAPON_FLAREGUN:
	{
		return Vector(2, 2, 2); 
	}
	default:
	{
		return Vector(2, 2, 2); 
	}
	}
}

bool c_projectile_sim::init(const  projectile_info_t& info)
{

	if (!env)
	{
		env = I::IPhysics->create_environment();
	}

	if (!obj)
	{
		//it doesn't matter what the size is for non drag affected projectiles
		//pipes use the size below so it works out just fine
		auto col{ I::IPhyicsCollison->bbox_to_collide(Size() * -1, Size() * 1) };

		auto params{ g_phys_default_object_params };

		params.m_damping= 0.0f;
		params.m_rot_damping = 0.0f;
		params.m_inertia = 0.0f;
		params.m_rot_inertia_limit = 0.0f;
		params.m_enable_collisions = false;

	
		obj = reinterpret_cast<c_physics_object*>(env->create_poly_object(col, 0, info.m_position, info.m_angle, &params));
		obj->wake();
	}

	if (!env || !obj)
	{
		return false;
	}

	//set position and velocity
	{
		Vec3 forward{}, up{};

		Math::AngleVectors(info.m_angle, &forward, nullptr, &up);

		Vec3 vel{ forward * info.m_speed };
		Vec3 ang_vel{};

		switch (info.m_type)
		{
		case TF_PROJECTILE_PIPEBOMB:
		case TF_PROJECTILE_PIPEBOMB_REMOTE:
		case TF_PROJECTILE_PIPEBOMB_PRACTICE:
		case TF_PROJECTILE_CANNONBALL:
		{
			//CTFWeaponBaseGun::FirePipeBomb
			//pick your poison

			ang_vel = Vec3(600.0f, Utils::RandInt(-1200.0f, 1200.0f), 0.0f);

			break;
		}

		default:
		{
			break;
		}
		}

		if (info.m_no_spin)
		{
			ang_vel.Zero();
		}

		obj->set_position(info.m_position, info.m_angle, true);
		obj->set_velocity_instantaneous(&vel, &ang_vel);
	}

	//set drag
	{
		float drag{};
		Vec3 drag_basis{};
		Vec3 ang_drag_basis{};

		//these values were dumped from the server by firing the projectiles with 0 0 0 angles
		//they are calculated in CPhysicsObject::RecomputeDragBases
		switch (info.m_type)
		{
		case TF_PROJECTILE_PIPEBOMB:
		{
			drag = 1.0f;
			drag_basis = { 0.003902f, 0.009962f, 0.009962f };
			ang_drag_basis = { 0.003618f, 0.001514f, 0.001514f };

			break;
		}

		case TF_PROJECTILE_PIPEBOMB_REMOTE:
		case TF_PROJECTILE_PIPEBOMB_PRACTICE:
		{
			drag = 1.0f;
			drag_basis = { 0.007491f, 0.007491f, 0.007306f };
			ang_drag_basis = { 0.002777f, 0.002842f, 0.002812f };

			break;
		}

		case TF_PROJECTILE_CANNONBALL:
		{
			drag = 1.0f;
			drag_basis = { 0.020971f, 0.019420f, 0.020971f };
			ang_drag_basis = { 0.012997f, 0.013496f, 0.013714f };

			break;
		}

		default:
		{
			break;
		}
		}

		obj->set_drag_coefficient(&drag, &drag);

		obj->m_drag_basis = drag_basis;
		obj->m_angular_drag_basis = ang_drag_basis;
	}


	//set env params
	{
		auto max_vel{ 1000000.0f };
		auto max_ang_vel{ 1000000.0f };

		//only pipes need k_flMaxVelocity and k_flMaxAngularVelocity
		switch (info.m_type)
		{
		case TF_PROJECTILE_PIPEBOMB:
		case TF_PROJECTILE_PIPEBOMB_REMOTE:
		case TF_PROJECTILE_PIPEBOMB_PRACTICE:
		case TF_PROJECTILE_CANNONBALL:
		{
			max_vel = 2000;
			max_ang_vel = 3600;

			break;
		}

		default:
		{
			break;
		}
		}

		physics_performanceparams_t params;
		params.defaults();

		params.m_max_velocity = max_vel;
		params.m_max_angular_velocity = max_ang_vel;

		env->set_performance_settings(&params);

		env->set_air_density(2.0f);
		env->set_gravity({ 0.0f, 0.0f, -(g_ConVars.sv_gravity->GetFloat() * info.m_gravity_modifier) });

		//env->reset_simulation_clock(); //not needed?
	}

	return true;
}

void c_projectile_sim::run_tick()
{
	if (!env)
	{
		return;
	}

	env->simulate(TICK_INTERVAL);
}

Vec3 c_projectile_sim::get_origin()
{
	if (!obj)
	{
		return {};
	}

	Vec3 out{};

	obj->get_position(&out, nullptr);

	return out;
}



void c_projectile_sim::run_simulation(CBaseEntity* local, CBaseCombatWeapon* weapon, const Vector& angles)
{
	if (!G::projectile_lines.empty())
	{
		G::projectile_lines.clear();
	}

	projectile_info_t info{};

	if (!get_info(local, weapon, angles, info))
	{
		return;
	}

	if (!init(info))
	{
		return;
	}

	for (auto n{ 0 }; n < TIME_TO_TICKS(5.0f); n++)
	{
		run_tick();
		G::projectile_lines.push_back(get_origin());
	}
}