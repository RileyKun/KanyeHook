#include "movementsim.h"
#include "../../Backtrack/Backtrack.h"

//we'll use this to set current player's command, without it CGameMovement::CheckInterval will try to access a nullptr
static CUserCmd DummyCmd = {};

//since we're going to call game functions some entity data will be modified (we'll modify it too), we'll have to restore it after running
class CPlayerDataBackup
{
public:
	Vec3 m_vecOrigin = {};
	Vec3 m_vecVelocity = {};
	Vec3 m_vecBaseVelocity = {};
	Vec3 m_vecViewOffset = {};
	Vec3 m_vecWorldSpaceCenter = {};
	int m_hGroundEntity = 0;
	int m_fFlags = 0;
	float m_flDucktime = 0.0f;
	float m_flDuckJumpTime = 0.0f;
	bool m_bDucked = false;
	bool m_bDucking = false;
	bool m_bInDuckJump = false;
	float m_flModelScale = 0.0f;

public:
	void Store(CBaseEntity* pPlayer)
	{
		m_vecOrigin = pPlayer->m_vecOrigin();
		m_vecVelocity = pPlayer->m_vecVelocity();
		m_vecBaseVelocity = pPlayer->m_vecBaseVelocity();
		m_vecViewOffset = pPlayer->m_vecViewOffset();
		m_hGroundEntity = pPlayer->m_hGroundEntity();
		m_vecWorldSpaceCenter = pPlayer->GetWorldSpaceCenter();
		m_fFlags = pPlayer->m_fFlags();
		m_flDucktime = pPlayer->m_flDucktime();
		m_flDuckJumpTime = pPlayer->m_flDuckJumpTime();
		m_bDucked = pPlayer->m_bDucked();
		m_bDucking = pPlayer->m_bDucking();
		m_bInDuckJump = pPlayer->m_bInDuckJump();
		m_flModelScale = pPlayer->m_flModelScale();
	}

	void Restore(CBaseEntity* pPlayer)
	{
		pPlayer->m_vecOrigin() = m_vecOrigin;
		pPlayer->m_vecVelocity() = m_vecVelocity;
		pPlayer->m_vecBaseVelocity() = m_vecBaseVelocity;
		pPlayer->m_vecViewOffset() = m_vecViewOffset;
		pPlayer->m_hGroundEntity() = m_hGroundEntity;
		pPlayer->m_fFlags() = m_fFlags;
		pPlayer->m_flDucktime() = m_flDucktime;
		pPlayer->m_flDuckJumpTime() = m_flDuckJumpTime;
		pPlayer->m_bDucked() = m_bDucked;
		pPlayer->m_bDucking() = m_bDucking;
		pPlayer->m_bInDuckJump() = m_bInDuckJump;
		pPlayer->m_flModelScale() = m_flModelScale;
	}
};

static CPlayerDataBackup PlayerDataBackup = {};

void CMovementSimulation::SetupMoveData(CBaseEntity* pPlayer, CMoveData* pMoveData)
{
	if (!pPlayer || !pMoveData)
	{
		return;
	}

	pMoveData->m_bFirstRunOfFunctions = false;
	pMoveData->m_bGameCodeMovedPlayer = false;
	pMoveData->m_nPlayerHandle = reinterpret_cast<IHandleEntity*>(pPlayer)->GetRefEHandle();
	pMoveData->m_vecVelocity = pPlayer->m_vecVelocity();	//	m_vecBaseVelocity hits -1950?
	pMoveData->m_vecAbsOrigin = pPlayer->m_vecOrigin();
	pMoveData->m_flMaxSpeed = pPlayer->TeamFortress_CalculateMaxSpeed();

	if (PlayerDataBackup.m_fFlags & FL_DUCKING)
	{
		pMoveData->m_flMaxSpeed *= 0.3333f;
	}

	pMoveData->m_flClientMaxSpeed = pMoveData->m_flMaxSpeed;

	//need a better way to determine angles probably
	pMoveData->m_vecViewAngles = { 0.0f, Math::VelocityToAngles(pMoveData->m_vecVelocity).y, 0.0f };

	Vec3 vForward = {}, vRight = {};
	Math::AngleVectors(pMoveData->m_vecViewAngles, &vForward, &vRight, nullptr);

	pMoveData->m_flForwardMove = (pMoveData->m_vecVelocity.y - (vRight.y / vRight.x) * pMoveData->m_vecVelocity.x) / (vForward.y - (vRight.y / vRight.x) * vForward.x);
	pMoveData->m_flSideMove = (pMoveData->m_vecVelocity.x - vForward.x * pMoveData->m_flForwardMove) / vRight.x;

}

bool CMovementSimulation::Initialize(CBaseEntity* pPlayer)
{
	

	if (!pPlayer || pPlayer->deadflag())
	{
		return false;
	}
	//set player
	m_pPlayer = pPlayer;

	//set current command
	m_pPlayer->SetCurrentCmd(&DummyCmd);
	//m_pPlayer->SetCurrentCommand(&DummyCmd);

	//store player's data
	PlayerDataBackup.Store(m_pPlayer);

	//store vars
	m_bOldInPrediction = I::Prediction->m_bInPrediction;
	m_bOldFirstTimePredicted = I::Prediction->m_bFirstTimePredicted;
	m_flOldFrametime = I::GlobalVars->frametime;

	//the hacks that make it work
	{
		if (pPlayer->m_fFlags() & FL_DUCKING)
		{
			pPlayer->m_fFlags() &= ~FL_DUCKING; //breaks origin's z if FL_DUCKING is not removed
			pPlayer->m_bDucked() = true; //(mins/maxs will be fine when ducking as long as m_bDucked is true)
			pPlayer->m_flDucktime() = 0.0f;
			pPlayer->m_flDuckJumpTime() = 0.0f;
			pPlayer->m_bDucking() = false;
			pPlayer->m_bInDuckJump() = false;
		}


		if (pPlayer->m_fFlags() & FL_ONGROUND)
		{
			pPlayer->m_vecOrigin().z += 0.03125f; //to prevent getting stuck in the ground
		}

		//for some reason if xy vel is zero it doesn't predict
		if (fabsf(pPlayer->m_vecVelocity().x) < 0.01f)
		{
			pPlayer->m_vecVelocity().x = 0.015f;
		}

		if (fabsf(pPlayer->m_vecVelocity().y) < 0.01f)
		{
			pPlayer->m_vecVelocity().y = 0.015f;
		}
	}


	SetupMoveData(m_pPlayer, &m_MoveData);
	bFirstRunTick = true;

	return true;

}


void CMovementSimulation::FillVelocities()
{
	for (const auto& pEntity : g_EntityCache.GetGroup(EGroupType::PLAYERS_ENEMIES))
	{
		const int iEntIndex = pEntity->GetIndex();
		if (!pEntity->IsAlive() || pEntity->GetDormant())
		{
			m_Velocities[iEntIndex].clear();
			continue;
		}

		m_Velocities[iEntIndex].push_front(pEntity->m_vecVelocity());
	}
}

void CMovementSimulation::Restore()
{
	if (!m_pPlayer)
	{
		return;
	}

	m_pPlayer->SetCurrentCmd(nullptr);

	PlayerDataBackup.Restore(m_pPlayer);

	I::Prediction->m_bInPrediction = m_bOldInPrediction;
	I::Prediction->m_bFirstTimePredicted = m_bOldFirstTimePredicted;
	I::GlobalVars->frametime = m_flOldFrametime;

	m_pPlayer = nullptr;

	memset(&m_MoveData, 0, sizeof(CMoveData));
	memset(&PlayerDataBackup, 0, sizeof(CPlayerDataBackup));
}

__inline void cs(const char* cFunction, const char* cLog, Color_t cColour) {
	I::Cvar->ConsoleColorPrintf(cColour, "[%s] ", cFunction);
	I::Cvar->ConsoleColorPrintf({ 255, 255, 255, 255 }, "%s\n", cLog);
}

bool CMovementSimulation::BetaStrafePred()
{
	if (!Vars::Aimbot::Hitscan::ExtinguishTeam.Value || !m_pPlayer->IsAlive()) { return false; }
	
	static float flAverageYaw = 0.f;
	static float flInitialYaw = 0.f;
	static float flRecordYaw = 0.f;

	if (bFirstRunTick)
	{
		flAverageYaw = 0.f;
		flInitialYaw = 0.f;
		flRecordYaw = 0.f;
		bFirstRunTick = false;

		const int iEntIndex = m_pPlayer->GetIndex();
		const auto& mVelocityRecord = m_Velocities[iEntIndex];

		const int iSamples = fmin(1, mVelocityRecord.size());
		if (!iSamples) { return false; }

		flInitialYaw = m_MoveData.m_vecViewAngles.y;
		float flCompareYaw = flInitialYaw;

		for (int i = 0; i <= iSamples; i++)
		{
			flRecordYaw = Math::VelocityToAngles(mVelocityRecord.at(i)).y;
		}

		const float flDelta = RAD2DEG(Math::AngleDiffRad(DEG2RAD(flCompareYaw), DEG2RAD(flRecordYaw)));
		flAverageYaw += flDelta;

		if (!m_pPlayer->OnSolid()) //true check for in air. fuck you if you change this -gucci 18/02/24
		{
			m_MoveData.m_flSideMove = flAverageYaw < 0.0f ? 450.0f : -450.0f;
			m_MoveData.m_flForwardMove = 0.0f; //no forward your auto strafe is lying.
		}
	}

	if (flAverageYaw == 0.f) { return false; }	//	fix

	flInitialYaw += flAverageYaw;
	m_MoveData.m_vecViewAngles.y = flInitialYaw;

	return true;
}

void CMovementSimulation::RunTick(CMoveData& moveDataOut, Vec3& m_vecAbsOrigin)
{
	if (!m_pPlayer)
	{
		return;
	}

	if (m_pPlayer->GetClassID() != ETFClassID::CTFPlayer)
	{
		return;
	}

	//make sure frametime and prediction vars are right
	I::Prediction->m_bInPrediction = true;
	I::Prediction->m_bFirstTimePredicted = false;
	I::GlobalVars->frametime = I::Prediction->m_bEnginePaused ? 0.0f : TICK_INTERVAL;

	if (!BetaStrafePred())
	{
		m_MoveData.m_vecViewAngles = { 0.0f, Math::VelocityToAngles(m_MoveData.m_vecVelocity).y, 0.0f };
	}
	
	//call CTFGameMovement::ProcessMovement
	using ProcessMovement_FN = void(__thiscall*)(void*, CBaseEntity*, CMoveData*);
	reinterpret_cast<ProcessMovement_FN>(Utils::GetVFuncPtr(I::CTFGameMovement, 1))(I::CTFGameMovement, m_pPlayer, &m_MoveData); //CheckForDoubleTap(m_pPlayer, &moveDataOut);


	G::PredictionLines.push_back(m_MoveData.m_vecAbsOrigin);

	moveDataOut = m_MoveData;
	m_vecAbsOrigin = m_MoveData.m_vecAbsOrigin;
}



