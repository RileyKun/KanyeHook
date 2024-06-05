#include "FakeAng.h"
#include "../../lagcomp/lagcomp.h"
#include "../../../Hooks/HookManager.h"
#include "../../Visuals/chams/Chams.h"
#include "./../../Glow/Glow.h"
#include "../../../Hooks/Hooks.h"
#include "../../Visuals/chams/dme.h"


void Draw(void* ecx, void* edx, CBaseEntity* pEntity, const DrawModelState_t& pState, const ModelRenderInfo_t& pInfo, matrix3x4* pBoneToWorld);
void DrawBT(void* ecx, void* edx, CBaseEntity* pEntity, const DrawModelState_t& pState, const ModelRenderInfo_t& pInfo, matrix3x4* pBoneToWorld);
void DrawFakeAngles(void* ecx, void* edx, const CBaseEntity* pEntity, const DrawModelState_t& pState, const ModelRenderInfo_t& pInfo);
#include "../../Backtrack/Backtrack.h"

MAKE_HOOK(ModelRender_DrawModelExecute, Utils::GetVFuncPtr(I::ModelRender, 19), void, __fastcall,
	void* ecx, void* edx, const DrawModelState_t& pState, const ModelRenderInfo_t& pInfo, matrix3x4* pBoneToWorld)
{
	CBaseEntity* pEntity = I::ClientEntityList->GetClientEntity(pInfo.m_nEntIndex);

	DrawBT(ecx, edx, pEntity, pState, pInfo, pBoneToWorld);

	if (!F::Glow.m_bRendering) {
		if (F::DMEChams.Render(pState, pInfo, pBoneToWorld)) { return; }
	}

	Hook.Original<FN>()(ecx, edx, pState, pInfo, pBoneToWorld);
}


void DrawBT(void* ecx, void* edx, CBaseEntity* pEntity, const DrawModelState_t& pState, const ModelRenderInfo_t& pInfo,
	matrix3x4* pBoneToWorld)
{
	if (!Vars::Chams::Main::Active.Value && !Vars::Backtrack::Enabled.Value) { return; }

	auto OriginalFn = Hooks::ModelRender_DrawModelExecute::Hook.Original<Hooks::ModelRender_DrawModelExecute::FN>();

	if (Vars::Backtrack::Enabled.Value)
	{
		if (pEntity && pEntity->GetClassID() == ETFClassID::CTFPlayer)
		{
			if (!pEntity->IsAlive())
			{
				return;
			}

			if (pEntity->m_vecVelocity().Length2D() < 0.5f)
			{
				return;
			}

			if (!F::Glow.m_bRendering && !F::Chams.m_bRendering)
			{
		
				IMaterial* chosenMat = F::DMEChams.v_MatList.at(4) ? F::DMEChams.v_MatList.at(4) : nullptr;

				I::ModelRender->ForcedMaterialOverride(chosenMat);

				if (chosenMat)
				{
					I::RenderView->SetColorModulation(
						Color::TOFLOAT(Vars::Backtrack::BtChams::BacktrackColor.r),
						Color::TOFLOAT(Vars::Backtrack::BtChams::BacktrackColor.g),
						Color::TOFLOAT(Vars::Backtrack::BtChams::BacktrackColor.b));
				}


				if (const auto& pRenderContext = I::MaterialSystem->GetRenderContext())
				{
					pRenderContext->DepthRange(0.0f, 0.2f);
				}

				I::RenderView->SetBlend(Color::TOFLOAT(255));

				const auto& lastRecord = F::Backtrack.GetRecord(pEntity->GetIndex(), BacktrackMode::Last);
		
				if (lastRecord)
				{
					OriginalFn(ecx, edx, pState, pInfo, (matrix3x4*)(&lastRecord->BoneMatrix));
				}


				I::ModelRender->ForcedMaterialOverride(nullptr);
				I::RenderView->SetColorModulation(1.0f, 1.0f, 1.0f);
				I::RenderView->SetBlend(1.0f);

				if (const auto& pRenderContext = I::MaterialSystem->GetRenderContext())
				{
					pRenderContext->DepthRange(0.0f, 1.0f);
				}
			}
		}
	}
}


void CFakeAng::DrawFake(CBaseEntity* pEntity, Color_t colourface, Color_t colouredge, float time)
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
		Math::ConcatTransforms((&F::FakeAng.BoneMatrix)->BoneMatrix[bbox->bone], rotMatrix, matrix);

		Vec3 bboxAngle;
		Math::MatrixAngles(matrix, bboxAngle);

		Vec3 matrixOrigin;
		Math::GetMatrixOrigin(matrix, matrixOrigin);

		I::DebugOverlay->AddBoxOverlay2(matrixOrigin, bbox->bbmin, bbox->bbmax, bboxAngle, colourface, colouredge, time);
	}
}

void CFakeAng::Run()
{
	if (const auto& pLocal = g_EntityCache.GetLocal())
	{
		if (G::Choking)
		{
			return;
		}

		if (pLocal->IsAlive() && !pLocal->IsAGhost())
		{
			if (const auto& pAnimState = pLocal->GetAnimState()) // doesn't work with certain cosmetics equipped
			{
				float pitch = 0;
				float flOldFrameTime = I::GlobalVars->frametime;
				int nOldSequence = pLocal->m_nSequence();
				auto pOldPoseParams = pLocal->GetPoseParam();
				char pOldAnimState[sizeof(CTFPlayerAnimState)] = {};
				memcpy(pOldAnimState, pAnimState, sizeof(CTFPlayerAnimState));

		


				pAnimState->m_flCurrentFeetYaw = pLocal->GetEyeAngles().y;
				pAnimState->m_flGoalFeetYaw = pLocal->GetEyeAngles().y;
				pAnimState->Update(pLocal->GetEyeAngles().y, pLocal->GetEyeAngles().x);


				I::GlobalVars->frametime = I::Prediction->m_bEnginePaused ? 0.0f : TICK_INTERVAL;

				matrix3x4 bones[128];
				if (pLocal->SetupBones(bones, 128, BONE_FIXED_ALIGNMENT, I::Prediction->m_bEnginePaused ? 0.0f : TICK_INTERVAL))
				{
					BoneMatrix = *reinterpret_cast<FakeMatrixes*>(bones);
				}

				pLocal->m_nSequence() = nOldSequence;
				pLocal->SetPoseParam(pOldPoseParams);
				memcpy(pAnimState, pOldAnimState, sizeof(CTFPlayerAnimState));
				I::GlobalVars->frametime = flOldFrameTime;
			}
		}
	}
}


