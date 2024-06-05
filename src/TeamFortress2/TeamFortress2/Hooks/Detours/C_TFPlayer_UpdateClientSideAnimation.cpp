#include "../Hooks.h"

MAKE_HOOK(C_TFPlayer_UpdateClientSideAnimation, getAddr(), void, __fastcall,
	void* ecx, void* edx)
{
	if (!G::UpdateAnim)
		return;

	return Hook.Original<FN>()(ecx, edx);
}