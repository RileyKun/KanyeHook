#pragma once
#include "../../SDK/SDK.h"
#pragma warning ( disable : 4091 )

typedef struct FakMatrixes {
	float BonMatrix[128][3][4];
};

class CESP
{
private:
	static bool ShouldRun();
	
	static const wchar_t* GetPlayerClass(int nClassNum);
	struct Predictor_t {
		CBaseEntity* m_pEntity = nullptr;
		Vec3 m_vPosition = {};
		Vec3 m_vVelocity = {};
		Vec3 m_vAcceleration = {};

		Vec3 Extrapolate(float time);
	};

	void DrawPlayers(CBaseEntity* pLocal);
	void DrawBuildings(CBaseEntity* pLocal) const;
	void DrawWorld() const;
	std::wstring ConvertUtf8ToWide(const std::string& str);
	static void Draw3DBox(const Vec3* vPoints, Color_t clr);
	FakMatrixes BonMatrix;
	static void CreateDLight(int nIndex, Color_t DrawColor, const Vec3& vOrigin, float flRadius);
	static void DrawBones(CBaseEntity* pPlayer, const std::vector<int>& vecBones, Color_t clr);

public:
	void Run();
	static bool GetDrawBounds(CBaseEntity* pEntity, Vec3* vTrans, int& x, int& y, int& w, int& h);
};

ADD_FEATURE(CESP, ESP)