#include "../Hooks.h"

#include "../../Features/Visuals/Visuals.h"

MAKE_HOOK(ClientModeShared_OverrideView, Utils::GetVFuncPtr(I::ClientModeShared, 24), bool, __fastcall, void* ecx, void* edx)
{
	if (const auto& pLocal = g_EntityCache.GetLocal())
	{
		if (pLocal->IsScoped() && Vars::Visuals::RemoveZoom.Value)
		{
			return true;
		}
	}

	return Hook.Original<FN>()(ecx, edx);
}