#include "../Hooks.h"

#include "../../Features/Prediction/Prediction.h"
#include "../../Features/Aimbot/Aimbot.h"
#include "../../Features/lagcomp/lagcomp.h"
#include "../../Features/Auto/Auto.h"
#include "../../Features/Aimbot/AimbotHitscan/AimbotHitscan.h"
#include "../../Features/Misc/Misc.h"
#include "../../Features/Visuals/Visuals.h"
#include "../../Features/AntiHack/AntiAim.h"
#include "../../Features/AntiHack/FakeLag/FakeLag.h"
#include "../../Features/Backtrack/Backtrack.h"
#include "../../Features/Visuals/FakeAngleManager/FakeAng.h"
#include "../../Features/CritHack/CritHack.h"
#include "../../Features/Vars.h"
#include "../../Features/AntiHack/Resolver.h"
#include "../../Features/seedprediction/seed.hpp"
#include "../../Features/Aimbot/ProjectileAim/ProjSim.h"



MAKE_HOOK(C_TFRagdoll_CreateTFRagdoll, g_Pattern.Find(L"client.dll", L"55 8B EC B8 ? ? ? ? E8 ? ? ? ? 53 56 57 8B F9 8B 8F"), void, __fastcall, void* ecx, void* edx)
{
	return; 
}


MAKE_HOOK(ClientModeShared_CreateMove, Utils::GetVFuncPtr(I::ClientModeShared, 21), bool, __fastcall,
	void* ecx, void* edx, float input_sample_frametime, CUserCmd* pCmd)
{

	G::LastUserCmd = pCmd;

	G::SilentTime = false;
	G::IsAttacking = false;
	G::FakeShotPitch = false;

	if (!pCmd || !pCmd->command_number)
	{
		return Hook.Original<FN>()(ecx, edx, input_sample_frametime, pCmd);
	}
	// Get the pointer to pSendPacket
	uintptr_t _bp;
	__asm mov _bp, ebp;
	auto pSendPacket = reinterpret_cast<bool*>(***reinterpret_cast<uintptr_t***>(_bp) - 0x1);

	int nOldFlags = 0;
	int nOldGroundEnt = 0;
	Vec3 vOldAngles = pCmd->viewangles;
	float fOldSide = pCmd->sidemove;
	float fOldForward = pCmd->forwardmove;

	if (const auto& pLocal = g_EntityCache.GetLocal())
	{
		nOldFlags = pLocal->GetFlags();
		nOldGroundEnt = pLocal->m_hGroundEntity();

		if (const int MaxSpeed = pLocal->GetMaxSpeed()) {
			G::Frozen = MaxSpeed == 1;
		}

	
		if (const auto& pWeapon = g_EntityCache.GetWeapon())
		{
			const int nItemDefIndex = pWeapon->GetItemDefIndex();

			G::CurItemDefIndex = nItemDefIndex;
			G::WeaponCanHeadShot = pWeapon->CanWeaponHeadShot();
			G::WeaponCanAttack = pWeapon->CanShoot(pCmd, pLocal);
			G::WeaponCanSecondaryAttack = pWeapon->CanSecondaryAttack(pLocal);
			G::CurWeaponType = Utils::GetWeaponType(pWeapon);
			G::IsAttacking = Utils::IsAttacking(pCmd, pWeapon);
			G::faggot = (G::CurItemDefIndex != nItemDefIndex || !pWeapon->GetClip1() || !pWeapon->GetClip2() || !G::WeaponCanAttack || (!pLocal->IsAlive() || pLocal->IsTaunting() || pLocal->IsBonked() || pLocal->GetFeignDeathReady() || pLocal->IsCloaked() || pLocal->IsInBumperKart() || pLocal->IsAGhost()));

			if (pWeapon->GetSlot() != SLOT_MELEE)
			{
				if (pWeapon->IsInReload())
				{
					G::WeaponCanAttack = true;
				}

				if (G::CurItemDefIndex != Soldier_m_TheBeggarsBazooka)
				{
					if (pWeapon->GetClip1() == 0)
					{
						G::WeaponCanAttack = false;
					}
				}
			}
		}
	}



	{
		F::Misc.Run(pCmd);
		F::EnginePrediction.Start(pCmd);
		{
			F::Aimbot.Run(pCmd);
			F::Auto.Run(pCmd);
			{
			
			}
		}
		F::EnginePrediction.End(pCmd);
		F::NS.Correction(pCmd);
		F::CritHack.Run(pCmd);
		F::AntiAim.Run(pCmd, pSendPacket);
		F::FakeLag.OnTick(pCmd, pSendPacket);
		F::Resolver.Update(pCmd);
		F::Misc.RunLate(pCmd);
	}


	G::ViewAngles = pCmd->viewangles;

	G::FakeAngles.push_back(pCmd->viewangles);

	static int nChoked = G::Recharging;

	if (G::ShouldShift)
	{
		if (!*pSendPacket)
		{
			nChoked++;
		}
		else
		{
			nChoked = 0;
		}

		if (nChoked > 24)
		{
			*pSendPacket = true;
		}

	}

	auto& out = G::commands.emplace_back();

	out.is_outgoing = pSendPacket;
	out.is_used = false;
	out.command_number = pCmd->command_number;
	out.previous_command_number = 0;

	while (G::commands.size() > I::GlobalVars->interval_per_tick)
		G::commands.pop_front();

	if (!pSendPacket)
	{
		auto net_channel = I::ClientState->m_NetChannel;

		if (net_channel)
		{
			if (net_channel->m_nChokedPackets > 0 && !(net_channel->m_nChokedPackets % 4))
			{
				auto backup_choke = net_channel->m_nChokedPackets;
				net_channel->m_nChokedPackets = 0;

				net_channel->SendDatagram(0);
				--net_channel->m_nOutSequenceNr;

				net_channel->m_nChokedPackets = backup_choke;
			}
		}
	}

	static bool bWasSet = false;
	if (G::SilentTime)
	{
		*pSendPacket = false;
		bWasSet = true;
	}
	else
	{
		if (bWasSet)
		{
			*pSendPacket = true;
			pCmd->viewangles = vOldAngles;
			pCmd->sidemove = fOldSide;
			pCmd->forwardmove = fOldForward;
			bWasSet = false;
		}
	}



	G::Choking = !*pSendPacket;
	G::ChokedAngles.push_back(pCmd->viewangles);

	if (G::ForceSendPacket)
	{
		*pSendPacket = true;
		G::ForceSendPacket = false;
	} // if we are trying to force update do this lol


	// Stop movement if required
	if (G::ShouldStop)
	{
		G::ShouldStop = false;
		Utils::StopMovement(pCmd, !G::ShouldShift);
		return false;
	}

	if (G::SilentTime ||
		G::AAActive ||
		G::FakeShotPitch ||
		G::HitscanSilentActive ||
		G::AvoidingBackstab ||
		G::ProjectileSilentActive ||
		G::RollExploiting)
	{
		return false;
	}

	return false;
}