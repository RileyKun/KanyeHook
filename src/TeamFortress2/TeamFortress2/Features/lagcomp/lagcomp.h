#pragma once
#include "../../SDK/SDK.h"

#pragma warning ( disable : 4091 )

typedef struct FakeMatrixesr {
	float BoneMatrixr[128][3][4];
};



class CLagComp {
public:
	FakeMatrixesr BoneMatrixr;
	

void run(int iIndex, const Vec3 vAngles, const Vec3 vecOrigin);
private:

};

ADD_FEATURE(CLagComp, LagComp)
