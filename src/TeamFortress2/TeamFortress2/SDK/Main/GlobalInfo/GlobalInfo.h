#pragma once
#include "../BaseEntity/BaseEntity.h"

constexpr auto DT_WAIT_CALLS = 26;

struct VelFixRecord {
	Vec3 m_vecOrigin;
	int m_nFlags;
	float m_flSimulationTime;
};

struct PlayerCache {
	Vec3 m_vecOrigin;
	Vec3 m_vecVelocity;
	Vec3 eyePosition;
	int playersPredictedTick;
	bool didDamage = false;
};

struct DormantData {
	Vec3 Location;
	float LastUpdate = 0.f;
};

struct Priority {
	int Mode = 2; // 0 - Friend, 1 - Ignore, 2 - Default, 3 - Rage, 4 - Cheater
};

struct Command
{
	bool is_outgoing = false;
	bool is_used = false;
	int previous_command_number = 0;
	int command_number = 0;
};

namespace G
{
	inline bool faggot = false;
	inline int Back = 0;
	inline int New = 0;
	inline Vector FireBulletAngle = {};
	inline float FireBulletSpread = 0;
	inline float FireBulletSeed = 0;
	inline std::deque <Command> commands;
	inline bool Choking = false;
	inline std::vector<Vec3> ChokedAngles;
	inline std::vector<Vec3> FakeAngles;
	inline float CorrectDamage = 0;
	inline bool lol = false;
	inline Vector VisCheck = {};
	inline bool CritHack = false;
	inline bool Landed = false;
	inline bool djump = false;
	inline bool DT = false;
	inline bool LC = false;
	inline std::vector<Vector> projectile_lines;
	inline bool UpdateAnim = false;
	inline int CurrentTargetIdx = 0; // Index of the current aimbot target
	inline int CurItemDefIndex = 0; // DefIndex of the current weapon
	inline int NotifyCounter = 0;
	inline int EyeAngDelay = 25;
	inline int LoadInCount = 0; //	increments each time we change server / map.
	inline int NextSafeTick = 0;	//	I::GlobalVars->tickcount + sv_maxusrcmdprocessticks_holdaim + 1 (when attacking)
	inline float LerpTime = 0.f;	//	current lerp time
	inline bool WeaponCanHeadShot = false; // Can the current weapon headshot?
	inline bool WeaponCanAttack = false; // Can the current weapon attack?
	inline bool WeaponCanSecondaryAttack = false;
	inline bool AAActive = false; // Is the Anti-Aim active?
	inline bool FakeShotPitch = false;
	inline bool HitscanSilentActive = false;
	inline bool AvoidingBackstab = false; // Are we currently avoiding a backstab? (Overwrites AA)
	inline bool ProjectileSilentActive = false; //flamethrower
	inline bool AutoBackstabRunning = false;
	inline bool LocalSpectated = false; // Is the local player being spectated?
	inline bool RollExploiting = false; // Are we performing the roll exploit?
	inline bool ShouldStop = false; // Stops our players movement, takes 1 tick.
	inline bool UnloadWndProcHook = false;
	inline bool Frozen = false;	//	angles & movement are frozen.

	/* Double tap / Tick shift */
	inline int WaitForShift = 0;
	inline int ShiftedTicks = 0; // Amount of ticks that are shifted
	inline bool ShouldShift = false; // Should we shift now?
	inline bool Recharging = false; // Are we currently recharging?
	inline bool RechargeQueued = false; // Queues a recharge
	inline int TickShiftQueue = 0; // Ticks that shouls be shifted

	/* Choking / Packets */
	inline bool ForceSendPacket = false; // might not actually be useful 
	inline bool ForceChokePacket = false; // might not actually be useful 
	inline bool IsChoking = false; // might not actually be useful 

	/* Aimbot */
	inline bool IsAttacking = false; // this is only used by aimbot, and is also set to false at the start of a lot of functions, this is not reliable
	inline bool HitscanRunning = false;
	inline bool SilentTime = false;
	inline Vec3 AimPos = {};
	inline Vec3 PredAim = {};
	inline VMatrix WorldToProjection = {};

	/* Angles */
	inline Vec3 ViewAngles = {};
	inline Vec3 RealViewAngles = {}; // Real view angles (AA)
	inline Vec3 FakeViewAngles = {}; // Fake view angles (AA)
	inline Vec3 PunchAngles = {};

	/* Projectile prediction */
	inline Vec3 PredictedPos = {};
	inline Vec3 LinearPredLine = {};
	inline std::vector<Vec3> PredictionLines;
	inline std::vector<Vec3> PredLinesBackup;

	inline CUserCmd* CurrentUserCmd{nullptr}; // Unreliable! Only use this if you really have to.
	inline CUserCmd* LastUserCmd{nullptr};
	
	inline EWeaponType CurWeaponType = {};
	inline std::unordered_map<CBaseEntity*, VelFixRecord> VelFixRecords;
	inline std::unordered_map<CBaseEntity*, std::unordered_map<int, PlayerCache>> Cache; // caches movement, angles, add more if you want. format is <Entity, <tickcount, pData>>
	inline bool FreecamActive = false;
	inline Vec3 FreecamPos = {};
	inline std::unordered_map<int, DormantData> PartyPlayerESP; // <Index, DormantData>
	inline std::unordered_map<int, int> ChokeMap; // Choked packets of players <Index, Amount>
	inline bool DrawingStaticProps = false;
	inline std::unordered_map<uint32_t, Priority> PlayerPriority; // Playerlist priorities <FriendsID, Priority>

	inline DWORD CalcIsAttackCriticalHelperOffset = 0;

	inline bool ShouldAutoQueue = false;

	inline int BackpackQuality = 1;

	inline std::vector<int> MedicCallers;

	inline bool ShouldUpdateMaterialCache = false;

	inline bool IsIgnored(uint32_t friendsID)
	{
		return PlayerPriority[friendsID].Mode < 2;
	}

	inline bool IsRage(uint32_t friendsID)
	{
		return PlayerPriority[friendsID].Mode > 2;
	}
};
