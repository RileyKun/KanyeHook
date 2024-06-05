#include "../../SDK/SDK.h"

class CNS {
public:
	bool synced{ false };
	float  server_time{ 0.0f };
	float  prev_server_time{ 0.0f };
	float ask_time{ 0.0f };
	float  guess_time{ 0.0f };
	float  sync_offset{ 0.0f };
	bool waiting_for_pp{ false };
	float  guess_delta{ 0.0f };
	float response_time{ 0.0f };
	void askForPlayerPerf();
	bool parsePlayerPerf(bf_read& msg_data);
	void reset();
	int getSeed();
	void Correction(CUserCmd* cmd);
};

ADD_FEATURE(CNS, NS)
