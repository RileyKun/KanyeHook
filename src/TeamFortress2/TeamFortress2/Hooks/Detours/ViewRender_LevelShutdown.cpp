#include "../Hooks.h"

#include "../../Features/Visuals/Visuals.h"
#include "../../Features/seedprediction/seed.hpp"
#include "../../Features/CritHack/CritHack.h"

MAKE_HOOK(ViewRender_LevelShutdown, Utils::GetVFuncPtr(I::ViewRender, 2), void, __fastcall,
		  void* ecx, void* edx)
{
	
	Hook.Original<FN>()(ecx, edx);
	F::NS.reset();
	G::commands.clear();    I::Cvar->ConsoleColorPrintf({ 255,255,255,255 }, "[FL] Reset \n");
}