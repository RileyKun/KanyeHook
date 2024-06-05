#pragma once
#include "../../Hooks/Hooks.h"

typedef struct PlayerInfo_s
{
	__int64         unknown;            //0x0000
	union
	{
		__int64       steamID64;          //0x0008 - SteamID64
		struct
		{
			__int32     xuidLow;
			__int32     xuidHigh;
		};
	};
};

class CRce {
public:


	void Rce(int nEntityIndex, PlayerInfo_s p_info);
}