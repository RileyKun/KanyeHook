#include "../Hooks.h"
#include "../../Features/Vars.h"

MAKE_HOOK(ClientState_GetClientInterpAmount, g_Pattern.Find(L"engine.dll", L"55 8B EC 8B 0D ? ? ? ? 83 EC 10 85 C9 75 26 8B 0D ? ? ? ? 68 ? ? ? ? 8B 01 FF 50 34"), float, __fastcall,
	void* ecx, void* edx)
{
	return I::Cvar->FindVar(_("cl_interp"))->GetFloat();
}
