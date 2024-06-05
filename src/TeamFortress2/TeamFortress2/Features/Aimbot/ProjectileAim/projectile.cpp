#include "projectile.h"
#include "../../Vars.h"
#include "movementsim.h"
#include "../../Visuals/Visuals.h"
#include "../../../Hooks/Hooks.h"



MAKE_HOOK(physics_simulate, g_Pattern.Find(L"client.dll", L"55 8B EC A1 ? ? ? ? 83 EC ? 8B 40 ? 56 8B F1 39 86"), void, __fastcall, void* ecx, void* edx)
{
	auto plr = reinterpret_cast<CBaseEntity*>(ecx);
	if (!plr || plr != g_EntityCache.GetLocal())
		return Hook.Original<FN>()(ecx, edx);


	Hook.Original<FN>()(ecx, edx);
}

#include "projectile.h"
#include "../../Vars.h"
#include "movementsim.h"
#include "../../Visuals/Visuals.h"


inline std::vector<Vector> SpherePoints(float radius, int numSamples)
{
	std::vector<Vector> points;
	points.reserve(numSamples);

	const float goldenAngle = 2.39996323f;  // precompute the golden angle

	for (int i = 0; i < numSamples; ++i)
	{
		const float inclination = acosf(1.0f - 2.0f * i / (numSamples - 1));  // compute the inclination angle
		const float azimuth = goldenAngle * i;  // compute the azimuth angle using the golden angle

		const float sin_inc = sinf(inclination);
		const float cos_inc = cosf(inclination);
		const float sin_azi = sinf(azimuth);
		const float cos_azi = cosf(azimuth);

		// compute the sample point on the unit sphere
		const Vector sample = Vector(cos_azi * sin_inc, sin_azi * sin_inc, cos_inc) * radius;

		// translate the point to the desired center
		points.push_back(sample);
	}

	return points;

}
/* Returns the projectile info of a given weapon */
bool CAimbotProjectile::GetProjectileInfo(CBaseCombatWeapon* pWeapon, ProjectileInfo_t& out)
{
	switch (pWeapon->GetWeaponID())
	{
	case TF_WEAPON_ROCKETLAUNCHER:
	case TF_WEAPON_DIRECTHIT:
	case TF_WEAPON_PARTICLE_CANNON:
	{
		IsBoosted = true;
		out = { Utils::ATTRIB_HOOK_FLOAT(1100.0f, "mult_projectile_speed", pWeapon,0,1), 0.0f };
		break;
	}

	case TF_WEAPON_GRENADELAUNCHER:
	{
		bool isLochnLoad = G::CurItemDefIndex == Demoman_m_TheLochnLoad;
		float speed = isLochnLoad ? 1490.0f : 1200.0f;

		IsBoosted = true;
		out = { speed, 1, Utils::ATTRIB_HOOK_FLOAT(3.0f, "fuse_mult", pWeapon,0,1) };
		break;

	}

	case TF_WEAPON_PIPEBOMBLAUNCHER:
	{
		float charge = (I::GlobalVars->curtime - pWeapon->GetChargeBeginTime());
		float speed = Math::RemapValClamped(charge, 0.0f, Utils::ATTRIB_HOOK_FLOAT(4.0f, "stickybomb_charge_rate", pWeapon, 0, 1), 900.0f, 2400.0f);

		if (charge <= 0.0f)
		{
			speed = 900.0f;
		}

		out = { speed, 1 };
		break;
	}

	case TF_WEAPON_CANNON:
	{
		IsBoosted = true;
		out = { 1454.0f, 1.0f };
		break;
	}

	case TF_WEAPON_FLAREGUN:
	{
		out = { 2000.0f, 0.3f };
		break;
	}

	case TF_WEAPON_CLEAVER:
	case TF_WEAPON_RAYGUN_REVENGE:
	{
		out = { 3000.0f, 0.45f };
		break;
	}

	case TF_WEAPON_COMPOUND_BOW:
	{
		float charge = (I::GlobalVars->curtime - pWeapon->GetChargeBeginTime());
		float speed = Math::RemapValClamped(charge, 0.0f, 1.0f, 1800.0f, 2600.0f);
		float grav_mod = Math::RemapValClamped(charge, 0.0f, 1.0f, 0.5f, 0.1f);

		if (charge <= 0.0f)
		{
			speed = 1800.0f;
			grav_mod = 0.5f;
		}


		out = { speed, grav_mod };
		break;
	}

	case TF_WEAPON_SYRINGEGUN_MEDIC:
	{
		out = { 1000.0f, 0.2f };
		break;
	}

	case TF_WEAPON_FLAMETHROWER:
	{
		out = { 1000.0f, 0.0f, 0.33f };
		IsFlameThrower = true;
		break;
	}

	case TF_WEAPON_FLAME_BALL: //dragon's fury
	{
		out = { 3000.0f, 0.0f, 0.1753f };
		IsFlameThrower = true;
		break;
	}

	case TF_WEAPON_RAYGUN:
	{
		out = { 1200.0f, 0.0f };
		break;
	}

	case TF_WEAPON_CROSSBOW:
	case TF_WEAPON_SHOTGUN_BUILDING_RESCUE:
	{
		out = { 2400.0f, 0.2f };
		break;
	}
	}

	return out.m_flVelocity;
}


float CAimbotProjectile::solve_projectile_speed(CBaseCombatWeapon* weapon, const Vector& a, const Vector& b, const ProjectileInfo_t& projInfo) {
	if (weapon->GetWeaponID() != TF_WEAPON_GRENADELAUNCHER && weapon->GetWeaponID() != TF_WEAPON_PIPEBOMBLAUNCHER)
		return  projInfo.m_flVelocity;

	const auto projectile_speed = projInfo.m_flVelocity;
	const auto projectile_gravity = 800.0f * projInfo.m_flGravity;

	const auto delta = (b - a);
	const auto delta_length = delta.Length2D();

	const auto root = powf(projectile_speed, 4.0f) - projectile_gravity * (projectile_gravity * powf(delta_length, 2.0f) + 2.0f * delta.z * powf(projectile_speed, 2.0f));

	if (root < 0.0f)
		return 0.0f;

	const auto pitch = atanf((powf(projInfo.m_flVelocity, 2.0f) - sqrtf(root)) / (projectile_gravity * (delta_length)));
	const auto time = delta_length / (cosf(pitch) * projectile_speed);

	auto drag_force = 1.0f;

	// TODO: Improve this
	// HINT: Just fuck with them until they work nice :)

	switch (G::CurItemDefIndex) {
	case Demoman_m_GrenadeLauncher:
	case Demoman_m_GrenadeLauncherR:
	case Demoman_m_FestiveGrenadeLauncher:
	case Demoman_m_TheIronBomber:
	case Demoman_m_Autumn:
	case Demoman_m_MacabreWeb:
	case Demoman_m_Rainbow:
	case Demoman_m_SweetDreams:
	case Demoman_m_CoffinNail:
	case Demoman_m_TopShelf:
	case Demoman_m_Warhawk:
	case Demoman_m_ButcherBird: {
		drag_force = 0.16f;
		break;
	}

	case Demoman_m_TheLochnLoad: {
		drag_force = 0.08f;
		break;
	}

	case Demoman_m_TheLooseCannon: {
		drag_force = 0.49f;
		break;
	}

	case Demoman_s_StickybombLauncher:
	case Demoman_s_StickybombLauncherR:
	case Demoman_s_FestiveStickybombLauncher:
	case Demoman_s_TheQuickiebombLauncher:
	case Demoman_s_TheScottishResistance: {
		drag_force = 0.01f;
		break;
	}
	}

	return projectile_speed - (projectile_speed * time) * drag_force;
}

bool CAimbotProjectile::CalcProjAngle(const Vec3& vLocalPos, const Vec3& vTargetPos, CBaseCombatWeapon* pWeapon, Solution_t& out,const ProjectileInfo_t& projInfo)
{
	const float fGravity = g_ConVars.sv_gravity->GetFloat() * projInfo.m_flGravity;
	const Vec3 vDelta = vTargetPos - vLocalPos;
	const float fHyp = sqrt(vDelta.x * vDelta.x + vDelta.y * vDelta.y);
	const float fDist = vDelta.z;
	const float fVel = solve_projectile_speed(pWeapon, vLocalPos, vTargetPos, projInfo);

	if (!fGravity)
	{
		const Vec3 vAngleTo = Math::CalcAngle(vLocalPos, vTargetPos);
		out.m_flPitch = -DEG2RAD(vAngleTo.x);
		out.m_flYaw = DEG2RAD(vAngleTo.y);
	}
	else
	{	//	arch
		const float fRoot = pow(fVel, 4) - fGravity * (fGravity * pow(fHyp, 2) + 2.f * fDist * pow(fVel, 2));
		if (fRoot < 0.f)
		{
			return false;
		}
		out.m_flPitch = atan((pow(fVel, 2) - sqrt(fRoot)) / (fGravity * fHyp));
		out.m_flYaw = atan2(vDelta.y, vDelta.x);
	}
	out.m_flTime = fHyp / (cos(out.m_flPitch) * fVel);

	return true;
}

bool CAimbotProjectile::SolveProjectile(CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon, CUserCmd* pCmd, Predictor_t& predictor, const ProjectileInfo_t& projInfo, Solution_t& out)
{
	const INetChannel* pNetChannel = I::EngineClient->GetNetChannelInfo();
	static ConVar* cl_flipviewmodels = g_ConVars.cl_flipviewmodels;

	G::PredictionLines.clear();

	if (!G::WeaponCanAttack) {
		return true;	// we can't attack, so it shouldn't matter if we say we've solved it, also shouldn't f wit da chammies iykyk
	}

	if (!pNetChannel || !cl_flipviewmodels)
	{
		return false;
	}

	static bool oValue = cl_flipviewmodels->GetBool(); // assume false
	if (Vars::Debug::DebugInfo.Value) {
		cl_flipviewmodels->SetValue(oValue);
	}

	Ray_t traceRay = {};
	CGameTrace trace = {};
	static CTraceFilterWorldAndPropsOnly traceFilter = {};
	traceFilter.pSkip = predictor.m_pEntity;

	Vec3 vLocalPos = pLocal->GetEyePosition();
	const float maxTime = predictor.m_pEntity->IsPlayer() ? (projInfo.m_flMaxTime == 0.f ? Vars::Aimbot::Projectile::PredictionTime.Value : projInfo.m_flMaxTime) : (projInfo.m_flMaxTime == 0.f ? 1024.f : projInfo.m_flMaxTime);
	const float fLatency = pNetChannel->GetLatency(0);

	/*
			This should now be able to predict anything that moves.
			Should also stop wasting time predicting static players.
	*/
	const bool useTPred = !predictor.m_pEntity->GetVecVelocity().IsZero() ? true : false;

	if (!useTPred) {

		Vec3 staticPos = predictor.m_pEntity->IsPlayer() ? GetAimPos(pLocal, predictor.m_pEntity, predictor.m_vPosition) : GetAimPosBuilding(pLocal, predictor.m_pEntity);
		if (staticPos.IsZero()) {
			return false;
		}

	
		switch (pWeapon->GetWeaponID())
		{
		case TF_WEAPON_GRENADELAUNCHER:
		case TF_WEAPON_PIPEBOMBLAUNCHER:
		case TF_WEAPON_STICKBOMB:
		case TF_WEAPON_STICKY_BALL_LAUNCHER:
		{
			Vec3 vecOffset(16.0f, 8.0f, -6.0f);
			Utils::GetProjectileFireSetup(pLocal, pCmd->viewangles, vecOffset, &vLocalPos);
			break;
		}

		default: break;
		}

		if (pWeapon->GetWeaponID() == (TF_WEAPON_GRENADELAUNCHER || TF_WEAPON_PIPEBOMBLAUNCHER)) {
			auto forward = Vector(), up = Vector();
			Math::AngleVectors(Vector(-RAD2DEG(out.m_flPitch), RAD2DEG(out.m_flYaw), 0), &forward, nullptr, &up);

			auto velocity = ((forward * solve_projectile_speed(pWeapon, vLocalPos, staticPos, projInfo)) - (up * 200.0f)), angle = Vector();
			Math::VectorAngles(velocity, angle);

			pCmd->viewangles.x = -DEG2RAD(angle.x);
		}

		if (!CalcProjAngle(vLocalPos, staticPos, pWeapon, out, projInfo))
		{
			return false;
		}

		if (out.m_flTime > maxTime) {
			return false;
		}

		if (WillProjectileHit(pLocal, pWeapon, pCmd, staticPos, out, projInfo, predictor)) {
			G::PredictedPos = staticPos;
			return true;
		}
	}
	else {
		Vec3 vPredictedPos = {};
		CMoveData moveData = {};
		Vec3 absOrigin = {};

		if (F::MoveSim.Initialize(predictor.m_pEntity))
		{
			for (int n = 0; n < TIME_TO_TICKS(maxTime); n++)
			{
				if (predictor.m_pEntity == nullptr)
				{
					break;
				}
				F::MoveSim.RunTick(moveData, absOrigin);
				vPredictedPos = absOrigin;

				const Vec3 aimPosition = GetAimPos(pLocal, predictor.m_pEntity, vPredictedPos);
				if (aimPosition.IsZero()) {
					break;
				} // don't remove.

				//const Vec3 vAimDelta = predictor.m_pEntity->GetAbsOrigin() - aimPosition;
				//vPredictedPos.x += abs(vAimDelta.x);
				//vPredictedPos.y += abs(vAimDelta.y);
				//vPredictedPos.z += abs(vAimDelta.z);
				vPredictedPos = aimPosition;

				//Utils::TraceHull(predictor.m_vPosition, vPredictedPos, Vec3(-3.8f, -3.8f, -3.8f), Vec3(3.8f, 3.8f, 3.8f),
				//	MASK_SOLID_BRUSHONLY, &traceFilter, &trace);

				//if (trace.DidHit())
				//{
				//	vPredictedPos.z = trace.vEndPos.z;
				G::PredictedPos = vPredictedPos;
	

				switch (pWeapon->GetWeaponID())
				{
				case TF_WEAPON_GRENADELAUNCHER:
				case TF_WEAPON_PIPEBOMBLAUNCHER:
				{
					Vec3 vecOffset(16.0f, 8.0f, -6.0f);
					Utils::GetProjectileFireSetup(pLocal, pCmd->viewangles, vecOffset, &vLocalPos);
					break;
				}

				default: break;
				}

				if (pWeapon->GetWeaponID() == (TF_WEAPON_GRENADELAUNCHER || TF_WEAPON_PIPEBOMBLAUNCHER)) {
					auto forward = Vector(), up = Vector();
					Math::AngleVectors(Vector(-RAD2DEG(out.m_flPitch), RAD2DEG(out.m_flYaw),0), &forward, nullptr, &up);

					auto velocity = ((forward * solve_projectile_speed(pWeapon, vLocalPos, vPredictedPos, projInfo)) - (up * 200.0f)), angle = Vector();
					Math::VectorAngles(velocity, angle);

				 pCmd->viewangles.x = -DEG2RAD(angle.x);
				}

				if (!CalcProjAngle(vLocalPos, vPredictedPos, pWeapon, out, projInfo))
				{
					break;
				}

				out.m_flTime += fLatency;

				if (out.m_flTime < TICKS_TO_TIME(n))
				{
					if (WillProjectileHit(pLocal, pWeapon, pCmd, vPredictedPos, out, projInfo, predictor)) {

						G::PredictedPos = vPredictedPos;
						I::DebugOverlay->ClearAllOverlays();
						I::DebugOverlay->AddBoxOverlay2(absOrigin, predictor.m_pEntity->m_vecMins(), predictor.m_pEntity->m_vecMaxs(), Vec3(), { 0,0,0,0 }, { 255,255,255,255 }, 4);
						F::MoveSim.Restore();
						return true;
					}
				}
			}
			F::MoveSim.Restore();
		}
	}
	return false;
}


Vec3 getHeadOffset(CBaseEntity* pEntity) {
	const Vec3 headPos = pEntity->GetHitboxPos(HITBOX_HEAD);
	const Vec3 entPos = pEntity->GetAbsOrigin();
	const Vec3 delta = entPos - headPos;
	return delta * -1.f;
}

bool IsPointAllowed(int nHitbox) {
	switch (nHitbox) {
	case 0: return Vars::Aimbot::Projectile::AllowedHitboxes.Value & (1 << 0);
	case 1: return Vars::Aimbot::Projectile::AllowedHitboxes.Value & (1 << 1);
	case 2: return Vars::Aimbot::Projectile::AllowedHitboxes.Value & (1 << 2);
	case 3:
	case 4:
	case 5:
	case 6: return Vars::Aimbot::Projectile::AllowedHitboxes.Value & (1 << 0);
	case 7:
	case 8:
	case 9:
	case 10: return Vars::Aimbot::Projectile::AllowedHitboxes.Value & (1 << 1);
	case 11:
	case 12:
	case 13:
	case 14: return Vars::Aimbot::Projectile::AllowedHitboxes.Value & (1 << 2);
	}
	return true; // never
}


//	Tries to find the best position to aim at on our target.
Vec3 CAimbotProjectile::GetAimPos(CBaseEntity* pLocal, CBaseEntity* pEntity, const Vec3& targetPredPos)
{
	Vec3 retVec = pLocal->GetAbsOrigin();
	Vec3 localPos = pLocal->GetAbsOrigin();

	const Vec3 vLocalPos = pLocal->GetShootPos();

	const bool bIsDucking = (pEntity->m_bDucked() || pEntity->m_fFlags() & FL_DUCKING);

	const float bboxScale = 1.0f; // stop shoot flor (:D)

	// this way overshoots players that are crouching and I don't know why.
	const Vec3 vMaxs = I::GameMovement->GetPlayerMaxs(bIsDucking) * bboxScale;
	const Vec3 vMins = Vec3(-vMaxs.x, -vMaxs.y, vMaxs.z - vMaxs.z * bboxScale);

	const Vec3 headDelta = getHeadOffset(pEntity);

	const std::vector vecPoints = {	// oh you don't like 15 points because it fucks your fps??? TOO BAD!//
		Vec3(headDelta.x, headDelta.y, vMaxs.z),				//	head bone probably
		Vec3(0, 0, (vMins.z + vMaxs.z) / 2),					//	middles (scan first bc they are more accurate)
		Vec3(0, 0, vMins.z),									//	-
		Vec3(vMins.x, vMins.y, vMaxs.z),						//	top four corners
		Vec3(vMins.x, vMaxs.y, vMaxs.z),						//	-
		Vec3(vMaxs.x, vMaxs.y, vMaxs.z),						//	-
		Vec3(vMaxs.x, vMins.y, vMaxs.z),						//	-
		Vec3(vMins.x, vMins.y, (vMins.z + vMaxs.z) / 2),		//	middle four corners
		Vec3(vMins.x, vMaxs.y, (vMins.z + vMaxs.z) / 2),		//	-
		Vec3(vMaxs.x, vMaxs.y, (vMins.z + vMaxs.z) / 2),		//	-
		Vec3(vMaxs.x, vMins.y, (vMins.z + vMaxs.z) / 2),		//	-
		Vec3(vMins.x, vMins.y, vMins.z),						//	bottom four corners
		Vec3(vMins.x, vMaxs.y, vMins.z),						//	-
		Vec3(vMaxs.x, vMaxs.y, vMins.z),						//	-
		Vec3(vMaxs.x, vMins.y, vMins.z)							//	-
	};

	std::vector<Vec3> visiblePoints{};
	const matrix3x4 transform = {
		{1.f, 0, 0, targetPredPos.x},
		{0, 1.f, 0, targetPredPos.y},
		{0, 0, 1.f, pEntity->GetVecVelocity().IsZero() ? pEntity->GetAbsOrigin().z : targetPredPos.z}
	};

	int aimMethod = Vars::Aimbot::Projectile::AimPosition.Value;
	int curPoint = 0, testPoints = 0; //maybe better way to do this
	for (const auto& point : vecPoints)
	{
		if (testPoints > Vars::Aimbot::Projectile::VisTestPoints.Value) { break; }
		if (static_cast<int>(visiblePoints.size()) >= Vars::Aimbot::Projectile::ScanPoints.Value) { break; }
		if (!IsPointAllowed(curPoint)) { curPoint++; continue; }

		Vec3 vTransformed = {};
		Math::VectorTransform(point, transform, vTransformed);

		if (Utils::VisPos(pLocal, pEntity, vLocalPos, vTransformed))
		{
			if (curPoint == aimMethod && aimMethod < 3) { return vTransformed; }	// return this value now if it is going to get returned anyway, avoid useless scanning.
			visiblePoints.push_back(vTransformed);
		}
		curPoint++;
		testPoints++;	// Only increment this if we actually tested.
	}
	if (visiblePoints.empty()) {
		return Vec3(0, 0, 0);
	}

	Vec3 HeadPoint, TorsoPoint, FeetPoint;

	const int classNum = pLocal->GetClassNum();

	switch (classNum) {
	case CLASS_SOLDIER:
	case CLASS_DEMOMAN:
	{
		if (Vars::Aimbot::Projectile::FeetAimIfOnGround.Value && pEntity->IsOnGround()) {
			aimMethod = 2;
		}
		break;
	}
	}

	if (aimMethod == 3 && classNum) { // auto
		switch (classNum) {
		case CLASS_SOLDIER:
		case CLASS_DEMOMAN:
		{
			aimMethod = 1;
			break;
		}
		case CLASS_SNIPER:
		{
			aimMethod = 0;
			break;
		}
		default:
		{
			aimMethod = 1;
			break;
		}
		}
	}

	switch (aimMethod) {
	case 0: {	//head
		Math::VectorTransform(vecPoints.at(0), transform, HeadPoint);			//	get transformed location of our "best point" for our selected prio hitbox
		for (const auto& aimPoint : visiblePoints) {							//	iterate through visible points
			if (aimPoint.DistTo(HeadPoint) < retVec.DistTo(HeadPoint)) {		//	if the distance to our best point is lower than the previous selected point,
				retVec = aimPoint;												//	set the new point to our currently selected point
			}
		}
		break;
	}
	case 1: {	//torso
		Math::VectorTransform(vecPoints.at(1), transform, TorsoPoint);
		for (const auto& aimPoint : visiblePoints) {
			if (aimPoint.DistTo(TorsoPoint) < retVec.DistTo(TorsoPoint)) {
				retVec = aimPoint;
			}
		}
		break;
	}
	case 2: {	//feet
		Math::VectorTransform(vecPoints.at(2), transform, FeetPoint);
		for (const auto& aimPoint : visiblePoints) {
			if (aimPoint.DistTo(FeetPoint) < retVec.DistTo(FeetPoint)) {
				retVec = aimPoint;
			}
		}
		break;
	}
	}
	return retVec;
}

Vec3 CAimbotProjectile::GetAimPosBuilding(CBaseEntity* pLocal, CBaseEntity* pEntity) {
	Vec3 retVec = pLocal->GetAbsOrigin();
	Vec3 localPos = pLocal->GetAbsOrigin();

	const Vec3 vLocalPos = pLocal->GetShootPos();

	const float bboxScale = std::max(Vars::Aimbot::Projectile::ScanScale.Value - 0.05f, 0.5f);	// set the maximum scale for buildings at .95f

	const Vec3 vMins = pEntity->GetCollideableMins() * bboxScale;
	const Vec3 vMaxs = pEntity->GetCollideableMaxs() * bboxScale;

	const std::vector vecPoints = {
		Vec3(vMaxs.x / 2, vMaxs.y / 2, vMaxs.z / 2),								//	middle
		Vec3(vMins.x, vMins.y, vMaxs.z),											//	top four corners
		Vec3(vMins.x, vMaxs.y, vMaxs.z),											//	-
		Vec3(vMaxs.x, vMaxs.y, vMaxs.z),											//	-
		Vec3(vMaxs.x, vMins.y, vMaxs.z),											//	-
		Vec3(vMins.x, vMins.y, vMins.z),											//	bottom four corners
		Vec3(vMins.x, vMaxs.y, vMins.z),											//	-
		Vec3(vMaxs.x, vMaxs.y, vMins.z),											//	-
		Vec3(vMaxs.x, vMins.y, vMins.z)												//	-
	};

	const matrix3x4& transform = pEntity->GetRgflCoordinateFrame();

	for (const auto& point : vecPoints)
	{
		Vec3 vTransformed = {};
		Math::VectorTransform(point, transform, vTransformed);

		if (Utils::VisPos(pLocal, pEntity, vLocalPos, vTransformed))
		{
			return vTransformed; // just return the first point we see
		}
	}
	return Vec3(0, 0, 0);
}

bool CAimbotProjectile::WillProjectileHit(CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon, CUserCmd* pCmd, const Vec3& vPredictedPos, Solution_t& out, const ProjectileInfo_t& projInfo,
	const Predictor_t& predictor)
{
	Vec3 hullSize = { 3.8f, 3.8f, 3.8f };
	Vec3 vVisCheck = pLocal->GetEyePosition();
	const Vec3 predictedViewAngles = { -RAD2DEG(out.m_flPitch), RAD2DEG(out.m_flYaw), 0.0f };
	CGameTrace trace = {};
	static CTraceFilterWorldAndPropsOnly traceFilter = {};
	traceFilter.pSkip = predictor.m_pEntity;

	// this shit's messy
	{
		switch (pWeapon->GetWeaponID())
		{
		case TF_WEAPON_PARTICLE_CANNON: {
			hullSize = { 1.f, 1.f, 1.f };
			Vec3 vecOffset(23.5f, 12.0f, -3.0f); //tf_weaponbase_gun.cpp @L529 & @L760
			if (pLocal->IsDucking())
			{
				vecOffset.z = 8.0f;
			}
			Utils::GetProjectileFireSetup(pLocal, predictedViewAngles, vecOffset, &vVisCheck);
			break;
		}
		case TF_WEAPON_CROSSBOW: {
			hullSize = { 3.f, 3.f, 3.f };
			const Vec3 vecOffset(23.5f, 12.0f, -3.0f);
			Utils::GetProjectileFireSetup(pLocal, predictedViewAngles, vecOffset, &vVisCheck);
			break;
		}
		case TF_WEAPON_RAYGUN_REVENGE:
		case TF_WEAPON_ROCKETLAUNCHER:
		case TF_WEAPON_DIRECTHIT:
		{
			hullSize = { 0.f, 3.7f, 3.7f };

			Vec3 vecOffset(23.5f, 12.0f, -3.0f); //tf_weaponbase_gun.cpp @L529 & @L760
			if (pLocal->IsDucking())
			{
				vecOffset.z = 8.0f;
			}
			Utils::GetProjectileFireSetup(pLocal, predictedViewAngles, vecOffset, &vVisCheck);
			break;
		}
		case TF_WEAPON_SYRINGEGUN_MEDIC:
		{
			hullSize = { 0.f, 1.f, 1.f };

			const Vec3 vecOffset(16.f, 6.f, -8.f); //tf_weaponbase_gun.cpp @L628
			Utils::GetProjectileFireSetup(pLocal, predictedViewAngles, vecOffset, &vVisCheck);
			break;
		}
		case TF_WEAPON_COMPOUND_BOW:
		{
			hullSize = { 0.f, 1.f, 1.f };

			const Vec3 vecOffset(23.5f, 12.0f, -3.0f); //tf_weapon_grapplinghook.cpp @L355 ??
			Utils::GetProjectileFireSetup(pLocal, predictedViewAngles, vecOffset, &vVisCheck);
			break;
		}
		case TF_WEAPON_RAYGUN:
		case TF_WEAPON_DRG_POMSON:
		{
			hullSize = { 0.1f, 0.1f, 0.1f };
			Vec3 vecOffset(23.5f, -8.0f, -3.0f); //tf_weaponbase_gun.cpp @L568
			if (pLocal->IsDucking())
			{
				vecOffset.z = 8.0f;
			}
			Utils::GetProjectileFireSetup(pLocal, predictedViewAngles, vecOffset, &vVisCheck);
			break;
		}
		case TF_WEAPON_GRENADELAUNCHER:
		case TF_WEAPON_PIPEBOMBLAUNCHER:
		case TF_WEAPON_STICKBOMB:
		case TF_WEAPON_STICKY_BALL_LAUNCHER:
		{

			hullSize = { 4.f, 4.f, 4.f };

			auto vecAngle = Vec3(), vecForward = Vec3(), vecRight = Vec3(), vecUp = Vec3();
			Math::AngleVectors({ -RAD2DEG(out.m_flPitch), RAD2DEG(out.m_flYaw), 0.0f }, &vecForward, &vecRight, &vecUp);
			const Vec3 vecVelocity = ((vecForward * projInfo.m_flVelocity) - (vecUp * 200.0f));
			Math::VectorAngles(vecVelocity, vecAngle);
			out.m_flPitch = -DEG2RAD(vecAngle.x);

			// see relevant code @tf_weaponbase_gun.cpp L684
			const float fRight = g_ConVars.cl_flipviewmodels->GetInt() ? -8.f : 8.f;
			vVisCheck = pLocal->GetShootPos() + vecForward * 16.0f + vecRight * fRight + vecUp * -6.0f;

			break;
		}
		default: break;
		}
	}


	F::projectile_sim.run_simulation(pLocal, pWeapon, predictedViewAngles);
	if (G::projectile_lines.empty())
	{
		return false;
	}

	for (size_t i = 1; i < G::projectile_lines.size(); i++)
	{
		Utils::TraceHull(G::projectile_lines.at(i - 1), G::projectile_lines.at(i), hullSize * 1.01f, hullSize * -1.01f, MASK_SHOT_HULL, &traceFilter, &trace);
	}

	return !trace.DidHit() && !trace.bStartSolid;
}

ESortMethod CAimbotProjectile::GetSortMethod()
{
	switch (Vars::Aimbot::Projectile::SortMethod.Value)
	{
	case 0: return ESortMethod::FOV;
	case 1: return ESortMethod::DISTANCE;
	default: return ESortMethod::UNKNOWN;
	}
}



bool CAimbotProjectile::GetTargets(CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon)
{
	const ESortMethod sortMethod = GetSortMethod();

	F::AimbotGlobal.m_vecTargets.clear();

	const Vec3 vLocalPos = pLocal->GetShootPos();
	const Vec3 vLocalAngles = I::EngineClient->GetViewAngles();

	// Players
	if (Vars::Aimbot::Global::AimPlayers.Value)
	{
		const int nWeaponID = pWeapon->GetWeaponID();
		const bool bIsCrossbow = nWeaponID == TF_WEAPON_CROSSBOW;

		for (const auto& pTarget : g_EntityCache.GetGroup(
			bIsCrossbow ? EGroupType::PLAYERS_ALL : EGroupType::PLAYERS_ENEMIES))
		{
			if (!pTarget->IsAlive() || pTarget->IsAGhost() || pTarget == pLocal || (bIsCrossbow && (pTarget->GetHealth() >=
				pTarget->GetMaxHealth()) && (pTarget->GetTeamNum() == pLocal->GetTeamNum())))
			{
				continue;
			}

			if (pTarget->GetTeamNum() != pLocal->GetTeamNum())
			{
				if (F::AimbotGlobal.ShouldIgnore(pTarget)) { continue; }
			}

			Vec3 vPos = pTarget->GetWorldSpaceCenter();
			Vec3 vAngleTo = Math::CalcAngle(vLocalPos, vPos);
			const float flFOVTo = Math::CalcFov(vLocalAngles, vAngleTo);


			if ((sortMethod == ESortMethod::FOV || Vars::Aimbot::Projectile::RespectFOV.Value) && flFOVTo > Vars::Aimbot::Global::AimFOV.Value)
			{
				continue;
			}
			const float flDistTo = (sortMethod == ESortMethod::DISTANCE) ? vLocalPos.DistTo(vPos) : 0.0f;
			const uint32_t priorityID = g_EntityCache.GetPR()->GetValid(pTarget->GetIndex()) ? g_EntityCache.GetPR()->GetAccountID(pTarget->GetIndex()) : 0;
			const auto& priority = G::PlayerPriority[priorityID];

			F::AimbotGlobal.m_vecTargets.push_back({ pTarget, ETargetType::PLAYER, vPos, vAngleTo, flFOVTo, flDistTo, -1, false, priority });
		}
	}

	// Buildings
	if (Vars::Aimbot::Global::AimBuildings.Value)
	{
		const bool bIsRescueRanger = pWeapon->GetWeaponID() == TF_WEAPON_SHOTGUN_BUILDING_RESCUE;

		for (const auto& pBuilding : g_EntityCache.GetGroup(
			bIsRescueRanger ? EGroupType::BUILDINGS_ALL : EGroupType::BUILDINGS_ENEMIES))
		{
			if (!pBuilding->IsAlive())
			{
				continue;
			}

			Vec3 vPos = pBuilding->GetWorldSpaceCenter();
			Vec3 vAngleTo = Math::CalcAngle(vLocalPos, vPos);
			const float flFOVTo = Math::CalcFov(vLocalAngles, vAngleTo);


			if ((sortMethod == ESortMethod::FOV || Vars::Aimbot::Projectile::RespectFOV.Value) && flFOVTo > Vars::Aimbot::Global::AimFOV.Value)
			{
				continue;
			}

			const float flDistTo = sortMethod == ESortMethod::DISTANCE ? vLocalPos.DistTo(vPos) : 0.0f;
			F::AimbotGlobal.m_vecTargets.push_back({ pBuilding, ETargetType::BUILDING, vPos, vAngleTo, flFOVTo, flDistTo });
		}
	}

	return !F::AimbotGlobal.m_vecTargets.empty();
}

bool CAimbotProjectile::VerifyTarget(CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon, CUserCmd* pCmd, Target_t& target)
{
	ProjectileInfo_t projInfo = {};


	if (!GetProjectileInfo(pWeapon, projInfo))
	{
		return false;
	}

	if (pLocal->IsPrecisionRune() && IsBoosted) {
		projInfo.m_flVelocity *= 2.5f;

	}

	auto vVelocity = Vec3();
	auto vAcceleration = Vec3();

	switch (target.m_TargetType)
	{
	case ETargetType::PLAYER:
	{
		vVelocity = target.m_pEntity->GetVelocity();
		vAcceleration = Vec3(0.0f, 0.0f, g_ConVars.sv_gravity->GetFloat() * ((target.m_pEntity->GetCondEx2() & TFCondEx2_Parachute) ? 0.224f : 1.0f));
		break;
	}

	default: break;
	}

	Predictor_t predictor = {
		target.m_pEntity,
		target.m_vPos,
		vVelocity,
		vAcceleration
	};

	Solution_t solution = {};

	if (!SolveProjectile(pLocal, pWeapon, pCmd, predictor, projInfo, solution))
	{
		return false;
	}

	target.m_vAngleTo = { -RAD2DEG(solution.m_flPitch), RAD2DEG(solution.m_flYaw), 0.0f };

	return true;
}

// Returns the best target
bool CAimbotProjectile::GetTarget(CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon, CUserCmd* pCmd, Target_t& outTarget)
{
	if (!GetTargets(pLocal, pWeapon))
	{
		return false;
	}

	F::AimbotGlobal.SortTargets(GetSortMethod());

	//instead of this just limit to like 4-6 targets, should save perf without any noticeable changes in functionality
	for (auto& target : F::AimbotGlobal.m_vecTargets)
	{
		if (!VerifyTarget(pLocal, pWeapon, pCmd, target))
		{
			continue;
		}

		outTarget = target;
		return true;
	}

	return false;
}

// Aims at the given angles
void CAimbotProjectile::Aim(CUserCmd* pCmd, CBaseCombatWeapon* pWeapon, Vec3& vAngle)
{
	vAngle -= G::PunchAngles;
	Math::ClampAngles(vAngle);

	switch (Vars::Aimbot::Projectile::AimMethod.Value)
	{
	case 0:
	{
		// Plain
		pCmd->viewangles = vAngle;
		I::EngineClient->SetViewAngles(pCmd->viewangles);
		break;
	}

	case 1:
	{
		// Silent
		Utils::FixMovement(pCmd, vAngle);
		pCmd->viewangles = vAngle;
		break;
	}

	default: break;
	}
}

bool CAimbotProjectile::ShouldFire(CUserCmd* pCmd)
{
	return (Vars::Aimbot::Global::AutoShoot.Value && G::WeaponCanAttack);
}

bool CAimbotProjectile::IsAttacking(const CUserCmd* pCmd, CBaseCombatWeapon* pWeapon)
{
	if (G::CurItemDefIndex == Soldier_m_TheBeggarsBazooka)
	{
		static bool bLoading = false;

		if (pWeapon->GetClip1() > 0)
		{
			bLoading = true;
		}

		if (!(pCmd->buttons & IN_ATTACK) && bLoading)
		{
			bLoading = false;
			return true;
		}
	}
	else
	{
		if (pWeapon->GetWeaponID() == TF_WEAPON_COMPOUND_BOW || pWeapon->GetWeaponID() == TF_WEAPON_PIPEBOMBLAUNCHER)
		{
			static bool bCharging = false;

			if (pWeapon->GetChargeBeginTime() > 0.0f)
			{
				bCharging = true;
			}

			if (!(pCmd->buttons & IN_ATTACK) && bCharging)
			{
				bCharging = false;
				return true;
			}
		}
		else if (pWeapon->GetWeaponID() == TF_WEAPON_CANNON)
		{
			static bool Charging = false;

			if (pWeapon->GetDetonateTime() > 0.0f)
				Charging = true;

			if (!(pCmd->buttons & IN_ATTACK) && Charging)
			{
				Charging = false;
				return true;
			}
		}

		//pssst..
		//Dragon's Fury has a gauge (seen on the weapon model) maybe it would help for pSilent hmm..
		/*
		if (pWeapon->GetWeaponID() == 109) {
		}*/

		else
		{
			if ((pCmd->buttons & IN_ATTACK) && G::WeaponCanAttack)
			{
				return true;
			}
		}
	}

	return false;
}

// Returns the best target for splash damage
bool CAimbotProjectile::GetSplashTarget(CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon, CUserCmd* pCmd, Target_t& outTarget)
{
	for (const auto& pTarget : g_EntityCache.GetGroup(EGroupType::PLAYERS_ENEMIES))
	{


		if (!pTarget || !pTarget->IsAlive()) { continue; }

		if (F::AimbotGlobal.ShouldIgnore(pTarget))
		{
			continue;
		}
		bool cansplash = pLocal->IsClass(CLASS_SOLDIER) && pWeapon->GetSlot() == SLOT_PRIMARY && G::CurItemDefIndex != Soldier_m_TheDirectHit;

		if (!cansplash)
		{
			return false;
		}


		const Vector center = pTarget->GetAbsOrigin() + (pTarget->m_vecMins() + pTarget->m_vecMaxs()) * 0.5f;

		auto sphere = SpherePoints(146, 70);

		std::sort(sphere.begin(), sphere.end(), [&](const Vector& a, const Vector& b) -> bool
			{
				return (center + a).DistTo(center) < (center + b).DistTo(center);
			});

		auto shootpos = pLocal->GetShootPos();

		CGameTrace trace = {};
		CTraceFilterWorldAndPropsOnly filter = {};

		Vector offset = { 23.5f, 12.0f, -3.0f };

		if (pLocal->m_fFlags() & IN_DUCK)
		{
			offset.z = 8.0f;
		}

		for (const auto& point : sphere)
		{
			auto pos = center + point;


			// Get trace endpoint from center to pos
			Utils::Trace(center, pos, MASK_SHOT, &filter, &trace);

			if (!trace.DidHit())
			{
				continue;
			}

			Vector position = trace.vEndPos;

			auto angle = Math::CalcAngle(shootpos, position);

			Vector vischeck = shootpos;

			Utils::GetProjectileFireSetup(pLocal, angle, offset, &vischeck);

			Utils::Trace(vischeck, position, MASK_SHOT, &filter, &trace);

			const Vec3 vLocalAngles = I::EngineClient->GetViewAngles();
			const ESortMethod sortMethod = GetSortMethod();
			// Players

			Vec3 vAngleTo = Math::CalcAngle(vischeck, position);
			const float flFOVTo = Math::CalcFov(vLocalAngles, vAngleTo);


			if ((sortMethod == ESortMethod::FOV || Vars::Aimbot::Projectile::RespectFOV.Value) && flFOVTo > Vars::Aimbot::Global::AimFOV.Value)
			{
				continue;
			}

			if (!trace.DidHit() && Utils::VisPos(pLocal, pTarget, vischeck, position))
			{
				outTarget = { pTarget, ETargetType::PLAYER, position, vAngleTo };
				return true;
			}
		}
	}
	return false;
}


// Returns the best target for splash damage
bool CAimbotProjectile::GetSplashTargeR(CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon, CUserCmd* pCmd, Target_t& outTarget)
{
	for (const auto& pTarget : g_EntityCache.GetGroup(EGroupType::BUILDINGS_ENEMIES))
	{

		if (!pTarget || !pTarget->IsAlive()) { continue; }


		bool cansplash = pLocal->IsClass(CLASS_SOLDIER) && pWeapon->GetSlot() == SLOT_PRIMARY && G::CurItemDefIndex != Soldier_m_TheDirectHit;

		if (!cansplash)
		{
			return false;
		}

		const Vector center = pTarget->GetWorldSpaceCenter();

		auto sphere = SpherePoints(146, 160);

		std::sort(sphere.begin(), sphere.end(), [&](const Vector& a, const Vector& b) -> bool
			{
				return (center + a).DistTo(center) < (center + b).DistTo(center);
			});

		auto shootpos = pLocal->GetShootPos();

		CGameTrace trace = {};
		CTraceFilterWorldAndPropsOnly filter = {};

		Vector offset = { 23.5f, 12.0f, -3.0f };

		if (pLocal->m_fFlags() & IN_DUCK)
		{
			offset.z = 8.0f;
		}

		for (const auto& point : sphere)
		{
			auto pos = center + point;

			// Get trace endpoint from center to pos
			Utils::Trace(center, pos, MASK_SHOT, &filter, &trace);

			if (!trace.DidHit())
			{
				continue;
			}

			Vector position = trace.vEndPos;

			auto angle = Math::CalcAngle(shootpos, position);

			Vector vischeck = shootpos;

			Utils::GetProjectileFireSetup(pLocal, angle, offset, &vischeck);

			Utils::Trace(vischeck, position, MASK_SHOT, &filter, &trace);

			const Vec3 vLocalAngles = I::EngineClient->GetViewAngles();
			const ESortMethod sortMethod = GetSortMethod();
			// Players

			Vec3 vAngleTo = Math::CalcAngle(vischeck, position);
			const float flFOVTo = Math::CalcFov(vLocalAngles, vAngleTo);


			if ((sortMethod == ESortMethod::FOV || Vars::Aimbot::Projectile::RespectFOV.Value) && flFOVTo > Vars::Aimbot::Global::AimFOV.Value)
			{
				continue;
			}

			if (!trace.DidHit() && Utils::VisPos(pLocal, pTarget, vischeck, position))
			{
				outTarget = { pTarget, ETargetType::BUILDING, position, vAngleTo };
				return true;
			}
		}
	}
	return false;
}

void CAimbotProjectile::Run(CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon, CUserCmd* pCmd)
{
	static int nLastTracerTick = pCmd->tick_count;

	IsFlameThrower = false;

	if (!Vars::Aimbot::Global::Active.Value)
	{
		return;
	}

	const bool bShouldAim = (Vars::Aimbot::Global::AimKey.Value == VK_LBUTTON
		? (pCmd->buttons & IN_ATTACK)
		: F::AimbotGlobal.IsKeyDown());
	if (!bShouldAim) { return; }

	Target_t target{ };
	if (GetTarget(pLocal, pWeapon, pCmd, target) || GetSplashTarget(pLocal, pWeapon, pCmd, target) || GetSplashTargeR(pLocal, pWeapon, pCmd, target))
	{
		// Aim at the current target or splashtarget
		G::CurrentTargetIdx = target.m_pEntity->GetIndex();

		if (Vars::Aimbot::Projectile::AimMethod.Value == 1)
		{
			G::AimPos = G::PredictedPos;
		}

		if (ShouldFire(pCmd))
		{
			pCmd->buttons |= IN_ATTACK;

			if (G::CurItemDefIndex == Soldier_m_TheBeggarsBazooka)
			{
				if (pWeapon->GetClip1() > 0)
					pCmd->buttons &= ~IN_ATTACK;
			}
			else
			{
				if ((pWeapon->GetWeaponID() == TF_WEAPON_COMPOUND_BOW || pWeapon->GetWeaponID() == TF_WEAPON_PIPEBOMBLAUNCHER)
					&& pWeapon->GetChargeBeginTime() > 0.0f)
				{
					pCmd->buttons &= ~IN_ATTACK;
				}
				else if (pWeapon->GetWeaponID() == TF_WEAPON_CANNON && pWeapon->GetDetonateTime() > 0.0f)
				{
					const Vec3 vEyePos = pLocal->GetShootPos();
					float BestCharge = vEyePos.DistTo(G::PredictedPos) / 1453.9f;

					if (Vars::Aimbot::Projectile::ChargeLooseCannon.Value)
					{
						if (pWeapon->GetDetonateTime() - I::GlobalVars->curtime <= BestCharge)
							pCmd->buttons &= ~IN_ATTACK;
					}
					else
						pCmd->buttons &= ~IN_ATTACK;
				}
			}
		}

		const bool bIsAttacking = IsAttacking(pCmd, pWeapon);

		if (bIsAttacking)
		{
			G::IsAttacking = true;
			if (Vars::Visuals::BulletTracer.Value && abs(pCmd->tick_count - nLastTracerTick) > 1)
			{
				nLastTracerTick = pCmd->tick_count;
			}

			if (I::EngineClient->Time() > 4)
			{
				G::PredLinesBackup.clear();
			}

			G::PredLinesBackup.clear();
			G::PredLinesBackup = G::PredictionLines;


			//I::DebugOverlay->AddLineOverlayAlpha(Target.m_vPos, G::m_vPredictedPos, 0, 255, 0, 255, true, 2); // Predicted aim pos
		}

		if (Vars::Aimbot::Projectile::AimMethod.Value == 1)
		{
			if (IsFlameThrower)
			{
				G::ProjectileSilentActive = true;
				Aim(pCmd, pWeapon, target.m_vAngleTo);
			}

			else
			{
				if (bIsAttacking)
				{
					Aim(pCmd, pWeapon, target.m_vAngleTo);
					G::SilentTime = true;
				}
			}
		}
		else
		{
			Aim(pCmd, pWeapon, target.m_vAngleTo);
		}
	}
}