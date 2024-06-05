#include "dme.h"

#include "Chams.h"
#include "../../Glow/Glow.h"
#include "../../Backtrack/Backtrack.h"
#include "../../../Hooks/HookManager.h"
#include "../../../Hooks/Hooks.h"

bool CDMEChams::ShouldRun()
{
	if (!Vars::Chams::DME::Active.Value || I::EngineVGui->IsGameUIVisible() || !Vars::Chams::Main::Active.
		Value)
		return false;

	return true;
}

IMaterial* CDMEChams::CreateNRef(char const* szName, void* pKV) {
	IMaterial* returnMaterial = I::MaterialSystem->Create(szName, pKV);
	returnMaterial->IncrementReferenceCount();

	int $flags{}, $flags_defined{}, $flags2{}, $flags_defined2{};
	if (auto var = returnMaterial->FindVar("$flags", nullptr))
		$flags = std::stoi(var->GetStringValue());
	if (auto var = returnMaterial->FindVar("$flags_defined", nullptr))
		$flags_defined = std::stoi(var->GetStringValue());
	if (auto var = returnMaterial->FindVar("$flags2", nullptr))
		$flags2 = std::stoi(var->GetStringValue());
	if (auto var = returnMaterial->FindVar("$flags_defined2", nullptr))
		$flags_defined2 = std::stoi(var->GetStringValue());
	backupInformation[returnMaterial] = {
		$flags, $flags_defined, $flags2, $flags_defined2,
	};

	v_MatListGlobal.push_back(returnMaterial);

	return returnMaterial;
}

void CDMEChams::ValidateMaterial(IMaterial* mTarget) {
	ChamInfo backupInfo = backupInformation[mTarget];
	if (auto $flags = mTarget->FindVar("$flags", nullptr))
		$flags->SetIntValue(backupInfo.$flags);
	if (auto $flags_defined = mTarget->FindVar("$flags_defined", nullptr))
		$flags_defined->SetIntValue(backupInfo.$flags_defined);
	if (auto $flags2 = mTarget->FindVar("$flags2", nullptr))
		$flags2->SetIntValue(backupInfo.$flags2);
	if (auto $flags_defined2 = mTarget->FindVar("$flags_defined2", nullptr))
		$flags_defined2->SetIntValue(backupInfo.$flags_defined2);
}

void CDMEChams::Init()
{
	static KeyValues* m_pMatShadedkv = new KeyValues("VertexLitGeneric");
	static KeyValues* m_pMatShinykv = new KeyValues("VertexLitGeneric");
	static KeyValues* m_pMatFlatkv = new KeyValues("UnlitGeneric");
	static KeyValues* m_pMatFresnelkv = new KeyValues("VertexLitGeneric");
	static KeyValues* m_pMatBrickkv = new KeyValues("VertexLitGeneric");
	static KeyValues* m_pMatOverlaykv = new KeyValues("VertexLitGeneric");
	static KeyValues* m_pMatWFShadedkv = new KeyValues("VertexLitGeneric");
	static KeyValues* m_pMatWFShinykv = new KeyValues("VertexLitGeneric");
	static KeyValues* m_pMatWFFlatkv = new KeyValues("UnlitGeneric");

	static bool setup = false;

	if (!setup)
	{
		m_pMatShadedkv->SetString("$basetexture", "vgui/white_additive");
		m_pMatShadedkv->SetString("$bumpmap", "vgui/white_additive");
		m_pMatShadedkv->SetString("$selfillum", "1");
		m_pMatShadedkv->SetString("$selfillumfresnel", "1");
		m_pMatShadedkv->SetString("$selfillumfresnelminmaxexp", "[-0.25 1 1]");
		m_pMatShadedkv->SetString("$cloakPassEnabled", "1");

		m_pMatShinykv->SetString("$basetexture", "vgui/white_additive");
		m_pMatShinykv->SetString("$bumpmap", "vgui/white_additive");
		m_pMatShinykv->SetString("$envmap", "cubemaps/cubemap_sheen001");
		m_pMatShinykv->SetString("$envmaptint", "[1 1 1]");
		m_pMatShinykv->SetString("$selfillum", "1");
		m_pMatShinykv->SetString("$selfillumfresnel", "1");
		m_pMatShinykv->SetString("$selfillumfresnelminmaxexp", "[-0.25 1 1]");
		m_pMatShinykv->SetString("$cloakPassEnabled", "1");

		m_pMatFlatkv->SetString("$basetexture", "vgui/white_additive");
		m_pMatFlatkv->SetString("$cloakPassEnabled", "1");

		m_pMatFresnelkv->SetString("$basetexture", "vgui/white_additive");
		m_pMatFresnelkv->SetString("$bumpmap", "models/player/shared/shared_normal");
		m_pMatFresnelkv->SetString("$envmap", "skybox/sky_dustbowl_01");
		m_pMatFresnelkv->SetString("$envmapfresnel", "1");
		m_pMatFresnelkv->SetString("$phong", "1");
		m_pMatFresnelkv->SetString("$phongfresnelranges", "[0 0.05 0.1]");
		m_pMatFresnelkv->SetString("$selfillum", "1");
		m_pMatFresnelkv->SetString("$selfillumfresnel", "1");
		m_pMatFresnelkv->SetString("$selfillumfresnelminmaxexp", "[0.5 0.5 0]");
		m_pMatFresnelkv->SetString("$selfillumtint", "[0 0 0]");
		m_pMatFresnelkv->SetString("$envmaptint", "[1 1 1]");
		m_pMatFresnelkv->SetString("$cloakPassEnabled", "1");

		m_pMatBrickkv->SetString("$basetexture", "brick/brickwall031b");
		m_pMatBrickkv->SetString("$bumpmap", "vgui/white_additive");
		m_pMatBrickkv->SetString("$color2", "[10 10 10]");
		m_pMatBrickkv->SetString("$additive", "1");
		m_pMatBrickkv->SetString("$envmap", "cubemaps/cubemap_sheen001");
		m_pMatBrickkv->SetString("$envmapfresnel", "1");
		m_pMatBrickkv->SetString("$envmaptint", "[1 1 1]");
		m_pMatBrickkv->SetString("$selfillum", "1");
		m_pMatBrickkv->SetString("$rimlight", "1");
		m_pMatBrickkv->SetString("$rimlightboost", "10");
		m_pMatBrickkv->SetString("$cloakPassEnabled", "1");

		m_pMatOverlaykv->SetString("$basetexture", "models/player/shared/ice_player");
		m_pMatOverlaykv->SetString("$bumpmap", "models/player/shared/shared_normal");
		m_pMatOverlaykv->SetString("$translucent", "1");
		m_pMatOverlaykv->SetString("$additive", "1");
		m_pMatOverlaykv->SetString("$phong", "1");
		m_pMatOverlaykv->SetString("$phongfresnelranges", "[0 0.5 10]");
		m_pMatOverlaykv->SetString("$phongtint", "[0 0 0]");
		m_pMatOverlaykv->SetString("$envmap", "skybox/sky_dustbowl_01");
		m_pMatOverlaykv->SetString("$envmapfresnel", "1");
		m_pMatOverlaykv->SetString("$envmaptint", "[1 1 1]");
		m_pMatOverlaykv->SetString("$selfillum", "1");
		m_pMatOverlaykv->SetString("$selfillumtint", "[0 0 0]");
		m_pMatOverlaykv->SetString("$rimlight", "1");
		m_pMatOverlaykv->SetString("$rimlightboost", "-5");
		m_pMatOverlaykv->SetString("$wireframe", "0");
		m_pMatOverlaykv->SetString("$cloakPassEnabled", "1");

		m_pMatWFShadedkv->SetString("$wireframe", "1");
		m_pMatWFShadedkv->SetString("$basetexture", "vgui/white_additive");
		m_pMatWFShadedkv->SetString("$bumpmap", "vgui/white_additive");
		m_pMatWFShadedkv->SetString("$selfillum", "1");
		m_pMatWFShadedkv->SetString("$selfillumfresnel", "1");
		m_pMatWFShadedkv->SetString("$selfillumfresnelminmaxexp", "[-0.25 1 1]");
		m_pMatWFShadedkv->SetString("$cloakPassEnabled", "1");

		m_pMatWFShinykv->SetString("$wireframe", "1");
		m_pMatWFShinykv->SetString("$basetexture", "vgui/white_additive");
		m_pMatWFShinykv->SetString("$bumpmap", "vgui/white_additive");
		m_pMatWFShinykv->SetString("$envmap", "cubemaps/cubemap_sheen001");
		m_pMatWFShinykv->SetString("$envmaptint", "[1 1 1]");
		m_pMatWFShinykv->SetString("$selfillum", "1");
		m_pMatWFShinykv->SetString("$selfillumfresnel", "1");
		m_pMatWFShinykv->SetString("$selfillumfresnelminmaxexp", "[-0.25 1 1]");
		m_pMatWFShinykv->SetString("$cloakPassEnabled", "1");

		m_pMatWFFlatkv->SetString("$wireframe", "1");
		m_pMatWFFlatkv->SetString("$basetexture", "vgui/white_additive");
		m_pMatWFFlatkv->SetString("$cloakPassEnabled", "1");

		setup = true;
	}

	v_MatList.push_back(nullptr);
	v_MatList.push_back(CreateNRef("m_pMatShaded", m_pMatShadedkv));
	v_MatList.push_back(CreateNRef("m_pMatShiny", m_pMatShinykv));
	v_MatList.push_back(CreateNRef("m_pMatFlat", m_pMatFlatkv));
	v_MatList.push_back(CreateNRef("m_pMatWFShaded", m_pMatWFShadedkv));
	v_MatList.push_back(CreateNRef("m_pMatWFShiny", m_pMatWFShinykv));
	v_MatList.push_back(CreateNRef("m_pMatWFFlat", m_pMatWFFlatkv));
	v_MatList.push_back(CreateNRef("m_pMatFresnel", m_pMatFresnelkv));
	v_MatList.push_back(CreateNRef("m_pMatBrick", m_pMatBrickkv));
	v_MatList.push_back(CreateNRef("m_pMatScuffed", m_pMatOverlaykv));



}

IMaterial* CDMEChams::GetChamMaterial(const Chams_t& chams) {

	return v_MatList.at(chams.drawMaterial) ? v_MatList.at(chams.drawMaterial) : nullptr;	//	this check is only if we end up out of range, unless it crashes
}

//IMaterial* CDMEChams::GetProxyMaterial(int nIndex) {
//
//}

int GetType(int EntIndex) {
	CBaseEntity* pEntity = I::ClientEntityList->GetClientEntity(EntIndex);
	if (!pEntity) { return 0; }
	switch (pEntity->GetClassID()) {
	case ETFClassID::CTFViewModel: {
		return 1;
	}
	case ETFClassID::CBasePlayer:
	case ETFClassID::CTFPlayer: {
		return 2;
	}
	case ETFClassID::CRagdollPropAttached:
	case ETFClassID::CRagdollProp:
	case ETFClassID::CTFRagdoll: {
		return 3;
	}
	case ETFClassID::CTFWearable: {
		return 4;
	}
	case ETFClassID::CTFAmmoPack: {
		return 6;
	}
	case ETFClassID::CBaseAnimating: {
		const auto szName = pEntity->GetModelName();

		if (Hash::IsAmmo(szName))
		{
			return 6;
		}

		if (Hash::IsHealth(szName))
		{
			return 7;
		}

		break;
	}
	case ETFClassID::CObjectSentrygun:
	case ETFClassID::CObjectDispenser:
	case ETFClassID::CObjectTeleporter: {
		return 8;
	}
	case ETFClassID::CTFProjectile_Rocket:
	case ETFClassID::CTFGrenadePipebombProjectile:
	case ETFClassID::CTFProjectile_Jar:
	case ETFClassID::CTFProjectile_JarGas:
	case ETFClassID::CTFProjectile_JarMilk:
	case ETFClassID::CTFProjectile_Arrow:
	case ETFClassID::CTFProjectile_SentryRocket:
	case ETFClassID::CTFProjectile_Flare:
	case ETFClassID::CTFProjectile_Cleaver:
	case ETFClassID::CTFProjectile_EnergyBall:
	case ETFClassID::CTFProjectile_HealingBolt:
	case ETFClassID::CTFProjectile_ThrowableBreadMonster: {
		return 9;
	}
	case ETFClassID::CBaseDoor: {
		return 10;
	}
	}
	CBaseCombatWeapon* pWeapon = reinterpret_cast<CBaseCombatWeapon*>(pEntity);
	if (pWeapon) {
		return 5;
	}
	return -1;
}

Chams_t GetPlayerChams(CBaseEntity* pEntity) {
	CBaseEntity* pLocal = g_EntityCache.GetLocal();
	if (pEntity && pLocal)
	{
		if (pEntity->GetIndex() == G::CurrentTargetIdx && Vars::Chams::Players::Target.chamsActive) {
			return Vars::Chams::Players::Target;
		}
		if (pEntity == pLocal) {
			return Vars::Chams::Players::Local;
		}
		if (g_EntityCache.IsFriend(pEntity->GetIndex()) && Vars::Chams::Players::Friend.chamsActive) {
			return Vars::Chams::Players::Friend;
		}
		if (pEntity->GetTeamNum() != pLocal->GetTeamNum()) {
			return Vars::Chams::Players::Enemy;
		}
		if (pEntity->GetTeamNum() == pLocal->GetTeamNum()) {
			return Vars::Chams::Players::Team;
		}
	}
	return Chams_t();
}

Chams_t GetBuildingChams(CBaseEntity* pEntity) {
	CBaseEntity* pLocal = g_EntityCache.GetLocal();
	if (pEntity && pLocal)
	{
		if (pEntity->GetIndex() == G::CurrentTargetIdx && Vars::Chams::Buildings::Target.chamsActive) {
			return Vars::Chams::Buildings::Target;
		}
		if (pEntity->GetIndex() == pLocal->GetIndex()) {
			return Vars::Chams::Buildings::Local;
		}
		if (g_EntityCache.IsFriend(pEntity->GetIndex()) && Vars::Chams::Buildings::Friend.chamsActive) {
			return Vars::Chams::Buildings::Friend;
		}
		if (pEntity->GetTeamNum() != pLocal->GetTeamNum()) {
			return Vars::Chams::Buildings::Enemy;
		}
		if (pEntity->GetTeamNum() == pLocal->GetTeamNum()) {
			return Vars::Chams::Buildings::Team;
		}
	}
	return Chams_t();
}

Chams_t getChamsType(int nIndex, CBaseEntity* pEntity = nullptr) {
	switch (nIndex) {
	case 0: {
		return Vars::Chams::DME::Weapon;
	}
	case 1: {
		return Vars::Chams::DME::Hands;
	}
	case 2: {
		return pEntity ? GetPlayerChams(pEntity) : Chams_t();
	}
	case 3: {
		return Vars::Chams::Players::Ragdoll;
	}
	case 4: {
		if (!Vars::Chams::Players::Wearables.Value) { return Chams_t(); }
		if (!pEntity) { return Chams_t(); }
		if (CBaseEntity* pOwner = I::ClientEntityList->GetClientEntityFromHandle(pEntity->m_hOwnerEntity())) {
			return GetPlayerChams(pOwner);
		}
		return Chams_t();
	}
	case 5: {
		if (!Vars::Chams::Players::Weapons.Value) { return Chams_t(); }
		if (!pEntity) { return Chams_t(); }
		if (CBaseEntity* pOwner = I::ClientEntityList->GetClientEntityFromHandle(pEntity->m_hOwnerEntity())) {
			return GetPlayerChams(pOwner);
		}
		return Chams_t();
	}
	case 6: {
		return Vars::Chams::World::Ammo;
	}
	case 7: {
		return Vars::Chams::World::Health;
	}
	case 8: {
		if (!pEntity) { return Chams_t(); }
		const auto& Building = reinterpret_cast<CBaseObject*>(pEntity);
		if (!Building || !(!Building->GetCarried() && Building->GetConstructed())) { return Chams_t(); }
		if (CBaseEntity* pOwner = Building->GetOwner()) {
			return GetBuildingChams(pOwner);
		}
		else if (int teamNum = pEntity->GetTeamNum()) {	// if we don't have an owner, we need to do this, or else spawned buildings that do have a team will return no cham struct.
			CBaseEntity* pLocal = g_EntityCache.GetLocal();
			if (pLocal) {
				return (teamNum = pLocal->GetTeamNum()) ? Vars::Chams::Buildings::Team : Vars::Chams::Buildings::Enemy;
			}
		}
		return Chams_t();
	}
	case 9: {
		if (!pEntity) { return Chams_t(); }
		if (CBaseEntity* pOwner = I::ClientEntityList->GetClientEntityFromHandle(reinterpret_cast<int>(pEntity->GetThrower()))) {
			return GetPlayerChams(pOwner);
		}
		return Chams_t();
	}
	default:
		return Chams_t();
	}
}



bool CDMEChams::Render(const DrawModelState_t& pState, const ModelRenderInfo_t& pInfo, matrix3x4* pBoneToWorld)
{
	const auto dmeHook = g_HookManager.GetMapHooks()["ModelRender_DrawModelExecute"];
	const auto& pRenderContext = I::MaterialSystem->GetRenderContext();

	m_bRendering = false;
	if (ShouldRun() && pRenderContext)
	{
		m_bRendering = true;

		const int drawType = GetType(pInfo.m_nEntIndex);

		// filter weapon draws
		if (!drawType)
		{
			std::string_view szModelName(I::ModelInfoClient->GetModelName(pInfo.m_pModel));
			if (!(szModelName.find(_("weapon")) != std::string_view::npos
				&& szModelName.find(_("arrow")) == std::string_view::npos
				&& szModelName.find(_("w_syringe")) == std::string_view::npos
				&& szModelName.find(_("nail")) == std::string_view::npos
				&& szModelName.find(_("shell")) == std::string_view::npos
				&& szModelName.find(_("parachute")) == std::string_view::npos
				&& szModelName.find(_("buffbanner")) == std::string_view::npos
				&& szModelName.find(_("shogun_warbanner")) == std::string_view::npos
				&& szModelName.find(_("targe")) == std::string_view::npos //same as world model, can't filter
				&& szModelName.find(_("shield")) == std::string_view::npos //same as world model, can't filter
				&& szModelName.find(_("repair_claw")) == std::string_view::npos)) {
				return false;
			}
		}
		if (drawType == -1) { return false; }

		CBaseEntity* pEntity = I::ClientEntityList->GetClientEntity(pInfo.m_nEntIndex);
		CBaseEntity* pLocal = g_EntityCache.GetLocal();

		if (drawType == 3) {	//	don't interfere with ragdolls
			if (Vars::Visuals::RagdollEffects::RagdollType.Value) {
				if (Vars::Visuals::RagdollEffects::EnemyOnly.Value) {
					if (pEntity->GetTeamNum() == pLocal->GetTeamNum()) {
						return false;
					}
				}
				else {
					return false;
				}
			}
		}


		Chams_t chams = pEntity ? getChamsType(drawType, pEntity) : getChamsType(drawType);
		Color_t team = Utils::GetEntityDrawColor(pEntity, Vars::ESP::Main::EnableTeamEnemyColors.Value);
		if (!chams.chamsActive) { return false; }

		{
			pRenderContext->DepthRange(0.0f, 1.f);
			I::ModelRender->ForcedMaterialOverride(nullptr);
			I::RenderView->SetColorModulation(1.0f, 1.0f, 1.0f);
			I::RenderView->SetBlend(1.0f);
		}



		// handle
		{


			IMaterial* chamsMaterial = GetChamMaterial(chams);

			if (chamsMaterial) {
				chamsMaterial->IncrementReferenceCount();
			}

			pRenderContext->DepthRange(0.0f, chams.showObstructed ? 0.2f : 1.f);

			I::ModelRender->ForcedMaterialOverride(chamsMaterial);

			if (dmeHook) {
				dmeHook->Original<void(__thiscall*)(CModelRender*, const DrawModelState_t&, const ModelRenderInfo_t&, matrix3x4*)>()(I::ModelRender, pState, pInfo, pBoneToWorld);
			}

			pRenderContext->DepthRange(0.0f, 1.f);
			I::ModelRender->ForcedMaterialOverride(nullptr);
			I::RenderView->SetColorModulation(1.0f, 1.0f, 1.0f);

			I::RenderView->SetBlend(1.0f);

			return true;
		}
		m_bRendering = false;
	}

	return false;
}