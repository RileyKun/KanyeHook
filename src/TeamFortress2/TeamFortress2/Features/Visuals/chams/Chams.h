#pragma once
#include "../../../SDK/SDK.h"

class CChams
{
private:
	IMaterial* m_pMatFlat;
	IMaterial* m_pMatBlur;

	IMaterialVar *m_pMatFresnelHDRSelfillumTint, *m_pMatFresnelHDREnvmapTint;

	std::unordered_map<CBaseEntity*, bool> m_DrawnEntities;

	
private:
	bool ShouldRun();
	void DrawModel(CBaseEntity* pEntity);

private:
	void RenderEnts(CBaseEntity* pLocal, IMatRenderContext* pRenderContext);

public:
	void Init();
	void Render();
	bool HasDrawn(CBaseEntity* pEntity)
	{
		return m_DrawnEntities.find(pEntity) != m_DrawnEntities.end();
	}

	bool IsChamsMaterial(IMaterial* pMat)
	{
		return pMat == m_pMatFlat;
	}

public:
	bool m_bHasSetStencil;
	bool m_bRendering;
};

ADD_FEATURE(CChams, Chams)