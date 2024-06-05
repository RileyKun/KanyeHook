#include "../Hooks.h"

MAKE_HOOK(BaseClientDLL_PostEntity, Utils::GetVFuncPtr(I::BaseClientDLL, 6), void, __fastcall,
		  void* ecx, void* edx)
{
	Hook.Original<FN>()(ecx, edx);
	
}