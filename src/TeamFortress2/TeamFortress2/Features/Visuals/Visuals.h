#pragma once
#include "../../SDK/SDK.h"

#pragma warning ( disable : 4091 )


class CVisuals
{
private:
	int m_nHudZoom = 0;
	int m_nHudMotd = 0;
public:
	bool RemoveScope(int nPanel);

	void FOV(CViewSetup* pView);

	void ThirdPerson(CViewSetup* pView);
	
	void DrawHitboxMatrix(CBaseEntity* pEntity, Color_t colourface, Color_t colouredge, float time);

	void DrawAimbotFOV(CBaseEntity* pLocal);

	void DrawTickbaseInfo(CBaseEntity* pLocal);

	void DrawMovesimLine();

	void RenderLine(const Vector& v1, const Vector& v2, Color_t c, bool bZBuffer);
};

ADD_FEATURE(CVisuals, Visuals)