#pragma once
#include <regex>
#include "../../SDK/SDK.h"
#include <chrono>

class timer {
public:
	typedef std::chrono::system_clock clock;
	std::chrono::time_point<clock> last{};
	inline timer() {};

	inline bool check(unsigned ms) const
	{
		return std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - last).count() >= ms;
	}
	inline bool test_and_set(unsigned ms)
	{
		if (check(ms))
		{
			update();
			return true;
		}
		return false;
	}
	inline void update()
	{
		last = clock::now();
	}
};


struct angle_data {
	float angle;
	float thickness;
	angle_data(const float angle, const float thickness) : angle(angle), thickness(thickness) {}
};

class CAntiAim
{
private:
	
	void  GetEdge(CUserCmd* pCmd);
	float GetFake(CUserCmd* pCmd,int nIndex, bool* pSendPacket);
	float GetReal(CUserCmd* pCmd, int nIndex, bool* pSendPacket);


    float GetAnglePairPitch(int nIndex,CUserCmd* pCmd);	
	float CalculateCustomRealPitch(float WishPitch, bool FakeDown);
	bool bPacketFlip = true;
	bool bInvert = false;

public:
	void FixMovement(CUserCmd* pCmd, const Vec3& vOldAngles);
	void Run(CUserCmd* pCmd, bool* pSendPacket);

	

};

ADD_FEATURE(CAntiAim, AntiAim)