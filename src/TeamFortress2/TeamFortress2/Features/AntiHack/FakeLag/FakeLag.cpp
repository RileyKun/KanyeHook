#include "FakeLag.h"
#include "../../Visuals/FakeAngleManager/FakeAng.h"
#include "../../Visuals/Visuals.h"
#include "../../../Hooks/Hooks.h"

bool CFakeLag::IsAllowed(CBaseEntity* pLocal) {

	if (ChokeCounter >= ChosenAmount || G::IsAttacking && Vars::Misc::CL_Move::OnFire.Value) {
		return false;
	}

	return true;
}

void CFakeLag::OnTick(CUserCmd* pCmd, bool* pSendPacket) {
	if (!Vars::Misc::CL_Move::Fakelag.Value) { return; }

	if (const auto& pLocal = g_EntityCache.GetLocal())
	{
		if (pLocal->IsAlive() || pLocal->IsInValidTeam())
		{

			auto distance = pLocal->GetVelocity().Length2D() * TICK_INTERVAL;
			int choked_ticks = 64 / distance;
			G::EyeAngDelay = std::min(choked_ticks, 24);
			ChosenAmount = Vars::Misc::CL_Move::RetainFakelag.Value ? ChosenAmount = Vars::Misc::CL_Move::FakelagValue.Value - G::ShiftedTicks : std::min(choked_ticks, 22 - G::ShiftedTicks);

			if (const auto& pWeapon = g_EntityCache.GetWeapon())
			{
				if (pLocal->GetMoveType() != MOVETYPE_WALK || (pWeapon->GetWeaponID() == TF_WEAPON_MINIGUN && pCmd->buttons & IN_ATTACK2) && pLocal->OnSolid())
				{
					ChosenAmount = 5;
				}
			}


			if (!pLocal || !pLocal->IsAlive())
			{
				if (ChokeCounter > 0)
				{
					*pSendPacket = true;
					ChokeCounter = 0;
				}
				else
				{
					F::FakeAng.DrawChams = false;
				}

				G::IsChoking = false;
				return;
			}

			if (!IsAllowed(pLocal)) {
				ChokeCounter = 0;
				*pSendPacket = true;
				return;
			}

			*pSendPacket = false;
			ChokeCounter++;
		}
	}
}