#include "../Hooks.h"
#include "../../Features/seedprediction/seed.hpp"

MAKE_HOOK(CL_Move, g_Pattern.Find(L"engine.dll", L"55 8B EC 83 EC 38 83 3D ? ? ? ? ? 0F 8C ? ? ? ? E8 ? ? ? ? 84 C0 0F 84 ? ? ? ? 8B 0D ? ? ? ? 56 33 F6 57 33 FF 89 75 EC"), void, __cdecl,
	float accumulated_extra_samples, bool bFinalTick)
{
	static auto oClMove = Hook.Original<FN>();
	const auto pLocal = g_EntityCache.GetLocal();

	static KeyHelper tpKey{ &Vars::Misc::CL_Move::TeleportKey.Value };
	static KeyHelper rechargeKey{ &Vars::Misc::CL_Move::RechargeKey.Value };

	if (!Vars::Misc::CL_Move::Enabled.Value)
	{
		G::ShiftedTicks = 0;
		return oClMove(accumulated_extra_samples, bFinalTick);

	}


	if (tpKey.Down() && Vars::Misc::CL_Move::SEnabled.Value)
	{
		int wishSpeed = Vars::Misc::CL_Move::DTTicks.Value;
		int speed = 0;
		while (speed < wishSpeed)
		{
			G::Choking = false;
			oClMove(accumulated_extra_samples, bFinalTick);
			speed++;
		}
	}

	if ((tpKey.Down()) && G::ShiftedTicks > 0 && !G::Recharging && !G::RechargeQueued)
	{
		oClMove(accumulated_extra_samples, bFinalTick);

		int wishSpeed = Vars::AntiHack::Resolver::Resolver.Value ? Vars::Misc::CL_Move::DTs.Value : G::EyeAngDelay;
		int speed = 0;
		while (speed < wishSpeed && G::ShiftedTicks)
		{
			G::Choking = false;
			G::ShiftedTicks--;
			oClMove(accumulated_extra_samples, false);
			speed++;
		}
		return;
	}

	if (G::RechargeQueued)
	{
		G::RechargeQueued = false; // see relevant code @clientmodehook
		G::Recharging = true;
		G::TickShiftQueue = 0;
	}
	else if (G::Recharging && (G::ShiftedTicks < 22))
	{
		for (int i = 0; i < 22; ++i)
		{
			G::ForceSendPacket = true; // force uninterrupted connection with server
			G::ShiftedTicks++; // add ticks to tick counter
			return; // this recharges
		}
	}
	else if (rechargeKey.Down() && !G::RechargeQueued && (G::ShiftedTicks < 22))
	{
		// queue recharge
		G::ForceSendPacket = true;
		G::RechargeQueued = true;
	}
	else
	{
		G::Recharging = false;
	}

	oClMove(accumulated_extra_samples, bFinalTick);

	if (!pLocal)
	{
		G::ShiftedTicks = 0; // we do not have charge if we do not exist
		return oClMove(accumulated_extra_samples, bFinalTick);
	}

	if (G::WaitForShift)
	{
		G::WaitForShift--;
		return;
	}

	if (G::IsAttacking && G::ShiftedTicks > 21)
	{
		G::ShouldShift = true;
	}

	if (G::ShouldShift && !G::WaitForShift)
	{
		for (int i = 2; i < 22; ++i)
		{
			G::DT = true;
		    oClMove(accumulated_extra_samples, bFinalTick);
			G::ShiftedTicks--;
			G::DT = false;
		}
		G::ShouldShift = false;
	}
}