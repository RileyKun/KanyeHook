#include "Playerlist.h"
#include "../../Vars.h"

#include "../../Menu/Menu.h"
#include "../ConfigManager/ConfigManager.h"
#include "../ImGui/imgui_color_gradient.h"
#include "../ImGui/imgui_internal.h"


#include <mutex>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

//?
const char* resolveListYaw[]{"None", "Forward", "Backward", "Left", "Right", "Invert", "Auto"};
const char* priorityModes[]{ "Friend", "Ignore", "Default", "Rage", "Cheater", "Fag", "sus", "Streamer", "Trans", "Closet"};

struct ListPlayer {
	const char* Name{};
	int UserID{};
	uint32_t FriendsID{};
	Color_t Color{};
	int Health{};
	int MaxHealth{};
	int Class{};
	bool Alive{};
	bool FakePlayer{};
};

std::mutex mutex;
std::multimap<int, ListPlayer> PlayerCache{};

void CPlayerList::UpdatePlayers()
{
	static Timer updateTimer{};
	if (updateTimer.Run(1000))
	{
		std::multimap<int, ListPlayer> listBuffer{};
		for (int i = 1; i <= I::GlobalVars->maxclients; i++)
		{
			const auto pr = g_EntityCache.GetPR();
			if (pr->GetValid(i) && pr->GetConnected(i))
			{
				PlayerInfo_t info{};
				if (I::EngineClient->GetPlayerInfo(i, &info))
				{
					ListPlayer player{
						pr->GetPlayerName(i),
						pr->GetUserID(i),
						pr->GetAccountID(i),
						Utils::GetTeamColor(pr->GetTeam(i), Vars::ESP::Main::EnableTeamEnemyColors.Value),
						pr->GetHealth(i),
						pr->GetMaxHealth(i),
						pr->GetClass(i),
						pr->IsAlive(i),
						info.fakeplayer
					};

					listBuffer.emplace(pr->GetTeam(i), player);
				}
			}
		}

		std::lock_guard lock(mutex);
		listBuffer.swap(PlayerCache);
	}
}

void CPlayerList::Run()
{
	// Save every 10 seconds, if needed
	static Timer saveTimer{ };
	if (saveTimer.Run(10000))
	{
		if (ShouldSave)
		{
			Save();
			ShouldSave = false;
		}

		if (ShouldLoad)
		{
			Load();
			ShouldLoad = false;
		}
	}
}

void CPlayerList::Render()
{
	if (!Vars::Menu::ShowPlayerlist) { return; }

	// Draw the playerlist
	ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(200, 22));
	if (ImGui::Begin("Playerlist", &Vars::Menu::ShowPlayerlist,
	                 ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse))
	{
		ImGui::PushFont(F::Menu.Verdana);
		const auto winSize = ImVec2(ImGui::GetWindowSize().x, ImGui::GetWindowSize().y);
		auto winPos = ImVec2(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y);
		ImGui::GradientRect(&F::Menu.TitleGradient, {winPos.x, winPos.y}, winSize.x, 3);
		ImGui::Dummy(ImVec2());

		// Check if we are in-game
		if (!I::EngineClient->IsInGame())
		{
	
		}
		else
		{
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 5, 5 });
			if (ImGui::BeginTable("Playerlist", 4,
				ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollY | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable))
			{
				ImGui::TableSetupColumn("Name");
				ImGui::TableSetupColumn("Class");
				ImGui::TableSetupColumn("Health");
				ImGui::TableSetupColumn("Priority");
				ImGui::TableHeadersRow();

				std::lock_guard lock(mutex);

				for (const auto& [Team, Player] : PlayerCache)
				{
					ImGui::TableNextRow();

					ImGui::PushID(ImGui::GetCurrentTable()->CurrentRow);

					for (int column = 0; column < 4; column++)
					{
						ImGui::TableSetColumnIndex(column);

						// don't show priority or resolver for bots
						if (Player.FakePlayer && column > 2)
						{
							continue;
						}

						switch (column)
						{
						case 0:
						{
							/* Name */
							ImGui::PushStyleColor(ImGuiCol_Text, ImColor(Player.Color.r, Player.Color.g, Player.Color.b, Player.Alive ? Player.Color.a : 127).Value);
							ImGui::Text("%s", Player.Name);
							ImGui::PopStyleColor();
							break;
						}

						case 1:
						{
							/* Class */
							ImGui::Text("%s", Utils::GetClassByIndex(Player.Class));
							break;
						}

						case 2:
						{
							/* Health */
							ImGui::Text("%i/%i", Player.Health, Player.MaxHealth);
							break;
						}

						case 3:
						{
							/* Priority */
							if (ImGui::Selectable(priorityModes[G::PlayerPriority[Player.FriendsID].Mode]))
							{
								ImGui::OpenPopup("priority_popup");
							}

							if (ImGui::BeginPopup("priority_popup"))
							{
								for (int i = 0; i < IM_ARRAYSIZE(priorityModes); i++)
								{
									if (ImGui::MenuItem(priorityModes[i]))
									{
										G::PlayerPriority[Player.FriendsID].Mode = i;
										ShouldSave = true;
									}
								}

								ImGui::EndPopup();
							}
							break;
						}

						}
					}

					ImGui::SameLine();
					ImGui::Selectable("##contextmenu", false, ImGuiSelectableFlags_SpanAllColumns);

					if (!Player.FakePlayer && ImGui::BeginPopupContextItem())
					{
						if (ImGui::Selectable("Profile"))
						{
							g_SteamInterfaces.Friends015->ActivateGameOverlayToUser(
								"steamid", CSteamID(0x0110000100000000ULL + Player.FriendsID));
						}

						if (ImGui::Selectable("Votekick"))
						{
							I::EngineClient->ClientCmd_Unrestricted(tfm::format("callvote kick %i", Player.UserID).c_str());
						}

						ImGui::EndPopup();
					}

					ImGui::PopID();
				}

				ImGui::EndTable();
			}

			ImGui::PopStyleVar();
		}

		ImGui::PopFont();
		ImGui::End();
	}
	ImGui::PopStyleVar();
}

void CPlayerList::Save()
{
	try
	{
		boost::property_tree::ptree writeTree;

		// Put map entries into ptree
		for (const auto& [user, prio] : G::PlayerPriority)
		{
			boost::property_tree::ptree userTree;
			userTree.put("Mode", prio.Mode);

			writeTree.put_child(std::to_string(user), userTree);
		}

		// Save the file
		write_json(g_CFG.GetConfigPath() + "\\Core\\Playerlist.json", writeTree);
	} catch (...) { }
}

void CPlayerList::Load()
{
	try
	{
		boost::property_tree::ptree readTree;
		read_json(g_CFG.GetConfigPath() + "\\Core\\Playerlist.json", readTree);
		G::PlayerPriority.clear();

		for (auto& it : readTree)
		{
			Priority prio;
			uint32_t userID = std::stoi(it.first);

			if (auto getValue = it.second.get_optional<int>("Mode"))
			{
				prio.Mode = std::max(*getValue, 0);
			}

			G::PlayerPriority[userID] = prio;
		}
	} catch (...) { }
}