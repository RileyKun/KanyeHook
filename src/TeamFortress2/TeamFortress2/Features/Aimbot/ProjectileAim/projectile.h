#pragma once
#include "../AimbotGlobal/AimbotGlobal.h"
#include "../ProjectileAim/ProjSim.h"

class CAimbotProjectile
{
	struct ProjectileInfo_t
	{
		float m_flVelocity = 0.0f;
		float m_flGravity = 0.0f;
		float m_flMaxTime = 0.0f;
	};

	struct Predictor_t
	{
		CBaseEntity* m_pEntity = nullptr;
		Vec3 m_vPosition = {};
		Vec3 m_vVelocity = {};
		Vec3 m_vAcceleration = {};
	};

	struct Solution_t
	{
		float m_flPitch = 0.0f;
		float m_flYaw = 0.0f;
		float m_flTime = 0.0f;
	};

	projectile_info_t info{};
	float solve_projectile_speed(CBaseCombatWeapon* weapon, const Vector& a, const Vector& b, const ProjectileInfo_t& projInfo);
	bool GetProjectileInfo(CBaseCombatWeapon* pWeapon, ProjectileInfo_t& out);
	bool CalcProjAngle(const Vec3& vLocalPos, const Vec3& vTargetPos, CBaseCombatWeapon* pWeapon, Solution_t& out, const ProjectileInfo_t& projInfo);
	bool SolveProjectile(CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon, CUserCmd* pCmd, Predictor_t& predictor,
		const ProjectileInfo_t& projInfo, Solution_t& out);

	Vec3 GetAimPos(CBaseEntity* pLocal, CBaseEntity* pEntity, const Vec3& targetPredPos);
	Vec3 GetAimPosBuilding(CBaseEntity* pLocal, CBaseEntity* pEntity);
	bool WillProjectileHit(CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon, CUserCmd* pCmd, const Vec3& vPredictedPos, Solution_t& out, const ProjectileInfo_t& projInfo, const Predictor_t& predictor);
	ESortMethod GetSortMethod();
	bool GetTargets(CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon);
	bool VerifyTarget(CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon, CUserCmd* pCmd, Target_t& target);
	bool GetTarget(CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon, CUserCmd* pCmd, Target_t& outTarget);
	void Aim(CUserCmd* pCmd, CBaseCombatWeapon* pWeapon, Vec3& vAngle);
	bool ShouldFire(CUserCmd* pCmd);
	bool IsAttacking(const CUserCmd* pCmd, CBaseCombatWeapon* pWeapon);
	bool GetSplashTarget(CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon, CUserCmd* pCmd, Target_t& outTarget);
	bool GetSplashTargeR(CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon, CUserCmd* pCmd, Target_t& outTarget);
	bool IsFlameThrower = false;
	bool IsBoosted = false;

public:

	bool running = false;
	void Run(CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon, CUserCmd* pCmd);
};

ADD_FEATURE(CAimbotProjectile, AimbotProjectile)