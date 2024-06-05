#pragma once
#include "../../../SDK/SDK.h"

class CFakeLag {


public:

	bool IsAllowed(CBaseEntity* pLocal);
	void OnTick(CUserCmd* pCmd, bool* pSendPacket);
	int ChokeCounter = 0; // How many ticks have been choked
	int ChosenAmount = 0; // How many ticks should be choked
};

ADD_FEATURE(CFakeLag, FakeLag)
