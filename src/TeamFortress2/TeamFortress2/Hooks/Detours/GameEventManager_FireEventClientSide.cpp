#include "../Hooks.h"


#include "../../Features/Visuals/Visuals.h"

MAKE_HOOK(GameEventManager_FireEventClientSide, Utils::GetVFuncPtr(I::GameEventManager, 8), bool, __fastcall,
		  void* ecx, void* edx, CGameEvent* pEvent)
{
	return Hook.Original<FN>()(ecx, edx, pEvent);
}