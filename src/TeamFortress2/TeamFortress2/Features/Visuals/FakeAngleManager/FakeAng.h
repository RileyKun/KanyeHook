#pragma once
#include "../../../SDK/SDK.h"

#pragma warning ( disable : 4091 )


typedef struct FakeMatrixes
{
	float BoneMatrix[128][3][4];
};


class CFakeAng {
public:
	FakeMatrixes BoneMatrix;
	void Run();
	void AnimFix();
	bool DrawChams = false;
	void DrawFake(CBaseEntity* pEntity, Color_t colourface, Color_t colouredge, float time);
};

ADD_FEATURE(CFakeAng, FakeAng)