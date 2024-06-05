#include "../Hooks.h"
#include "../../Features/Vars.h"

MAKE_HOOK(ClientState_ProcessFixAngle, g_Pattern.Find(L"engine.dll", L"55 8B EC 8B 45 08 83 EC 08 F3 0F 10 15 ? ? ? ?"), bool, __fastcall,
	void* ecx, void* edx, SVC_FixAngle* msg)
{
	return false;
}