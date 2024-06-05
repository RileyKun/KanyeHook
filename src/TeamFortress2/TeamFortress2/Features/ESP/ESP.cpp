#include "ESP.h"

#include "../Vars.h"
#include "../Visuals/Visuals.h"
#include "../Menu/Playerlist/Playerlist.h"

#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include "../Backtrack/Backtrack.h"


bool CESP::ShouldRun()
{
	if (!Vars::ESP::Main::Active.Value)
	{
		return false;
	}

	return true;
}

void CESP::Run()
{
	if (const auto& pLocal = g_EntityCache.GetLocal())
	{
		if (ShouldRun())
		{
			DrawWorld();
			DrawBuildings(pLocal);
			DrawPlayers(pLocal);
		}
	}
}


std::wstring CESP::ConvertUtf8ToWide(const std::string& str)
{
	int count = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.length(), NULL, 0);
	std::wstring wstr(count, 0);
	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.length(), &wstr[0], count);
	return wstr;
}

bool CESP::GetDrawBounds(CBaseEntity* pEntity, Vec3* vTrans, int& x, int& y, int& w, int& h)
{
	bool bIsPlayer = false;
	Vec3 vMins, vMaxs;

	if (pEntity->IsPlayer())
	{
		bIsPlayer = true;
		const bool bIsDucking = pEntity->IsDucking();
		vMins = I::GameMovement->GetPlayerMins(bIsDucking);
		vMaxs = I::GameMovement->GetPlayerMaxs(bIsDucking);
	}

	else
	{
		vMins = pEntity->GetCollideableMins();
		vMaxs = pEntity->GetCollideableMaxs();
	}

	const matrix3x4& transform = pEntity->GetRgflCoordinateFrame();

	const Vec3 vPoints[] =
	{
		Vec3(vMins.x, vMins.y, vMins.z),
		Vec3(vMins.x, vMaxs.y, vMins.z),
		Vec3(vMaxs.x, vMaxs.y, vMins.z),
		Vec3(vMaxs.x, vMins.y, vMins.z),
		Vec3(vMaxs.x, vMaxs.y, vMaxs.z),
		Vec3(vMins.x, vMaxs.y, vMaxs.z),
		Vec3(vMins.x, vMins.y, vMaxs.z),
		Vec3(vMaxs.x, vMins.y, vMaxs.z)
	};

	for (int n = 0; n < 8; n++)
	{
		Math::VectorTransform(vPoints[n], transform, vTrans[n]);
	}

	Vec3 flb, brt, blb, frt, frb, brb, blt, flt;

	if (Utils::W2S(vTrans[3], flb) && Utils::W2S(vTrans[5], brt)
		&& Utils::W2S(vTrans[0], blb) && Utils::W2S(vTrans[4], frt)
		&& Utils::W2S(vTrans[2], frb) && Utils::W2S(vTrans[1], brb)
		&& Utils::W2S(vTrans[6], blt) && Utils::W2S(vTrans[7], flt)
		&& Utils::W2S(vTrans[6], blt) && Utils::W2S(vTrans[7], flt))
	{
		const Vec3 arr[] = { flb, brt, blb, frt, frb, brb, blt, flt };

		float left = flb.x;
		float top = flb.y;
		float righ = flb.x;
		float bottom = flb.y;

		for (int n = 1; n < 8; n++)
		{
			if (left > arr[n].x)
			{
				left = arr[n].x;
			}

			if (top < arr[n].y)
			{
				top = arr[n].y;
			}

			if (righ < arr[n].x)
			{
				righ = arr[n].x;
			}

			if (bottom > arr[n].y)
			{
				bottom = arr[n].y;
			}
		}

		float x_ = left;
		const float y_ = bottom;
		float w_ = righ - left;
		const float h_ = top - bottom;


		x = static_cast<int>(x_);
		y = static_cast<int>(y_);
		w = static_cast<int>(w_);
		h = static_cast<int>(h_);

		return !(x > g_ScreenSize.w || x + w < 0 || y > g_ScreenSize.h || y + h < 0);
	}

	return false;
}

Vec3 CESP::Predictor_t::Extrapolate(float time)
{
	Vec3 vecOut = {};

	if (m_pEntity->IsOnGround())
		vecOut = (m_vPosition + (m_vVelocity * time));

	else vecOut = (m_vPosition + (m_vVelocity * time) - m_vAcceleration * 0.5f * time * time);

	return vecOut;
}

void DrawCircle(Vector2D Center, Color_t c, float rad, int segments)
{
	float ang_step = (2.0f * PI) / (float)segments;
	for (float Angle = 0; Angle < 2.0f * PI; Angle += ang_step)
	{
		Vector2D pos1 = Vector2D(cosf(Angle), sinf(Angle)) * rad + Center;
		Vector2D pos2 = Vector2D(cosf(Angle + ang_step), sinf(Angle + ang_step)) * rad + Center;
		g_Draw.Line(pos1.x, pos1.y, pos2.x, pos2.y, c);
	}
}

void DrawOutlinedCircle(Vector2D Center, Color_t c, int rad)
{
	int res = (2 + 1) * 12;

    DrawCircle(Center + Vector2D(-1, 0), Color_t{ 0, 0, 0, 255 }, rad, res);
	DrawCircle(Center + Vector2D(1, 0), Color_t{ 0, 0, 0, 255 }, rad, res);
	DrawCircle(Center + Vector2D(0, -1), Color_t{ 0, 0, 0, 255 }, rad, res);
	DrawCircle(Center + Vector2D(0, 1), Color_t{ 0, 0, 0, 255 }, rad, res);
	DrawCircle(Center, c, rad, res);
}

void RenderSkeleton(CBaseEntity* pLocal)
{

	//Get the appropriate drawing color
	auto SourceColor = pLocal->IsUbered() ? Color_t{ 255,105,180,255 } : Color_t{ 255,255,255,255 };
	auto Mdl = pLocal->GetModel();
	if (!Mdl) return;
	studiohdr_t* pStudioHdr = I::ModelInfoClient->GetStudioModel(Mdl);
	if (!pStudioHdr) return;

	Vector vParent, vChild, sParent, sChild;

	for (int j = 0; j < pStudioHdr->numbones; j++)
	{
		mstudiobone_t* pBone = pStudioHdr->GetBone(j);
		if (pBone && (pBone->flags & BONE_USED_BY_HITBOX) && (pBone->parent != -1))
		{
			Vector vChild = pLocal->GetBonePos(j);
			Vector vParent = pLocal->GetBonePos(pBone->parent);
			Vector sParent, sChild;

			if (Utils::W2S(vParent, sParent) && Utils::W2S(vChild, sChild))
				g_Draw.Line(sParent.x, sParent.y, sChild.x, sChild.y, SourceColor);
		}
	}

	Vector vHeadPos;
	Vector vHeadTop;
	Utils::W2S(pLocal->GetHitboxPos(HITBOX_HEAD), vHeadPos);
	Utils::W2S(pLocal->GetHitboxPos(HITBOX_HEAD) + Vector(0, 0, 5), vHeadTop);
	DrawOutlinedCircle(Vector2D(vHeadPos.x, vHeadPos.y), SourceColor, vHeadTop.DistTo(vHeadPos));
}


Vec3 p(CBaseEntity* Player)
{
	Vec3 vecOut = {};
	Vec3 vel = Player->GetVelocity(); //we fast
	auto diff = std::clamp((TIME_TO_TICKS(fabs(Player->GetSimulationTime() - Player->GetOldSimulationTime()))) - 0.5f, 0.f, 22.f);
	Vec3 Accel = Vec3(0.0f, 0.0f, 800); 
	float i = 0;
	for (i; i < diff; ++i)
	{

	}
	float Time = (i * (1.0f / 66.7f)); //64/interval (csgo)

	if (Player->IsOnGround())
		vecOut = (Player->GetAbsOrigin() + (vel * Time));
	else
		vecOut = (Player->GetAbsOrigin() + (vel * Time) - Accel * 0.5f * Time * Time);

	return vecOut;
}


int Choke(CBaseEntity* Player)
{
	if (Player->GetSimulationTime() > Player->GetOldSimulationTime())
		return TIME_TO_TICKS(fabs(Player->GetSimulationTime() - Player->GetOldSimulationTime()));
	return 0;

}

void DrawBackTrack(CBaseEntity* pEntity, Color_t colourface, Color_t colouredge, float time)
{

	if (const auto& pLastTick = F::Backtrack.GetRecord(pEntity->GetIndex(), BacktrackMode::Last))
	{
		const model_t* model = pLastTick->Model;
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
			Math::ConcatTransforms(pLastTick->BoneMatrix.BoneMatrix[bbox->bone], rotMatrix, matrix);

			const Vec3 vPos = (bbox->bbmin + bbox->bbmax) * 0.5f;
			Vec3 vOut;
			const matrix3x4& bone = pLastTick->BoneMatrix.BoneMatrix[bbox->bone];
			Math::VectorTransform(vPos, bone, vOut);

			Vec3 bboxAngle;
			Math::MatrixAngles(matrix, bboxAngle);

			Math::GetMatrixOrigin(matrix, vOut);

			I::DebugOverlay->AddBoxOverlay2(vOut, bbox->bbmin, bbox->bbmax, bboxAngle, colourface, colouredge, time);

		}
	}
}


void CESP::DrawPlayers(CBaseEntity* pLocal)
{


	if (!Vars::ESP::Main::Active.Value && !I::EngineClient->IsConnected() || !I::EngineClient->IsInGame())
	{
		return;
	}

	for (const auto& Player : g_EntityCache.GetGroup(EGroupType::PLAYERS_ALL))
	{
		if (!Player->IsAlive() || Player->IsAGhost())
		{
			continue;
		}

		int nIndex = Player->GetIndex();
		bool bIsLocal = nIndex == I::EngineClient->GetLocalPlayer();

		if (!bIsLocal)
		{
			switch (Vars::ESP::Players::IgnoreCloaked.Value)
			{
			case 0: { break; }
			case 1:
			{
				if (Player->IsCloaked()) { continue; }
				break;
			}
			case 2:
			{
				if (Player->IsCloaked() && Player->GetTeamNum() != pLocal->GetTeamNum()) { continue; }
				break;
			}
			}
		}
		else
		{
			//RenderSkeleton();
		}

		Color_t drawColor = Utils::GetEntityDrawColor(Player, false);

		int x = 0, y = 0, w = 0, h = 0;
		Vec3 vTrans[8];
		if (GetDrawBounds(Player, vTrans, x, y, w, h))
		{
	
			int nHealth = Player->GetHealth(), nMaxHealth = Player->GetMaxHealth();
			Color_t healthColor = Utils::GetHealthColor(nHealth, nMaxHealth);

			size_t FONT = FONT_ESP, FONT_NAME = FONT_ESP_NAME;

			int nTextX = x + w + 3, nTextOffset = -1, nClassNum = Player->GetClassNum();

			I::VGuiSurface->DrawSetAlphaMultiplier(1.f);

			if (nClassNum == CLASS_MEDIC)
			{
				if (const auto& pMedGun = Player->GetWeaponFromSlot(SLOT_SECONDARY))
				{
					if (pMedGun->GetUberCharge())
					{
						x += w + 1;

						float flUber = pMedGun->GetUberCharge() * (pMedGun->GetItemDefIndex() == Medic_s_TheVaccinator
							? 400.0f
							: 100.0f);

						float flMaxUber = (pMedGun->GetItemDefIndex() == Medic_s_TheVaccinator ? 400.0f : 100.0f);

						if (flUber > flMaxUber)
						{
							flUber = flMaxUber;
						}
						float flHealth = static_cast<float>(nHealth);
						float flMaxHealth = static_cast<float>(nMaxHealth);


						static constexpr int RECT_WIDTH = 2;
						int nHeight = h + (flUber < flMaxUber ? 2 : 1);
						int nHeight2 = h + 1;

						float ratio = flUber / flMaxUber;

						g_Draw.Rect((x + RECT_WIDTH), (y + nHeight - (nHeight * ratio)), RECT_WIDTH, (nHeight * ratio), Colors::UberColor);
						g_Draw.OutlinedRect(x + RECT_WIDTH - 2, y + nHeight - nHeight * ratio - 1, RECT_WIDTH + 2, nHeight * ratio + 2, Colors::OutlineESP);


						x -= w + 1;
					}
				}
			}

			PlayerInfo_t pi{};
			if (I::EngineClient->GetPlayerInfo(nIndex, &pi))
			{
				int middle = x + w / 2;

				int offset = (g_Draw.m_vecFonts[FONT_NAME].nTall + (g_Draw.m_vecFonts[FONT_NAME].nTall / 6));
				std::string attackString = std::string(pi.name) + " (" + Utils::GetClassByIndex(nClassNum) + ")";
				g_Draw.String(FONT_NAME, middle, y - offset, drawColor, ALIGN_CENTERHORIZONTAL, Utils::ConvertUtf8ToWide(attackString).data());

			}
			
			int offset = g_Draw.m_vecFonts[FONT].nTall / 8;

		


			Colors::Cond = { 255,225, 255,255 };

			const int nCond = Player->GetCond();
			const int nCondEx = Player->GetCondEx();
			const int nCondEx2 = Player->GetCondEx2();
			const Color_t teamColors = Utils::GetTeamColor(Player->GetTeamNum(), Vars::ESP::Main::EnableTeamEnemyColors.Value);
			{
				if (nCond & TFCond_Ubercharged || nCondEx & TFCondEx_UberchargedHidden || nCondEx & TFCondEx_UberchargedCanteen)
				{
					g_Draw.String(FONT_ESP_COND, nTextX, y + nTextOffset, { 255, 255, 255, 255 }, ALIGN_DEFAULT, "UBER");
					nTextOffset += g_Draw.m_vecFonts[FONT_ESP_COND].nTall;
				}

				if (nCondEx & TFCondEx_BulletResistance || nCondEx & TFCondEx_BulletCharge)
				{
					g_Draw.String(FONT_ESP_COND, nTextX, y + nTextOffset, { 255, 255, 255, 255 }, ALIGN_DEFAULT, "BULLET (RES)");
					nTextOffset += g_Draw.m_vecFonts[FONT_ESP_COND].nTall;
				}

				if (nCondEx & TFCondEx_ExplosiveResistance || nCondEx & TFCondEx_ExplosiveCharge)
				{
					g_Draw.String(FONT_ESP_COND, nTextX, y + nTextOffset, { 255, 255, 255, 255 }, ALIGN_DEFAULT, "BLAST(RES)");
					nTextOffset += g_Draw.m_vecFonts[FONT_ESP_COND].nTall;
				}

				if (nCondEx & TFCondEx_FireResistance || nCondEx & TFCondEx_FireCharge)
				{
					g_Draw.String(FONT_ESP_COND, nTextX, y + nTextOffset, { 255, 255, 255, 255 }, ALIGN_DEFAULT, "FIRE (RES)");
					nTextOffset += g_Draw.m_vecFonts[FONT_ESP_COND].nTall;
				}

				if (Player->IsCritBoostedNoMini())
				{															//light red
					g_Draw.String(FONT_ESP_COND, nTextX, y + nTextOffset, { 255, 107, 108, 255 }, ALIGN_DEFAULT, "CRIT");
					nTextOffset += g_Draw.m_vecFonts[FONT_ESP_COND].nTall;
				}

				if (G::PlayerPriority[pi.friendsID].Mode == 4 || g_EntityCache.IsFriend(nIndex))
				{
					g_Draw.String(FONT_ESP_COND, nTextX, y + nTextOffset, { 255, 0,0, 255 }, ALIGN_DEFAULT, "CHEATER");
					nTextOffset += g_Draw.m_vecFonts[FONT_ESP_COND].nTall;
				}
			}

			x -= 1;

			auto flHealth = static_cast<float>(nHealth);
			auto flMaxHealth = static_cast<float>(nMaxHealth);

			Gradient_t clr = flHealth > flMaxHealth ? Colors::GradientOverhealBar : Colors::GradientHealthBar;

			Color_t HealthColor = flHealth > flMaxHealth ? Colors::Health : Utils::GetHealthColor(nHealth, nMaxHealth);

			if (!Player->IsVulnerable())
			{
				clr = { Colors::Invuln, Colors::Invuln };
			}

			if (flHealth > flMaxHealth)
			{
				flHealth = flMaxHealth;
			}
			bool d = true;
			float ratio = flHealth / flMaxHealth;

			float SPEED_FREQ = 360 / 0.6;

			int player_hp = flHealth;
			int player_hp_max = flMaxHealth;

			static float prev_player_hp[45];

			if (prev_player_hp[Player->GetIndex()] > player_hp)
				prev_player_hp[Player->GetIndex()] -= SPEED_FREQ * I::GlobalVars->frametime;
			else
				prev_player_hp[Player->GetIndex()] = player_hp;


			g_Draw.OutlinedGradientBar(x - 2 - 3, y + h, 3, h, prev_player_hp[Player->GetIndex()] / player_hp_max, Colors::Health, Colors::Health, Colors::OutlineESP, false);
			x += 1;


			g_Draw.String(FONT, x - 8, (y + h) - (prev_player_hp[Player->GetIndex()] / player_hp_max * h) - 2, Colors::White, ALIGN_REVERSE, "%d", nHealth);



			I::VGuiSurface->DrawSetAlphaMultiplier(1.0f);
		}
	}
}

void CESP::DrawBuildings(CBaseEntity* pLocal) const
{
	if (!Vars::ESP::Buildings::Active.Value || !Vars::ESP::Main::Active.Value)
	{
		return;
	}

	for (const auto& pBuilding : g_EntityCache.GetGroup(EGroupType::BUILDINGS_ALL))
	{
		if (!pBuilding->IsAlive())
		{
			continue;
		}

		size_t FONT = FONT_ESP, FONT_NAME = FONT_ESP_NAME;

		const auto& building = reinterpret_cast<CBaseObject*>(pBuilding);

		Color_t drawColor = Utils::GetTeamColor(building->GetTeamNum(), Vars::ESP::Main::EnableTeamEnemyColors.Value);

		int x = 0, y = 0, w = 0, h = 0;
		Vec3 vTrans[8];
		if (GetDrawBounds(building, vTrans, x, y, w, h))
		{
			const auto nHealth = building->GetHealth();
			const auto nMaxHealth = building->GetMaxHealth();
			auto nTextOffset = 0, nTextTopOffset = 0;
			const auto nTextX = x + w + 3;

			Color_t healthColor = Utils::GetHealthColor(nHealth, nMaxHealth);

			const auto nType = static_cast<EBuildingType>(building->GetType());

			const bool bIsMini = building->GetMiniBuilding();


			const wchar_t* szName;

			switch (nType)
			{
			case EBuildingType::SENTRY:
			{
				if (bIsMini)
				{
					szName = L"Mini Sentry";
				}
				else
				{
					szName = L"Sentry";
				}
				break;
			}
			case EBuildingType::DISPENSER:
			{
				szName = L"Dispenser";
				break;
			}
			case EBuildingType::TELEPORTER:
			{
				if (building->GetObjectMode())
				{
					szName = L"Teleporter Out";
				}
				else
				{
					szName = L"Teleporter In";
				}
				break;
			}
			default:
			{
				szName = L"Unknown";
				break;
			}
			}

			const char level = building->GetLevel();


			nTextTopOffset += g_Draw.m_vecFonts[FONT_NAME].nTall + g_Draw.m_vecFonts[FONT_NAME].nTall / 4;
			g_Draw.String(FONT_NAME, x + w / 2, y - nTextTopOffset, drawColor, ALIGN_CENTERHORIZONTAL, szName);



			std::vector<std::wstring> condStrings{};

			if (nType == EBuildingType::SENTRY && building->GetControlled())
			{
				condStrings.emplace_back(L"Wrangled");
			}

			if (building->IsSentrygun() && !building->GetConstructing())
			{
				int iShells;
				int iMaxShells;
				int iRockets;
				int iMaxRockets;

				building->GetAmmoCount(iShells, iMaxShells, iRockets, iMaxRockets);

				if (iShells == 0)
					condStrings.emplace_back(L"No Ammo");

				if (!bIsMini && iRockets == 0)
					condStrings.emplace_back(L"No Rockets");
			}

			if (!condStrings.empty())
			{
				for (auto& condString : condStrings)
				{
					g_Draw.String(FONT_NAME, nTextX, y + nTextOffset, { 255,255,255,255 }, ALIGN_DEFAULT, condString.data());
					nTextOffset += g_Draw.m_vecFonts[FONT_NAME].nTall;
				}
			}



			x -= 1;

			auto flHealth = static_cast<float>(nHealth);
			const auto flMaxHealth = static_cast<float>(nMaxHealth);

			if (flHealth > flMaxHealth)
			{
				flHealth = flMaxHealth;
			}

			static constexpr int RECT_WIDTH = 2;
			const int nHeight = h + (flHealth < flMaxHealth ? 2 : 1);
			int nHeight2 = h + 1;

			const float ratio = flHealth / flMaxHealth;

			g_Draw.Rect(x - RECT_WIDTH - 2, y + nHeight - nHeight * ratio, RECT_WIDTH, nHeight * ratio,
				healthColor);


			g_Draw.OutlinedRect(x - RECT_WIDTH - 2 - 1, y + nHeight - nHeight * ratio - 1, RECT_WIDTH + 2, nHeight * ratio + 2, Colors::OutlineESP);


			x += 1;

		}
		I::VGuiSurface->DrawSetAlphaMultiplier(1.0f);
	}
}

void CESP::DrawWorld() const
{
	if (!Vars::ESP::World::Active.Value || !Vars::ESP::Main::Active.Value)
	{
		return;
	}

	Vec3 vScreen = {};
	constexpr size_t FONT = FONT_ESP_PICKUPS;

	I::VGuiSurface->DrawSetAlphaMultiplier(Vars::ESP::World::Alpha.Value);

	
		for (const auto& health : g_EntityCache.GetGroup(EGroupType::WORLD_HEALTH))
		{
			int x = 0, y = 0, w = 0, h = 0;
			Vec3 vTrans[8];
			if (GetDrawBounds(health, vTrans, x, y, w, h))
			{
				if (Utils::W2S(health->GetVecOrigin(), vScreen))
				{
					g_Draw.String(FONT, vScreen.x, y + h, Colors::Health, ALIGN_CENTER, _(L"HEALTH"));
				}
			} // obviously a health pack isn't going to be upside down, this just looks nicer.
		}

		CBaseEntity* pEntity;
		for (int n = 1; n < I::ClientEntityList->GetHighestEntityIndex(); n++)
		{
			pEntity = I::ClientEntityList->GetClientEntity(n);

			if (!pEntity)
				continue;

			auto nClassID = pEntity->GetClassID();
			switch (nClassID)
			{
			case ETFClassID::CTFProjectile_Rocket:
			case ETFClassID::CTFGrenadePipebombProjectile:
			case ETFClassID::CTFProjectile_Jar:
			case ETFClassID::CTFProjectile_JarGas:
			case ETFClassID::CTFProjectile_JarMilk:
			case ETFClassID::CTFProjectile_Arrow:
			case ETFClassID::CTFProjectile_SentryRocket:
			case ETFClassID::CTFProjectile_Flare:
			case ETFClassID::CTFProjectile_GrapplingHook:
			case ETFClassID::CTFProjectile_Cleaver:
			case ETFClassID::CTFProjectile_EnergyBall:
			case ETFClassID::CTFProjectile_EnergyRing:
			case ETFClassID::CTFProjectile_HealingBolt:
			case ETFClassID::CTFProjectile_ThrowableBreadMonster:
			{
				int x = 0, y = 0, w = 0, h = 0;
				Vec3 vTrans[8];
				if (GetDrawBounds(pEntity, vTrans, x, y, w, h))
				{
					if (Utils::W2S(pEntity->GetVecOrigin(), vScreen))
					{
						g_Draw.String(FONT, vScreen.x, y + h, { 255,255,255,255 }, ALIGN_CENTER, _(L"PROJECTILE"));
					
					}

					if (Utils::W2S(pEntity->GetVecOrigin() + pEntity->GetVelocity() * ((pEntity->GetVelocity().Length2D() * 0.015) / 64), vScreen))
					{
						g_Draw.Line(vScreen.x, vScreen.y, x, y, {255,255,255,255});
					}
				} // obviously a health pack isn't going to be upside down, this just looks nicer.
				break;
			}
			default:
			{
	
				break;
			}
			}
		}

		for (const auto& ammo : g_EntityCache.GetGroup(EGroupType::WORLD_AMMO))
		{
			int x = 0, y = 0, w = 0, h = 0;
			Vec3 vTrans[8];
			if (GetDrawBounds(ammo, vTrans, x, y, w, h))
			{
				if (Utils::W2S(ammo->GetVecOrigin(), vScreen))
				{
					g_Draw.String(FONT, vScreen.x, y + h, Colors::Ammo, ALIGN_CENTER, _(L"AMMO"));
				}
			}
		}
	

	I::VGuiSurface->DrawSetAlphaMultiplier(1.0f);
}



const wchar_t* CESP::GetPlayerClass(int nClassNum)
{
	static const wchar_t* szClasses[] = {
		L"unknown", L"Scout", L"Sniper", L"Soldier", L"Demoman",
		L"Medic", L"Heavy", L"Pyro", L"Spy", L"Engineer"
	};

	return nClassNum < 10 && nClassNum > 0 ? szClasses[nClassNum] : szClasses[0];
}

void CESP::CreateDLight(int nIndex, Color_t DrawColor, const Vec3& vOrigin, float flRadius)
{
	
}

//Got this from dude719, who got it from somewhere else
void CESP::Draw3DBox(const Vec3* vPoints, Color_t clr)
{
	
}

void CESP::DrawBones(CBaseEntity* pPlayer, const std::vector<int>& vecBones, Color_t clr)
{
	const size_t nMax = vecBones.size(), nLast = nMax - 1;
	for (size_t n = 0; n < nMax; n++)
	{
		if (n == nLast)
		{
			continue;
		}

		const auto vBone = pPlayer->GetHitboxPos(vecBones[n]);
		const auto vParent = pPlayer->GetHitboxPos(vecBones[n + 1]);

		Vec3 vScreenBone, vScreenParent;

		if (Utils::W2S(vBone, vScreenBone) && Utils::W2S(vParent, vScreenParent))
		{
			g_Draw.Line(vScreenBone.x, vScreenBone.y, vScreenParent.x, vScreenParent.y, clr);
		}
	}
}
