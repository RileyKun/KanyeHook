#include "../../Hooks/Hooks.h"
#include "CritHack.h"

#define MASK_SIGNED 0x7FFFFFFF

// i hate crithack

/* Returns whether random crits are enabled on the server */
bool CCritHack::AreRandomCritsEnabled()
{
	if (static auto tf_weapon_criticals = g_ConVars.FindVar("tf_weapon_criticals"); tf_weapon_criticals)
	{
		return tf_weapon_criticals->GetBool();
	}
	return true;
}

/* Returns whether the crithack should run */
bool CCritHack::IsEnabled()
{
	if (!Vars::CritHack::Active.Value) { return false; }
	if (!AreRandomCritsEnabled()) { return false; }
	if (!I::EngineClient->IsInGame()) { return false; }

	return true;
}


bool CCritHack::IsAttacking(const CUserCmd* pCmd, CBaseCombatWeapon* pWeapon)
{
	if (pWeapon->GetSlot() == EWeaponSlots::SLOT_MELEE)
	{
		if (pWeapon->GetWeaponID() == TF_WEAPON_KNIFE)
			return ((pCmd->buttons & IN_ATTACK) && G::WeaponCanAttack);

		else return fabs(pWeapon->GetSmackTime() - I::GlobalVars->curtime) < I::GlobalVars->interval_per_tick * 2.0f;
	}
	else
	{
		if (G::CurItemDefIndex == Soldier_m_TheBeggarsBazooka)
		{
			static bool bLoading = false;

			if (pWeapon->GetClip1() > 0)
				bLoading = true;

			if (!(pCmd->buttons & IN_ATTACK) && bLoading) {
				bLoading = false;
				return true;
			}
		}

		else
		{
			int ID = pWeapon->GetWeaponID();
			switch (ID) {
			case TF_WEAPON_COMPOUND_BOW:
			case TF_WEAPON_PIPEBOMBLAUNCHER:
			{
				static bool bCharging = false;

				if (pWeapon->GetChargeBeginTime() > 0.0f)
					bCharging = true;

				if (!(pCmd->buttons & IN_ATTACK) && bCharging) {
					bCharging = false;
					return true;
				}
				break;
			}
			case TF_WEAPON_CANNON:
			{
				static bool bCharging = false;

				if (pWeapon->GetDetonateTime() > 0.0f)
					bCharging = true;

				if (!(pCmd->buttons & IN_ATTACK) && bCharging) {
					bCharging = false;
					return true;
				}
				break;
			}
			//ig below you can remove even tho you can crit hack with them..
			case TF_WEAPON_JAR:
			case TF_WEAPON_JAR_MILK:
			case TF_WEAPON_JAR_GAS:
			case TF_WEAPON_GRENADE_JAR_GAS:
			{
				static float flThrowTime = 0.0f;

				if ((pCmd->buttons & IN_ATTACK) && G::WeaponCanAttack && !flThrowTime)
					flThrowTime = I::GlobalVars->curtime + I::GlobalVars->interval_per_tick;

				if (flThrowTime && I::GlobalVars->curtime >= flThrowTime) {
					flThrowTime = 0.0f;
					return true;
				}
				break;
			}
			case TF_WEAPON_MINIGUN:
			{

				if (pWeapon->GetMinigunState() == 2)
				{
					return true;
				}
				break;
			}
			default:
			{
				if ((pCmd->buttons & IN_ATTACK) && G::WeaponCanAttack)
				{
					return true;
				}
				break;
			}
			}
		}
	}

	return false;
}

bool CCritHack::NoRandomCrits(CBaseCombatWeapon* pWeapon)
{
	float CritChance = Utils::ATTRIB_HOOK_FLOAT(1, "mult_crit_chance", pWeapon, 0, 1);
	if (CritChance == 0)
	{
		return true;
	}
	else
		return false;
	//list of weapons that cant random crit, but dont have the attribute for it
	switch (pWeapon->GetWeaponID())
	{
	case TF_WEAPON_BUFF_ITEM: // soldier
	case TF_WEAPON_ROCKETPACK: //pyro
	case TF_WEAPON_PARACHUTE: //soldier
	case TF_WEAPON_LUNCHBOX: //pootis
	case TF_WEAPON_PDA_ENGINEER_BUILD://shortgnome
	case TF_WEAPON_PDA_ENGINEER_DESTROY://shortgnome
	case TF_WEAPON_LASER_POINTER://shortgnome
	case TF_WEAPON_MEDIGUN://medic
	case TF_WEAPON_SNIPERRIFLE://sniper
	case TF_WEAPON_SNIPERRIFLE_CLASSIC://sniper
	case TF_WEAPON_SNIPERRIFLE_DECAP://sniper
	case TF_WEAPON_COMPOUND_BOW://sniper
	case TF_WEAPON_KNIFE://spy
	case TF_WEAPON_PDA_SPY_BUILD://spy
	case TF_WEAPON_PDA_SPY://spy
	case TF_WEAPON_BUILDER: //shortgnome
	case Pyro_m_ThePhlogistinator://pyro
	case TF_WEAPON_INVIS://soldier
	case Scout_m_TheBackScatter://scout
	case TF_WEAPON_CLEAVER://scout
	case Soldier_m_TheCowMangler5000://soldier
	case Soldier_m_RocketJumper://soldier
	case Demoman_s_StickyJumper://demoman
	case Soldier_t_TheHalfZatoichi://soldier
	case Soldier_t_TheMarketGardener://soldier
	case Pyro_s_TheManmelter://pyro
	case TF_WEAPON_FIREAXE://pyro
	case TF_WEAPON_SWORD://demoman
	case Demoman_t_UllapoolCaber://demoman
	case Engi_m_FestiveFrontierJustice://idk why would you get it engineer
	case Engi_m_TheFrontierJustice://engineer
	case Engi_s_TheShortCircuit://engineer
	case Engi_t_TheGunslinger://engineer
	case Engi_t_TheSouthernHospitality://engineer
	case Sniper_s_TheCleanersCarbine: //sniper
	case Sniper_t_TheBushwacka: //sniper
	case Spy_m_TheEnforcer://spy
	case Spy_m_TheDiamondback://spy
	{
		return true;
		break;
	}
	default: return false; break;
	}

}

bool CCritHack::ShouldCrit()
{
	static KeyHelper critKey{ &Vars::CritHack::CritKey.Value };
	if (critKey.Down()) { return true; }
	if (G::CurWeaponType == EWeaponType::MELEE && Vars::CritHack::AlwaysMelee.Value) { return true; }

	const auto& pLocal = g_EntityCache.GetLocal();

	const auto& pWeapon = pLocal->GetActiveWeapon();

	if (pWeapon->GetSwingRange(pLocal) * 2 && G::CurWeaponType == EWeaponType::MELEE)
	{
		//Base melee damage
		int MeleeDamage = 65;

		//Could be missing wepaons or reskins
		switch (G::CurItemDefIndex)
		{
		case Scout_t_SunonaStick:
		{
			//The Sun on a Stick has a -25% melee damage stat
			MeleeDamage = 26;
			break;
		}
		case Scout_t_TheFanOWar:
		{
			//The Fan O'War has a -75% melee damage stat
			MeleeDamage = 9;
			break;
		}
		case Scout_t_TheWrapAssassin:
		{
			//The Wrap Assassin has a -65% melee damage stat
			MeleeDamage = 12;
			break;
		}
		case Soldier_t_TheDisciplinaryAction:
		case Engi_t_TheJag:
		{
			//The Disciplinary Action and The Jag have a -25% melee damage stat
			MeleeDamage = 49;
			break;
		}
		case Soldier_t_TheEqualizer:
		{
			//The Equalizer does more damage the lower the local player's health is
			break;
		}
		case Pyro_t_HotHand:
		{
			//The Hot Hand has a -20% melee damage stat
			MeleeDamage = 28;
			break;
		}
		case Pyro_t_SharpenedVolcanoFragment:
		case Medic_t_Amputator:
		{
			//The Sharpened Volcano Fragment and The Amputator have a -20% melee damage stat
			MeleeDamage = 52;
			break;
		}
		case Pyro_t_TheBackScratcher:
		{
			//The Back Scratcher has a +25% melee damage stat
			MeleeDamage = 81;
			break;
		}
		case Demoman_t_TheScotsmansSkullcutter:
		{
			//The Scotsmans Skullcutter has a +20% melee damage stat
			MeleeDamage = 78;
			break;
		}
		case Heavy_t_WarriorsSpirit:
		{
			//The Warriors Spirit has a +30% melee damage stat
			MeleeDamage = 85;
			break;
		}
		case Sniper_t_TheTribalmansShiv:
		{
			//The Tribalmans Shiv has a -50% melee damage stat
			MeleeDamage = 37;
			break;
		}
		case Sniper_t_TheShahanshah:
		{
			//The Shahanshah has a -25% melee damage stat when above half health and a +25% when below half health
			//81 if below half health
			//49 if above half health
			break;
		}
		default: break;
		}

		CBaseEntity* Player;
		if ((Player = I::ClientEntityList->GetClientEntity(G::CurrentTargetIdx)))
		{
			if (G::CurItemDefIndex == Heavy_t_TheHolidayPunch)
			{
				if (Player->OnSolid())
					return true;
			}

			if (MeleeDamage <= Player->GetHealth())
				return true;
		}
	}

	return false;
}

int CCritHack::LastGoodCritTick(const CUserCmd* pCmd)
{
	int retVal = -1;
	bool popBack = false;

	for (const auto& cmd : CritTicks)
	{
		if (cmd >= pCmd->command_number)
		{
			retVal = cmd;
		}
		else
		{
			popBack = true;
		}
	}

	if (popBack)
	{
		CritTicks.pop_back();
	}

	return retVal;
}

int CCritHack::LastGoodSkipTick(const CUserCmd* pCmd)
{
	int retVal = -1;
	bool popBack = false;

	for (const auto& cmd : skip)
	{
		if (cmd >= pCmd->command_number)
		{
			retVal = cmd;
		}
		else
		{
			popBack = true;
		}
	}

	if (popBack)
	{
		skip.pop_back();
	}

	return retVal;
}


UINT32 CCritHack::decrypt_or_encrypt_seed(CBaseEntity* local, CBaseCombatWeapon* weapon, UINT32 seed)
{
	if (!weapon)
	{
		return 0;
	}

	unsigned int iMask = weapon->GetIndex() << 8 | local->GetIndex();

	if (weapon->GetSlot() == SLOT_MELEE)
	{
		iMask <<= 8;
	}

	return iMask ^ seed;
}

bool CCritHack::is_pure_crit_command(const INT32 command_number) {

	const auto& local = g_EntityCache.GetLocal();
	const auto& weapon = local->GetActiveWeapon();
	if (!local || !weapon)
	{
		return false;
	}

	{
		float multiplier = critmultx3;
		multiplier *= 100;

		const auto random_seed = MD5_PseudoRandom(command_number) & 0x7FFFFFFF;
		Utils::RandomFloat(decrypt_or_encrypt_seed(local, weapon, random_seed));

		//return math.random_int( 0, 9999 ) < ( 0.01f * 10000 );
		return Utils::RandInt(0, 9999) < multiplier && Utils::RandInt(0, 9999) != 0;
	}

	return false;
}

void CCritHack::ScanForCrits(const CUserCmd* pCmd, int loops)
{
	static int previousWeapon = 0;
	static int previousCrit = 0;
	static int startingNum = pCmd->command_number;

	const auto& pLocal = g_EntityCache.GetLocal();
	if (!pLocal) { return; }

	const auto& pWeapon = pLocal->GetActiveWeapon();
	if (!pWeapon) { return; }

	if (G::IsAttacking)
	{
		return;
	}

	const bool bRescanRequired = previousWeapon != pWeapon->GetIndex();
	if (bRescanRequired)
	{
		startingNum = pCmd->command_number;
		previousWeapon = pWeapon->GetIndex();
		CritTicks.clear();
	}

	if (CritTicks.size() >= 256)
	{
		return;
	}

	const int seedBackup = MD5_PseudoRandom(pCmd->command_number) & MASK_SIGNED;
	for (int i = 0; i < loops; i++)
	{
		const int cmdNum = startingNum + i;
		const bool result = (Utils::RandInt(0, 9999) == 0);
		*I::RandomSeed = MD5_PseudoRandom(cmdNum) & MASK_SIGNED;
		if (pWeapon->WillCrit() || result)
		{
			// Check if the command number is already in CritTicks
			bool isNewCommand = true;
			for (const auto& critTick : CritTicks)
			{
				if (critTick == cmdNum)
				{
					isNewCommand = false;
					break;
				}
			}

			if (isNewCommand)
			{
				CritTicks.push_back(cmdNum); //	store our wish command number for later reference
			}
		}
	}
	startingNum += loops;

	//*reinterpet_cast<float*>(pWeapon + 0xA54) = CritBucketBP;
	*reinterpret_cast<int*>(pWeapon + 0xA5C) = 0; //	dont comment this out, makes sure our crit mult stays as low as possible.
	*I::RandomSeed = seedBackup;
}

std::pair<float, float> CCritHack::critMultInfo(CBaseCombatWeapon* wep)
{
	static auto tf_weapon_criticals_bucket_cap = g_ConVars.FindVar("tf_weapon_criticals_bucket_cap");
	const float bucketCap = tf_weapon_criticals_bucket_cap->GetFloat();

	const auto& pLocal = g_EntityCache.GetLocal();
	const auto& pWeapon = pLocal->GetActiveWeapon();

	float cur_crit = bucketCap;
	float observed_chance = (pWeapon->ObservedCritChance());
	float needed_chance = cur_crit + 0.1f;
	return std::pair<float, float>(observed_chance, needed_chance);

}
int CCritHack::damageUntilToCrit(CBaseCombatWeapon* wep)
{
	const auto& pLocal = g_EntityCache.GetLocal();
	const auto& pWeapon = pLocal->GetActiveWeapon();


	// First check if we even need to deal damage at all
	auto crit_info = critMultInfo(wep);
	if (crit_info.first <= crit_info.second || pWeapon->GetSlot() == 2)
		return 0;

	float target_chance = critMultInfo(wep).second;
	// Formula taken from TotallyNotElite
	int damage = std::ceil(crit_damage * (2.0f * target_chance + 1.0f) / (3.0f * target_chance));
	return damage - (cached_damage - round_damage);
}

float CCritHack::getWithdrawMult(CBaseCombatWeapon* pWeapon)
{
	const auto count = static_cast<float>(*reinterpret_cast<int*>(pWeapon + 0xa5c) + 1);
	const auto checks = static_cast<float>(*reinterpret_cast<int*>(pWeapon + 0xa58) + 1);

	float multiply = 0.5;
	if (pWeapon->GetSlot() != 2) { multiply = Math::RemapValClamped(count / checks, .1f, 1.f, 1.f, 3.f); }

	return multiply * 3.f;
}

float CCritHack::getWithdrawAmount(CBaseCombatWeapon* pWeapon)
{
	float amount = getWithdrawMult(pWeapon);
	if (pWeapon->IsRapidFire()) {
		reinterpret_cast<int&>(amount) &= ~1;
	}
	return amount;
}

void CCritHack::Run(CUserCmd* pCmd)
{
	if (!IsEnabled()) { return; }

	const auto& pWeapon = g_EntityCache.GetWeapon();
	if (!pWeapon || !pWeapon->CanFireCriticalShot(false)) { return; }

	ScanForCrits(pCmd, 50); //	fill our vector slowly.

	const int closestGoodTick = LastGoodCritTick(pCmd); //	retrieve our wish
	if (IsAttacking(pCmd, pWeapon)) //	is it valid & should we even use it
	{
		if (ShouldCrit())
		{
			if (closestGoodTick < 0) { return; }
			pCmd->command_number = closestGoodTick; //	set our cmdnumber to our wish
			pCmd->random_seed = MD5_PseudoRandom(closestGoodTick) & MASK_SIGNED; //	trash poopy whatever who cares

		}
		else
		{
			for (int tries = 1; tries < 25; tries++)
			{
				if (std::find(CritTicks.begin(), CritTicks.end(), pCmd->command_number + tries) != CritTicks.end())
				{
					continue; //	what a useless attempt
				}
				pCmd->command_number = tries;
				pCmd->random_seed = MD5_PseudoRandom(pCmd->command_number) & MASK_SIGNED;
				break; //	we found a seed that we can use to avoid a crit and have skipped to it, woohoo
			}
		}
	}

}

void fix_heavy_rev_bug(CUserCmd* pCmd)
{
	const auto& local = g_EntityCache.GetLocal();
	const auto& weapon = g_EntityCache.GetWeapon();
	if (!local || !weapon || local->deadflag())
	{
		return;
	}

	if (!local->IsClass(CLASS_HEAVY) || weapon->GetWeaponID() != TF_WEAPON_MINIGUN)
		return;

	if (pCmd->buttons & IN_ATTACK)
		pCmd->buttons &= ~IN_ATTACK2;
}

bool CCritHack::CritHandler()
{
	if (!I::Prediction->m_bFirstTimePredicted)
	{
		return false;
	}

	const auto& pLocal = g_EntityCache.GetLocal();
	const auto& pWeapon = g_EntityCache.GetWeapon();

	if (!pWeapon || pLocal)
	{
		return false;
	}

	{
		static int s_nPreviousTickcount = 0;

		if (s_nPreviousTickcount == I::GlobalVars->tickcount)
		{
			return false;
		}

		s_nPreviousTickcount = I::GlobalVars->tickcount;
	}

	{
		if (pWeapon->GetWeaponID() == TF_WEAPON_MINIGUN ||
			pWeapon->GetWeaponID() == TF_WEAPON_FLAMETHROWER)
		{
			auto nPreviousAmmoCount = pLocal->GetAmmoCount(pWeapon->m_iPrimaryAmmoType());
			static auto nNewAmmoCount = nPreviousAmmoCount;

			const auto bHasFiredBullet = nNewAmmoCount != nPreviousAmmoCount;

			if (!bHasFiredBullet)
			{
				return false;
			}
		}
	}

	if (wish_random_seed != 0)
	{
		*I::RandomSeed = wish_random_seed;
		wish_random_seed = 0;
	}

	return true;
}

MAKE_HOOK(AddToCritBucket, g_Pattern.Find(L"client.dll", L"55 8B EC A1 ? ? ? ? F3 0F 10 81 ? ? ? ? F3 0F 10 48"),
	void, __fastcall, void* ECX, void* EDX, float Damage)
{

	Hook.Original<FN>()(ECX, EDX, Damage);
}


MAKE_HOOK(CanFireRandomCriticalShot, g_Pattern.Find(L"client.dll", L"55 8B EC F3 0F 10 4D ? F3 0F 58 0D"),
	bool, __fastcall, void* ECX, void* EDX, float Chance)
{

	return Hook.Original<FN>()(ECX, EDX, Chance);
}

MAKE_HOOK(IsAllowedToWithdrawFromBucket, g_Pattern.Find(L"client.dll", L"55 8B EC 56 8B F1 0F B7 86 ? ? ? ? FF 86 ? ? ? ? 50 E8 ? ? ? ? 83 C4 04 80 B8 ? ? ? ? ? 74 0A F3 0F 10 15"),
	bool, __fastcall, void* ECX, void* EDX, float Damage)
{

	return Hook.Original<FN>()(ECX, EDX, Damage);
}


void CCritHack::Draw()
{
	if (!Vars::CritHack::Indicators.Value) { return; }

	const auto& pLocal = g_EntityCache.GetLocal();
	if (!pLocal || !pLocal->IsAlive()) { return; }

	const auto& pWeapon = pLocal->GetActiveWeapon();
	if (!pWeapon) { return; }


	const DragBox_t DTBox = Vars::Visuals::IndicatorPos;
	const auto fontHeight = Vars::Fonts::FONT_INDICATORS::nTall.Value;

	float random = getWithdrawAmount(pWeapon);
	g_Draw.String(FONT_INDICATORS, DTBox.x + 16, DTBox.y - fontHeight, { 255, 0, 0, 255 }, ALIGN_CENTERHORIZONTAL, "Ready");
	g_Draw.String(FONT_INDICATORS, DTBox.x + 40, DTBox.y + fontHeight + 12, { 255, 255, 255, 255 }, ALIGN_CENTERHORIZONTAL, std::format("{}/{} [{}]", static_cast<std::int32_t>(pWeapon->CritTokenBucket()), 1000, random).c_str());
	g_Draw.String(FONT_INDICATORS, DTBox.x + 40, DTBox.y + fontHeight + 24, { 255, 255, 255, 255 }, ALIGN_CENTERHORIZONTAL, std::format("damage untill ban: [{}]", static_cast<std::int32_t>(damageUntilToCrit(pWeapon))).c_str());
	const float ratioCurrent = std::clamp((static_cast<float>(random) / static_cast<float>(1000)), 0.0f, 1.0f); //cost shit xd.
	static float ratioInterp = 0.0f;
	ratioInterp = g_Draw.EaseIn(ratioInterp, ratioCurrent, 1.0f);
	Math::Clamp(ratioInterp, 0.0f, 1.0f);

	g_Draw.OutlinedRect(DTBox.x, DTBox.y, 124, 20, { 0,0,0,255 }); //	draw the outline
	g_Draw.Rect(DTBox.x + 1, DTBox.y + 1, 124 - 2, 20 - 2, { 28, 29, 38, 255 }); //	draw the background
	g_Draw.GradientRectWH(DTBox.x + 1, DTBox.y + 1, ratioInterp * (124 - 2), 20 - 2, { 255,127,80, 255 }, { 255,127,80, 255 }, true);

	if (pWeapon->CritTokenBucket() > random)
	{
		float fix = pWeapon->CritTokenBucket() - random;
		const float ratioCurren = std::clamp((static_cast<float>(fix) / static_cast<float>(1000)), 0.0f, 1.0f);
		static float ratioInter = 0.0f;
		ratioInter = g_Draw.EaseIn(ratioInter, ratioCurren, 1.0f);
		Math::Clamp(ratioInter, 0.0f, 1.0f);

		float update = (1000 - fix) / 1000;
		g_Draw.Rect(DTBox.x + update * (124 + 2), DTBox.y + 1, ratioInter * (124 + 2), 20 - 2, { 0,255,0, 255 });
	}
}
