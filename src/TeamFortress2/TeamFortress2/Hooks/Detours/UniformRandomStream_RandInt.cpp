#include "../Hooks.h"

MAKE_HOOK(UniformRandomStream_RandInt, Utils::GetVFuncPtr(I::UniformRandomStream, 2), int, __fastcall,
		  void* ecx, void* edx, int iMinVal, int iMaxVal)
{
	return Hook.Original<FN>()(ecx, edx, iMinVal, iMaxVal);
}