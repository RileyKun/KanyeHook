#include "../Hooks.h"

MAKE_HOOK(ProcessMovement, g_Pattern.Find(L"client.dll", L"55 8B EC 56 57 8B 7D 08 8B F1 85 FF 74 6F 53 8B 5D 0C 85 DB"), void, __fastcall, void* ecx, void* edx, CBaseEntity* pBasePlayer, CMoveData* pMove)
{
	I::CTFGameMovement = ecx;

	Hook.Original<FN>()(ecx, edx, pBasePlayer, pMove);
}