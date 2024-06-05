#include "../Hooks.h"
#include "../../Features/Aimbot/Aimbot.h"
#include "../../Features/seedprediction/seed.hpp"

__inline void s(const char* cFunction, const char* cLog, Color_t cColour) {
	I::Cvar->ConsoleColorPrintf(cColour, "[%s] ", cFunction);
	I::Cvar->ConsoleColorPrintf({ 255, 255, 255, 255 }, "%s\n", cLog);
}


inline CUserCmd* GetCmds()
{
	return *reinterpret_cast<CUserCmd**>(reinterpret_cast<uintptr_t>(I::Input) + 0xFC);
}

MAKE_HOOK(Input_GetUserCmd, Utils::GetVFuncPtr(I::Input, 8), CUserCmd*, __fastcall,
	void* ecx, void* edx, int sequence_number)
{
	return &GetCmds()[sequence_number % 90]; //90
}

MAKE_HOOK(FX_FireBullets, g_Pattern.Find(L"client.dll", L"55 8B EC 81 EC ? ? ? ? 53 8B 5D ? 56 53"), void, __cdecl,
	void* pWpn, int iPlayer, const Vec3& vecOrigin, const Vec3& vecAngles, int iWeapon, int iMode, int iSeed, float flSpread, float flDamage, bool bCritical)
{

	iSeed = F::NS.getSeed();
	Hook.Original<FN>()(pWpn, iPlayer, vecOrigin, vecAngles, iWeapon, iMode, iSeed, flSpread, flDamage, bCritical);
}



MAKE_HOOK(PacketStart, g_Pattern.Find(L"engine.dll", L"55 8B EC 8B 45 ? 89 81 ? ? ? ? 8B 45 ? 89 81 ? ? ? ? 5D C2 ? ? CC CC CC CC CC CC CC 55 8B EC 56"),
	void, __fastcall, void* ECX, void* EDX, int incoming, int outgoing)
{
	if (!Vars::Misc::CL_Move::Fakelag.Value)
		return Hook.Original<FN>()(ECX, EDX, incoming, outgoing);

	if (!g_EntityCache.GetLocal())
		return Hook.Original<FN>()(ECX, EDX, incoming, outgoing);

	if (!g_EntityCache.GetLocal()->IsAlive())
		return Hook.Original<FN>()(ECX, EDX, incoming, outgoing);

	if (G::commands.empty())
		return Hook.Original<FN>()(ECX, EDX, incoming, outgoing);


	for (auto it = G::commands.rbegin(); it != G::commands.rend(); ++it)
	{
		if (!it->is_outgoing)
			continue;

		if (it->command_number == outgoing || outgoing > it->command_number && (!it->is_used || it->previous_command_number == outgoing))
		{
			it->previous_command_number = outgoing;
			it->is_used = true;
			Hook.Original<FN>()(ECX, EDX, incoming, outgoing);
			break;
		}
	}

	auto result = false;

	for (auto it = G::commands.begin(); it != G::commands.end();)
	{
		if (outgoing == it->command_number || outgoing == it->previous_command_number)
			result = true;

		if (outgoing > it->command_number && outgoing > it->previous_command_number)
			it = G::commands.erase(it);
		else
			++it;
	}

	if (!result)
	Hook.Original<FN>()(ECX, EDX, incoming, outgoing);
}


static IPredictionSystem* g_pPredictionSystems;
MAKE_HOOK(CPrediction_RunCommand, Utils::GetVFuncPtr(I::Prediction, 17), void, __fastcall,
	void* ecx, void* edx, CBaseEntity* pPlayer, CUserCmd* pCmd, CMoveHelper* moveHelper)
{


	I::MoveHelper = moveHelper;


	Hook.Original<FN>()(ecx, edx, pPlayer, pCmd, moveHelper);

	if (pPlayer == g_EntityCache.GetLocal() && !pPlayer->IsInBumperKart())
	{
		if (const auto& pAnimState = pPlayer->GetAnimState())
		{
			if (G::Choking)
				return;

			for (auto& vAngle : G::ChokedAngles)
			{
				pPlayer->m_effects() |= EF_NOINTERP;
				pAnimState->Update(vAngle.y, G::RealViewAngles.x);
				pAnimState->m_flGoalFeetYaw = vAngle.y;
				pPlayer->FrameAdvance(I::GlobalVars->frametime);
				pPlayer->m_effects() &= ~EF_NOINTERP;
				G::ChokedAngles.clear();
			}
		}
	}
}
