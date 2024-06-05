#include "../Hooks.h"

#include "../../Features/Misc/Misc.h"
#include "../../Features/seedprediction/seed.hpp"

#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/predicate.hpp>

static int anti_balance_attempts = 0;
static std::string previous_name;

MAKE_HOOK(BaseClientDLL_DispatchUserMessage, Utils::GetVFuncPtr(I::BaseClientDLL, 36), bool, __fastcall,
		  void* ecx, void* edx, UserMessageType type, bf_read& msgData)
{

	switch (type)
	{
		case SayText2:
		{
			break;
		}

		case VoiceSubtitle:
		{
			int iEntityID = msgData.ReadByte();
			int iVoiceMenu = msgData.ReadByte();
			int iCommandID = msgData.ReadByte();

			if (iVoiceMenu == 1 && iCommandID == 6)
			{
				G::MedicCallers.push_back(iEntityID);
			}
			break;
		}

		case TextMsg:
		{
			if (msgData.GetNumBitsLeft() > 35)
			{
				if (F::NS.parsePlayerPerf(msgData))
				{
					return true;
				}

				const auto bufData = reinterpret_cast<const char*>(msgData.m_pData);
				msgData.SetAssertOnOverflow(false);

				const INetChannel* server = I::EngineClient->GetNetChannelInfo();
				const std::string data(bufData);

				if (data.find("TeamChangeP") != std::string::npos && g_EntityCache.GetLocal())
				{
					const std::string serverName(server->GetAddress());
					if (serverName != previous_name)
					{
						previous_name = serverName;
						anti_balance_attempts = 4;
					}
					if (anti_balance_attempts)
					{
						I::EngineClient->ClientCmd_Unrestricted("retry");
					}
					
					anti_balance_attempts++;
				}
			}
			break;
		}

		case VGUIMenu:
		{
			
				if (strcmp(reinterpret_cast<char*>(msgData.m_pData), "info") == 0)
				{
					I::EngineClient->ClientCmd_Unrestricted("closedwelcomemenu");
					return true;
				}
			

			
				if (strcmp(reinterpret_cast<char*>(msgData.m_pData), "team") == 0)
				{
					I::EngineClient->ClientCmd_Unrestricted("autoteam");
					I::EngineClient->ClientCmd_Unrestricted("autoteam");
					return true;
				}

				
			

			break;
		}

		case ForcePlayerViewAngles:
		{
			return Vars::Visuals::PreventForcedAngles.Value ? true : Hook.Original<FN>()(ecx, edx, type, msgData);
		}

		case SpawnFlyingBird:
		case PlayerGodRayEffect:
		case PlayerTauntSoundLoopStart:
		case PlayerTauntSoundLoopEnd:
		{
			return Vars::Visuals::RemoveTaunts.Value ? true : Hook.Original<FN>()(ecx, edx, type, msgData);
		}

		case Shake:
		case Fade:
		case Rumble:
		{
			return Vars::Visuals::RemoveScreenEffects.Value ? true : Hook.Original<FN>()(ecx, edx, type, msgData);
		}
	}

	return Hook.Original<FN>()(ecx, edx, type, msgData);
}