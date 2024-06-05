#include "../Hooks.h"

#include "../../Features/Visuals/Visuals.h"

MAKE_HOOK(Panel_PaintTraverse, Utils::GetVFuncPtr(I::VGuiPanel, 41), void, __fastcall,
		  void* ecx, void* edx, unsigned int vgui_panel, bool force_repaint, bool allow_force)
{

	Hook.Original<FN>()(ecx, edx, vgui_panel, force_repaint, allow_force);
}

