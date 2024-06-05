#include "lagcomp.h"
#include "../../Hooks/Hooks.h"
#include "../seedprediction/seed.hpp"

    std::unordered_map<void*, std::pair<int, float>> pAnimatingInfo;

	MAKE_HOOK(GetClientInterpAmount, g_Pattern.Find(L"client.dll", L"55 8B EC A1 ? ? ? ? 83 EC 08 56 A8 01 75 22 8B 0D ? ? ? ? 83 C8 01 A3 ? ? ? ? 68 ? ? ? ? 8B 01 FF 50 34 8B F0 89 35 ? ? ? ? EB 06 8B 35 ? ? ? ? 85 F6 74 68 8B 06 8B CE 8B 40 3C FF D0 8B 0D"), float, __cdecl)
	{

		return 0;
	}

	MAKE_HOOK(animations, g_Pattern.Find(L"client.dll", L"55 8B EC 81 EC ? ? ? ? 53 57 8B F9 8B 9F ? ? ? ? 89 5D E0 85 DB 0F 84"), void, __fastcall, void* ecx, void* edx, float eyeyaw, float eyepitch)
	{

		if (ecx == g_EntityCache.GetLocal()) {
			if (G::Choking) {
				G::Choking = true;
				return Hook.Original<FN>()(ecx, edx, eyeyaw, eyepitch);
			}
			return; // we update our animstate ourselves, stop game from doing it
		}

		return  Hook.Original<FN>()(ecx, edx, eyeyaw, eyepitch);
	}

	MAKE_HOOK(CSequenceTransitioner_CheckForSequenceChange, g_Pattern.Find(L"client.dll", L"55 8B EC 53 8B 5D 08 57 8B F9 85 DB 0F 84 ? ? ? ? 83 7F 0C 00 75 05 E8 ? ? ? ? 6B 4F 0C 2C 0F 57 C0 56 8B 37 83 C6 D4 03 F1 F3 0F 10 4E ? 0F 2E C8 9F F6 C4 44 7B 62 8B 45 0C"), void, __fastcall,
		void* ecx, void* edx, CStudioHdr* hdr, int nCurSequence, bool bForceNewSequence, bool bInterpolate)
	{
		return Hook.Original<FN>()(ecx, edx, hdr, nCurSequence, bForceNewSequence, false);
	}

	MAKE_HOOK(C_BaseEntity_BaseInterpolatePart1, g_Pattern.Find(L"client.dll", L"55 8B EC 53 8B 5D 18 56 8B F1 C7 03"), int, __fastcall,
		void* ecx, void* edx, float& currentTime, Vec3& oldOrigin, Vec3& oldAngles, Vec3& oldVel, int& bNoMoreChanges)
	{
		bNoMoreChanges = 1;
		return 0;
	}

	MAKE_HOOK(CInterpolatedVarArrayBase_Interpolate, g_Pattern.Find(L"client.dll", L"55 8B EC 83 EC 1C D9 45 0C 8D 45 FC 56 50 83 EC 08 C7 45 ? ? ? ? ? 8D 45 E4 8B F1 D9 5C 24 04 D9 45 08 D9 1C 24 50 E8 ? ? ? ? 84 C0 0F 84 ? ? ? ? F6 46 2C 01 53 57 74 31 83 7D FC 00 B9 ? ? ? ? D9 45 08 B8 ? ? ? ? 0F 44 C1 8B CE 50 8B 06 83 EC 08 DD 1C 24 FF 50 28 50 68 ? ? ? ? FF 15 ? ? ? ? 83 C4 14 80 7D E4 00 74 60 0F B7 5E 0E 8B 55 F0 0F B7 4E 0C 03"), int, __fastcall,
		void* ecx, void* edx, float currentTime, float interpolation_amount)
	{
		return 0;
	}


	MAKE_HOOK(C_BaseEntity_InterpolateServerEntities, g_Pattern.Find(L"client.dll", L"55 8B EC 83 EC 30 8B 0D ? ? ? ? 53 33 DB 89 5D DC 89 5D E0 8B 41 08 89 5D E4 89 5D E8 85 C0 74 41 68 ? ? ? ? 68 ? ? ? ? 68 ? ? ? ? 68 ? ? ? ? 68 ? ? ? ? 68 ? ? ? ? 53 53"), void, __fastcall,
		void* ecx, void* edx)
	{
		static auto cl_extrapolate = g_ConVars.FindVar("cl_extrapolate");

		if (cl_extrapolate && cl_extrapolate->GetInt())
			cl_extrapolate->SetValue(0);

	   return Hook.Original<FN>()(ecx, edx);
	}


#include "../../Features/Auto/AutoUber/AutoUber.h"
#include "../Aimbot/Aimbot.h"
#include "../Prediction/Prediction.h"
#include "../../Hooks/HookManager.h"

/*	MAKE_HOOK(SetChoked, g_Pattern.Find(L"engine.dll", L"FF 41 ? FF 41"), void, __fastcall, CNetChannel* ECX, void* EDX)
	{
		if (!ECX)
		{
			return Hook.Original<FN>()(ECX, EDX);
		}

		const auto ChokedBackup = ECX->m_nChokedPackets;
		ECX->m_nChokedPackets = 0;
		ECX->SendDatagram(NULL);
		--ECX->m_nOutSequenceNr;
		ECX->m_nChokedPackets = ChokedBackup;

		return Hook.Original<FN>()(ECX, EDX);
	}*/


void CLagComp::run(int iIndex, const Vec3 vAngles, const Vec3 vecOrigin)
{

}








