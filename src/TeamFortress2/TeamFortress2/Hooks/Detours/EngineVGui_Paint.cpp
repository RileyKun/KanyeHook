#include "../Hooks.h"

#include "../../SDK/Includes/icons.h"

#include "../../Features/ESP/ESP.h"
#include "../../Features/Aimbot/AimbotHitscan/AimbotHitscan.h"
#include "../../Features/Misc/Misc.h"
#include "../../Features/Visuals/Visuals.h"
#include "../../Features/CritHack/CritHack.h"
#include "../../Features/Menu/Menu.h"
#include "../../Features/Menu/SpectatorList/SpectatorList.h"
#include "../../Features/Menu/Playerlist/Playerlist.h"
#include "../../Features/CritHack/CritHack.h"
#include "../../Features/Aimbot/ProjectileAim/ProjSim.h"

MAKE_HOOK(EngineVGui_Paint, Utils::GetVFuncPtr(I::EngineVGui, 14), void, __fastcall,
	void* ecx, void* edx, int iMode)
{
	static auto StartDrawing = reinterpret_cast<void(__thiscall*)(void*)>(g_Pattern.Find(
		_(L"vguimatsurface.dll"), _(L"55 8B EC 64 A1 ? ? ? ? 6A FF 68 ? ? ? ? 50 64 89 25 ? ? ? ? 83 EC 14")));
	static auto FinishDrawing = reinterpret_cast<void(__thiscall*)(void*)>(g_Pattern.Find(
		_(L"vguimatsurface.dll"), _(L"55 8B EC 6A FF 68 ? ? ? ? 64 A1 ? ? ? ? 50 64 89 25 ? ? ? ? 51 56 6A 00")));

	if (!g_ScreenSize.w || !g_ScreenSize.h)
	{
		g_ScreenSize.Update();
	}

	//HACK: for some reason we need to do this
	{
		static bool bInitIcons = false;

		if (!bInitIcons)
		{
			for (int nIndex = 0; nIndex < ICONS::TEXTURE_AMOUNT; nIndex++)
			{
				ICONS::ID[nIndex] = -1;
				g_Draw.Texture(-200, 0, 18, 18, Colors::White, nIndex);
			}

			bInitIcons = true;
		}
	}

	Hook.Original<FN>()(ecx, edx, iMode);

	if (iMode & PAINT_UIPANELS)
	{
		//Update W2S
		{
			CViewSetup viewSetup = {};

			if (I::BaseClientDLL->GetPlayerView(viewSetup))
			{
				VMatrix worldToView = {}, viewToProjection = {}, worldToPixels = {};
				I::RenderView->GetMatricesForView(viewSetup, &worldToView, &viewToProjection,
					&G::WorldToProjection, &worldToPixels);
			}
		}

		StartDrawing(I::VGuiSurface);
		{
			if (g_Draw.m_vecFonts.empty())
			{
				g_Draw.RemakeFonts
				({
					{ 0x0, "calibri", 22, 0, FONTFLAG_OUTLINE | FONTFLAG_ANTIALIAS}, //name esp
					{ 0x0, "calibri", 22, 0, FONTFLAG_OUTLINE | FONTFLAG_ANTIALIAS}, //name esp
				{ 0x0, "calibri", 22, 0, FONTFLAG_OUTLINE | FONTFLAG_ANTIALIAS}, //name esp
				{ 0x0, "calibri", 22, 0, FONTFLAG_OUTLINE | FONTFLAG_ANTIALIAS}, //name esp
					{ 0x0, "Verdana", 12, 800, FONTFLAG_OUTLINE}, //menu esp
					{ 0x0, "Verdana", 12, 800, FONTFLAG_OUTLINE}, //indicators esp
					{ 0x0, "Verdana", 12, 800, FONTFLAG_OUTLINE}, //idk yet
					{ 0x0, "Verdana", 12, 800, FONTFLAG_OUTLINE}, //idk yet x2
					});
			}

			if (I::EngineVGui->IsGameUIVisible())
			{
				if (!I::EngineClient->IsInGame())
				{
					g_Draw.String(FONT_MENU, 5, g_ScreenSize.h - 5 - Vars::Fonts::FONT_MENU::nTall.Value, { 255, 255, 255, 255 }, ALIGN_DEFAULT, "Build: " __DATE__);
				}
			}

			if (CBaseEntity* pLocal = g_EntityCache.GetLocal())
			{
			

				F::Visuals.DrawTickbaseInfo(pLocal);
				F::Visuals.DrawAimbotFOV(pLocal);
				const auto fontHeight = Vars::Fonts::FONT_INDICATORS::nTall.Value;
				const int drawY = g_ScreenSize.h / static_cast<double>(2) + (g_ScreenSize.h * 0.25);
			}
				
			
			F::CritHack.Draw();
			F::ESP.Run();
			F::SpectatorList.Run();
			F::PlayerList.Run();
		}
		FinishDrawing(I::VGuiSurface);
	}
}