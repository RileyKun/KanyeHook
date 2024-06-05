#include "../Hooks.h"

#include "../../Features/Visuals/Visuals.h"
#include "../../Features/Backtrack/Backtrack.h"
#include "../../Features/seedprediction/seed.hpp"
#include "../../Features/CritHack/CritHack.h"

MAKE_HOOK(ViewRender_LevelInit, Utils::GetVFuncPtr(I::ViewRender, 1), void, __fastcall,
		  void* ecx, void* edx)
{
	Hook.Original<FN>()(ecx, edx);
	F::Backtrack.ResetLatency();
	F::NS.reset();
	G::commands.clear();    I::Cvar->ConsoleColorPrintf({ 255,255,255,255 }, "[FL] Reset \n");
}