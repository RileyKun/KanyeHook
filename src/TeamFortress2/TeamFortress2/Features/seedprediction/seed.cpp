#include "seed.hpp"

#include <regex>


void CNS::reset()
{
	synced = false;
	server_time = 0.0f;
	prev_server_time = 0.0f;
	ask_time = 0.0f;
	guess_time = 0.0f;
	sync_offset = 0.0f;
	waiting_for_pp = false;
	guess_delta = 0.0f;
	response_time = 0.0f;
}

void CNS::askForPlayerPerf()
{

	CBaseCombatWeapon* weapon{ g_EntityCache.GetWeapon() };

	if (!weapon || !(Utils::GetWeaponType(weapon) == EWeaponType::HITSCAN))
	{
		reset();

		return;
	}

	if (CBaseEntity* local{ g_EntityCache.GetLocal() })
	{
		if (local->deadflag())
		{
			reset();

			return;
		}
	}

	if (waiting_for_pp)
	{
		return;
	}

	I::EngineClient->ClientCmd_Unrestricted("playerperf");

	ask_time = static_cast<float>(Utils::plat_float_time());

	waiting_for_pp = true;
}

float test = { 0 };

bool CNS::parsePlayerPerf(bf_read& msg_data)
{


	char raw_msg[256]{};

	msg_data.ReadString(raw_msg, sizeof(raw_msg), true);
	msg_data.Seek(0);

	std::string msg(raw_msg);

	msg.erase(msg.begin()); //STX

	std::smatch matches{};

	std::regex_match(msg, matches, std::regex("(\\d+.\\d+)\\s\\d+\\s\\d+\\s\\d+.\\d+\\s\\d+.\\d+\\svel\\s\\d+.\\d+"));

	if (matches.size() == 2)
	{
		waiting_for_pp = false;

		//credits to kgb for idea

		double new_server_time{ std::stof(matches[1].str()) };

		if (new_server_time > server_time)
		{
			prev_server_time = server_time;

			server_time = new_server_time;

			response_time = static_cast<float>(Utils::plat_float_time() - ask_time);

			//if (!synced)
			{
				if (prev_server_time > 0.0f)
				{
					if (guess_time > 0.0f)
					{
						double delta{ server_time - guess_time };

						if (delta == 0.0f)
						{
							synced = true;

							sync_offset = guess_delta;
						}
					}

					guess_delta = server_time - prev_server_time;

					guess_time = server_time + (guess_delta);
				}
			}
		}

		return true;
	}

	else
	{
		return std::regex_match(msg, std::regex("\\d+.\\d+\\s\\d+\\s\\d+"));
	}

	return false;
}

int CNS::getSeed()
{
	double time{ (guess_time + sync_offset + response_time) * 1000.0f };

	return *reinterpret_cast<int*>((char*)&time) & 255;
}


void CNS::Correction(CUserCmd* pCmd)
{

	static double s_flPlatFloatTimeBeginUptime = Utils::plat_float_time();

	if (!pCmd || !G::IsAttacking)
	{
		return;
	}

	auto local{ g_EntityCache.GetLocal() };

	if (!local)
	{
		return;
	}

	auto weapon{ g_EntityCache.GetWeapon() };

	if (!weapon || !(Utils::GetWeaponType(weapon) == EWeaponType::HITSCAN))
	{
		return;
	}

	auto spread{ weapon->GetTFWeaponInfo()->GetWeaponData(0).m_flSpread };
	

	if (spread <= 0.0f)
	{
		return;
	}

	auto bullets_per_shot{ weapon->GetTFWeaponInfo()->GetWeaponData(0).m_nBulletsPerShot };

	bullets_per_shot = static_cast<int>(Utils::ATTRIB_HOOK_FLOAT(static_cast<float>(bullets_per_shot), "mult_bullets_per_shot", weapon,0x0, true));

	//credits to cathook for average spread stuff

	std::vector<Vec3> bullet_corrections{};

	Vec3 average_spread{};
	
	auto delta = (server_time) - (server_time);
	//server_time
	float time = float(server_time * 1000);
	int p = *reinterpret_cast<int*>((char*)&time);

	auto seed{ (p) & 255 }; //test

	for (auto bullet{ 0 }; bullet < bullets_per_shot; bullet++)
	{
		Utils::RandomSeed(seed); 

		I::DebugOverlay->ClearAllOverlays();
		I::DebugOverlay->AddTextOverlay(local->GetWorldSpaceCenter(), 0.1, std::format("{}\n{}\n{}\n{}\n", delta, guess_time, server_time, Utils::plat_float_time()).c_str());
		auto fire_perfect{ false };

		if (bullet == 0)
		{
			auto time_since_last_shot{ (local->m_nTickBase() * TICK_INTERVAL) - weapon->GetLastFireTime() };

			if (bullets_per_shot > 1 && time_since_last_shot > 0.25f)
			{
				fire_perfect = true;
			}

			else if (bullets_per_shot == 1 && time_since_last_shot > 1.25f)
			{
				fire_perfect = true;
			}
		}

		if (fire_perfect)
		{
			return;
		}

		auto x{ Utils::RandomFloat(-0.5f, 0.5f) + Utils::RandomFloat(-0.5f, 0.5f) };
		auto y{ Utils::RandomFloat(-0.5f, 0.5f) + Utils::RandomFloat(-0.5f, 0.5f) };

		Vec3 forward{}, right{}, up{};

		Math::AngleVectors(pCmd->viewangles, &forward, &right, &up);

		Vector fixed_spread{ forward + (right * x * spread) + (up * y * spread) };

		fixed_spread.NormalizeInPlace();

		average_spread += fixed_spread;

		bullet_corrections.push_back(fixed_spread);
	}

	average_spread /= static_cast<float>(bullets_per_shot);

	Vec3 fixed_spread{ FLT_MAX, FLT_MAX, FLT_MAX };

	for (const auto& spread : bullet_corrections)
	{
		if (spread.DistTo(average_spread) < fixed_spread.DistTo(average_spread))
		{
			fixed_spread = spread;
		}
	}

	Vec3 fixed_angles{};

	Math::VectorAngles(fixed_spread, fixed_angles);

	Vec3 correction{ pCmd->viewangles - fixed_angles };

	pCmd->viewangles += correction;

	Math::ClampAngles(pCmd->viewangles);
}

