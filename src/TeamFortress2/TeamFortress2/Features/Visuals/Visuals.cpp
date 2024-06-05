#include "Visuals.h"
#include "../Vars.h"
#include "../AntiHack/FakeLag/FakeLag.h"
#include "../seedprediction/seed.hpp"



void CVisuals::DrawHitboxMatrix(CBaseEntity* pEntity, Color_t colourface, Color_t colouredge, float time)
{
	
	const model_t* model = pEntity->GetModel();
	const studiohdr_t* hdr = I::ModelInfoClient->GetStudioModel(model);
	const mstudiohitboxset_t* set = hdr->GetHitboxSet(pEntity->GetHitboxSet());

	for (int i{}; i < set->numhitboxes; ++i)
	{
		const mstudiobbox_t* bbox = set->hitbox(i);
		if (!bbox)
		{
			continue;
		}

		/*if (bbox->m_radius <= 0.f) {*/
		matrix3x4 rotMatrix;
		Math::AngleMatrix(bbox->angle, rotMatrix);

		matrix3x4 matrix;
		matrix3x4 boneees[128];
		pEntity->SetupBones(boneees, 128, BONE_USED_BY_ANYTHING, 0);
		Math::ConcatTransforms(boneees[bbox->bone], rotMatrix, matrix);

		Vec3 bboxAngle;
		Math::MatrixAngles(matrix, bboxAngle);

		Vec3 matrixOrigin;
		Math::GetMatrixOrigin(matrix, matrixOrigin);

		I::DebugOverlay->AddBoxOverlay2(matrixOrigin, bbox->bbmin, bbox->bbmax, bboxAngle, colourface, colouredge, time);
	}
}

bool CVisuals::RemoveScope(int nPanel)
{
	if (!Vars::Visuals::RemoveScope.Value) { return false; }

	if (!m_nHudZoom && Hash::IsHudScope(I::VGuiPanel->GetName(nPanel)))
	{
		m_nHudZoom = nPanel;
	}

	return (nPanel == m_nHudZoom);
}

void CVisuals::FOV(CViewSetup* pView)
{
	CBaseEntity* pLocal = g_EntityCache.GetLocal();

	if (pLocal && pView)
	{
		if (pLocal->GetObserverMode() == OBS_MODE_FIRSTPERSON || (pLocal->IsScoped() && !Vars::Visuals::RemoveZoom.Value))
		{
			return;
		}

		pView->fov = static_cast<float>(Vars::Visuals::FieldOfView.Value);

		if (pLocal->IsAlive())
		{
			pLocal->SetFov(Vars::Visuals::FieldOfView.Value);
			pLocal->m_iDefaultFOV() = Vars::Visuals::FieldOfView.Value;
		}
	}
}

void CVisuals::ThirdPerson(CViewSetup* pView)
{
	if (const auto& pLocal = g_EntityCache.GetLocal())
	{

		// Toggle key
		if (Vars::Visuals::ThirdPersonKey.Value)
		{
			if (!I::EngineVGui->IsGameUIVisible() && !I::VGuiSurface->IsCursorVisible())
			{
				static KeyHelper tpKey{ &Vars::Visuals::ThirdPersonKey.Value };
				if (tpKey.Pressed())
				{
					Vars::Visuals::ThirdPerson.Value = !Vars::Visuals::ThirdPerson.Value;
				}
			}
		}

		const bool bIsInThirdPerson = I::Input->CAM_IsThirdPerson();

		if (!Vars::Visuals::ThirdPerson.Value
			|| ((!Vars::Visuals::RemoveScope.Value || !Vars::Visuals::RemoveZoom.Value) && pLocal->IsScoped()))
		{
			if (bIsInThirdPerson)
			{
				pLocal->ForceTauntCam(0);

			}

			return;
		}


		if (!bIsInThirdPerson)
		{
			pLocal->ForceTauntCam(1);
		}
	}
}

void CVisuals::DrawTickbaseInfo(CBaseEntity* pLocal)
{
	//Tickbase info
	if (Vars::Misc::CL_Move::Enabled.Value)
	{
		const auto& pLocal = g_EntityCache.GetLocal();
		const auto& pWeapon = g_EntityCache.GetWeapon();

		if (pWeapon)
		{
			if (pLocal->GetLifeState() == LIFE_ALIVE)
			{
				const int nY = (g_ScreenSize.h / 2) + 20;

				static Color_t color1, color2;

				color1 = { 0,255,255, 255 };
				color2 = { 0,255,255, 255 };

				switch (Vars::Misc::CL_Move::DTBarStyle.Value)
				{
				case 0:
					return;
				case 1:
				{
					const DragBox_t DTBox = Vars::Visuals::DTIndicator;
					const auto fontHeight = Vars::Fonts::FONT_INDICATORS::nTall.Value;
					g_Draw.String(FONT_INDICATORS, DTBox.x + 45, DTBox.y - fontHeight, { 255, 255, 255, 255 }, ALIGN_CENTERHORIZONTAL, "Stored Commands");
					const float ratioCurrent = std::clamp((static_cast<float>(F::FakeLag.ChokeCounter + G::ShiftedTicks) / static_cast<float>(22)), 0.0f, 1.0f);
					static float ratioInterp = 0.0f;
					ratioInterp = g_Draw.EaseIn(ratioInterp, ratioCurrent, 1.0f);
					Math::Clamp(ratioInterp, 0.0f, 1.0f);
				
					g_Draw.OutlinedRect(DTBox.x, DTBox.y, 124, 20, {0,0,0,255}); //	draw the outline
					g_Draw.Rect(DTBox.x + 1, DTBox.y + 1, 124 - 2, 20 - 2, { 28, 29, 38, 255 }); //	draw the background
					g_Draw.GradientRectWH(DTBox.x + 1, DTBox.y + 1, ratioInterp * (124 - 2), 20 - 2, color1, color2, true);
					break;
				}
				}
	
			}
		}
	}
}

inline bool vis_pos(const Vector& from, const Vector& to)
{
	CGameTrace game_trace = {};
	CTraceFilterHitscan filter = {};
	Utils::Trace(from, to, (MASK_SHOT | CONTENTS_GRATE), &filter, &game_trace); //MASK_SOLID
	return game_trace.flFraction > 0.99f;
}

void CVisuals::DrawMovesimLine()
{
	if (Vars::Visuals::MoveSimLine.Value)
	{
		if (!G::PredLinesBackup.empty())
		{
			for (size_t i = 1; i < G::PredLinesBackup.size(); i++)
			{
				Color_t wigger{ 0,0,0,255 };
				
				RenderLine(G::PredLinesBackup.at(i - 1) + Vector2D(-0.1, 0), G::PredLinesBackup.at(i) + Vector2D(-0.1, 0), wigger, false);
				RenderLine(G::PredLinesBackup.at(i - 1) + Vector2D(0.1, 0), G::PredLinesBackup.at(i) + Vector2D(0.1, 0), wigger, false);
				RenderLine(G::PredLinesBackup.at(i - 1) + Vector2D(0, -0.1), G::PredLinesBackup.at(i) + Vector2D(0, -0.1), wigger, false);
				RenderLine(G::PredLinesBackup.at(i - 1) + Vector2D(0, 0.1), G::PredLinesBackup.at(i) + Vector2D(0, 0.1), wigger, false);
				RenderLine(G::PredLinesBackup.at(i - 1), G::PredLinesBackup.at(i), Vars::Menu::Colors::MenuAccent, false);
			}
		}

		if (!G::projectile_lines.empty())
		{
			for (size_t i = 1; i < G::projectile_lines.size(); i++)
			{
				if (!vis_pos(G::projectile_lines.at(i - 1), G::projectile_lines.at(i)))
				{
					break;
				}

				Color_t wigger{ 0,0,0,255 };
				RenderLine(G::projectile_lines.at(i - 1) + Vector2D(-0.1, 0), G::projectile_lines.at(i) + Vector2D(-0.1, 0), wigger, false);
				RenderLine(G::projectile_lines.at(i - 1) + Vector2D(0.1, 0), G::projectile_lines.at(i) + Vector2D(0.1, 0), wigger, false);
				RenderLine(G::projectile_lines.at(i - 1) + Vector2D(0, -0.1), G::projectile_lines.at(i) + Vector2D(0, -0.1), wigger, false);
				RenderLine(G::projectile_lines.at(i - 1) + Vector2D(0, 0.1), G::projectile_lines.at(i) + Vector2D(0, 0.1), wigger, false);
				RenderLine(G::projectile_lines.at(i - 1), G::projectile_lines.at(i), Vars::Menu::Colors::MenuAccent, false);
			}
		}
	}
}

void CVisuals::RenderLine(const Vector& v1, const Vector& v2, Color_t c, bool bZBuffer)
{
	static auto RenderLineFn = reinterpret_cast<void(__cdecl*)(const Vector&, const Vector&, Color_t, bool)>(g_Pattern.Find(L"engine.dll", L"55 8B EC 81 EC ? ? ? ? 56 E8 ? ? ? ? 8B 0D ? ? ? ? 8B 01 FF 90 ? ? ? ? 8B F0 85 F6"));
	RenderLineFn(v1, v2, c, bZBuffer);
}

float lerp()
{
	bool useInterpolation = g_ConVars.FindVar("cl_interpolate")->GetBool();
	float m_nUpdateRate = g_ConVars.FindVar("cl_updaterate")->GetFloat();
	if (useInterpolation)
	{
		float flLerpRatio = g_ConVars.FindVar("cl_interp_ratio")->GetFloat();
		if (flLerpRatio == 0)
			flLerpRatio = 1.0f;
		float flLerpAmount = g_ConVars.FindVar("cl_interp")->GetFloat();

		static const ConVar* pMin = g_ConVars.FindVar("sv_client_min_interp_ratio");
		static const ConVar* pMax = g_ConVars.FindVar("sv_client_max_interp_ratio");
		if (pMin && pMax && pMin->GetFloat() != -1)
		{
			flLerpRatio = std::clamp(flLerpRatio, pMin->GetFloat(), pMax->GetFloat());
		}
		else
		{
			if (flLerpRatio == 0)
				flLerpRatio = 1.0f;
		}
		// #define FIXME_INTERP_RATIO
		return std::max(flLerpAmount, flLerpRatio / m_nUpdateRate);
	}
	else
	{
		return 0.0f;
	}
}

void CVisuals::DrawAimbotFOV(CBaseEntity* pLocal)
{
	//Current Active Aimbot FOV
	if (Vars::Visuals::AimFOVAlpha.Value && Vars::Aimbot::Global::AimFOV.Value)
	{
		const float flFOV = static_cast<float>(Vars::Visuals::FieldOfView.Value);
		const float flR = tanf(DEG2RAD(Vars::Aimbot::Global::AimFOV.Value) / 2.0f) / tanf(DEG2RAD((pLocal->IsScoped() && !Vars::Visuals::RemoveZoom.Value) ? 30.0f : flFOV) / 2.0f) * g_ScreenSize.w;
		const Color_t clr = Colors::FOVCircle;
		g_Draw.OutlinedCircle(g_ScreenSize.w / 2, g_ScreenSize.h / 2, flR, 70, clr);
	}
}
