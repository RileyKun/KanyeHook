#include "Chams.h"

bool CChams::ShouldRun()
{
	return !I::EngineVGui->IsGameUIVisible();
}

void CChams::DrawModel(CBaseEntity* pEntity)
{
	m_bRendering = true;
	pEntity->DrawModel(STUDIO_RENDER | STUDIO_NOSHADOWS);
	m_DrawnEntities[pEntity] = true;
	m_bRendering = false;
}

void CChams::Init()
{

	{
		auto kv = new KeyValues("UnlitGeneric");
		kv->SetString("$wireframe", "vgui/white_additive");
		m_pMatFlat = I::MaterialSystem->Create("m_pMatFlat", kv);
	}

	m_pMatBlur = I::MaterialSystem->Find("models/effects/muzzleflash/blurmuzzle", "Model textures");
}

void CChams::Render()
{
	if (!m_DrawnEntities.empty())
		m_DrawnEntities.clear();

	m_bHasSetStencil = false;

	if (const auto& pLocal = g_EntityCache.GetLocal())
	{
		if (!ShouldRun())
			return;

		if (const auto& pRenderContext = I::MaterialSystem->GetRenderContext())
		{
			//Let's do this in advance if Glow is enabled.
			/*if (Vars::Glow::Main::Active.m_Var)
			{*/
			ShaderStencilState_t StencilState = {};
			StencilState.m_bEnable = true;
			StencilState.m_nReferenceValue = 1;
			StencilState.m_CompareFunc = STENCILCOMPARISONFUNCTION_ALWAYS;
			StencilState.m_PassOp = STENCILOPERATION_REPLACE;
			StencilState.m_FailOp = STENCILOPERATION_KEEP;
			StencilState.m_ZFailOp = STENCILOPERATION_REPLACE;
			StencilState.SetStencilState(pRenderContext);
			m_bHasSetStencil = true;
			//}

			RenderEnts(pLocal, pRenderContext);
		}
	}
}

void CChams::RenderEnts(CBaseEntity* pLocal, IMatRenderContext* pRenderContext)
{
	if (!Vars::Chams::Main::Active.Value)
		return;

	std::vector<CBaseEntity*> Entities = g_EntityCache.GetGroup(EGroupType::PLAYERS_ALL);

	for (const auto& Entity : g_EntityCache.GetGroup(EGroupType::BUILDINGS_ALL)) {
		Entities.push_back(Entity);
	}
	for (const auto& Entity : g_EntityCache.GetGroup(EGroupType::WORLD_HEALTH)) {
		Entities.push_back(Entity);
	}
	for (const auto& Entity : g_EntityCache.GetGroup(EGroupType::WORLD_AMMO)) {
		Entities.push_back(Entity);
	}
	for (const auto& Entity : g_EntityCache.GetGroup(EGroupType::WORLD_PROJECTILES)) {
		Entities.push_back(Entity);
	}

	if (Entities.empty())
		return;

	for (const auto& Entity : Entities)
	{
		if (Entity->GetDormant())
			continue;

		const bool isPlayer = Entity->IsPlayer();
		if (isPlayer && (!Entity->IsAlive() || Entity->IsAGhost()))
			continue;

		if (!Utils::IsOnScreen(pLocal, Entity))
			continue;

		DrawModel(Entity);

		if (isPlayer) {
			if (Vars::Chams::Players::Wearables.Value)
			{
				CBaseEntity* pAttachment = Entity->FirstMoveChild();

				for (int n = 0; n < 32; n++)
				{
					if (!pAttachment)
						continue;

					if (pAttachment->IsWearable())
						DrawModel(pAttachment);

					pAttachment = pAttachment->NextMovePeer();
				}
			}

			if (Vars::Chams::Players::Weapons.Value)
			{
				if (const auto& pWeapon = Entity->GetActiveWeapon())
					DrawModel(pWeapon);
			}
		}

		I::ModelRender->ForcedMaterialOverride(nullptr);
		I::RenderView->SetColorModulation(1.0f, 1.0f, 1.0f);

		I::RenderView->SetBlend(1.0f);

		pRenderContext->DepthRange(0.0f, 1.0f);
	}
}