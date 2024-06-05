#include "Menu.h"

#include "../Vars.h"

#include "../Misc/Misc.h"

#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_stdlib.h"
#include "Fonts/IconsMaterialDesign.h"
#include "Playerlist/Playerlist.h"

#include "Components.hpp"
#include "ConfigManager/ConfigManager.h"

#include <mutex>

int unuPrimary = 0;
int unuSecondary = 0;

/* The main menu */
void CMenu::DrawMenu()
{
	ImGui::SetNextWindowSize(ImVec2(700, 400), ImGuiCond_FirstUseEver);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize,ImVec2(700, 400));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);

	if (ImGui::Begin(" ", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar | 2))
	{
		const auto drawList = ImGui::GetWindowDrawList();
		const auto windowSize = ImGui::GetWindowSize();
		const auto windowPos = ImGui::GetWindowPos();

		// Icons
		{
			float currentX = windowSize.x;

			// Settings Icon
			ImGui::SetCursorPos({ currentX -= 25, 0 });
			if (ImGui::IconButton(ICON_MD_SETTINGS))
			{
				ShowSettings = !ShowSettings;
			}
			ImGui::HelpMarker("Settings");

			// Playerlist Icon
			ImGui::SetCursorPos({ currentX -= 25, 0 });
			if (ImGui::IconButton(ICON_MD_PEOPLE))
			{
				Vars::Menu::ShowPlayerlist = !Vars::Menu::ShowPlayerlist;
			}
			ImGui::HelpMarker("Playerlist");

		

		}

		// Tabbar
		ImGui::SetCursorPos({ 0, TitleHeight });
		ImGui::PushStyleColor(ImGuiCol_ChildBg, BackgroundDark.Value);
		if (ImGui::BeginChild("Tabbar", { windowSize.x + 5, TabHeight + SubTabHeight }, false, ImGuiWindowFlags_NoScrollWithMouse | 2))
		{
			DrawTabbar();
		}
		ImGui::EndChild();
		ImGui::PopStyleColor();

		// Main content
		ImGui::SetCursorPos({ 0, TitleHeight + TabHeight + SubTabHeight });
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 8.f, 10.f });
		ImGui::PushStyleColor(ImGuiCol_ChildBg, BackgroundDark.Value);
		if (ImGui::BeginChild("Content", { windowSize.x, windowSize.y - (TitleHeight + TabHeight + SubTabHeight) }, false, ImGuiWindowFlags_AlwaysUseWindowPadding | ImGuiWindowFlags_NoScrollbar | 2))
		{
			ImGui::PushFont(VerdanaSmall);
			ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, { 3.f, 2.f });

			switch (CurrentTab)
			{
			case MenuTab::Aimbot: { MenuAimbot(); break; }
			case MenuTab::Trigger: { MenuTrigger(); break; }
			case MenuTab::Visuals: { MenuVisuals(); break; }
			case MenuTab::HvH: { MenuHvH(); break; }
			}

			ImGui::PopStyleVar();
			ImGui::PopFont();
		}
		ImGui::EndChild();
		ImGui::PopStyleColor();
		ImGui::PopStyleVar();


		// End
		ImGui::End();
	}

	ImGui::PopStyleVar(2);
}

void CMenu::DrawTabbar()
{
	ImGui::PushFont(VerdanaSmall);
	ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, { 0, 0 });
	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.f);
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0, 0 });

	if (ImGui::BeginTable("TabbarTable", 4))
	{
		ImGui::PushStyleColor(ImGuiCol_Button, BackgroundDark.Value);
		ImGui::PushStyleColor(ImGuiCol_Text, TextLight.Value);
		if (ImGui::TabButton("Aimbot", CurrentTab == MenuTab::Aimbot))
		{
			CurrentTab = MenuTab::Aimbot;
		}

		if (ImGui::TabButton("Triggerbot", CurrentTab == MenuTab::Trigger))
		{
			CurrentTab = MenuTab::Trigger;
		}

		if (ImGui::TabButton("Visuals", CurrentTab == MenuTab::Visuals))
		{
			CurrentTab = MenuTab::Visuals;
		}

		if (ImGui::TabButton("HvH", CurrentTab == MenuTab::HvH))
		{
			CurrentTab = MenuTab::HvH;
		}


		ImGui::PopStyleColor(2);
		ImGui::EndTable();
	}

	ImGui::SetCursorPosY(TabHeight);

	SubTabHeight = 0.f;
	
	
	ImGui::PopStyleVar(3);
	ImGui::PopFont();
}

#pragma region Tabs
/* Tab: Aimbot */
void CMenu::MenuAimbot()
{
	using namespace ImGui;

	if (BeginTable("AimbotTable", 3))
	{
		/* Column 1 */
		if (TableColumnChild("AimbotCol1"))
		{
			SectionTitle("Global");
			WToggle("Aimbot", &Vars::Aimbot::Global::Active.Value); HelpMarker("Aimbot master switch");
		    InputKeybind("Aimbot key", Vars::Aimbot::Global::AimKey); HelpMarker("The key to enable aimbot"); 
			WSlider("Aimbot FoV####AimbotFoV", &Vars::Aimbot::Global::AimFOV.Value, 0.f, 180.f, "%.f", ImGuiSliderFlags_AlwaysClamp);
			WToggle("Autoshoot###AimbotAutoshoot", &Vars::Aimbot::Global::AutoShoot.Value); HelpMarker("Automatically shoot when a target is found");
			MultiCombo({ "Players", "Buildings", "Stickies"}, {&Vars::Aimbot::Global::AimPlayers.Value, &Vars::Aimbot::Global::AimBuildings.Value, &Vars::Aimbot::Global::AimStickies.Value}, "Aim targets");
			HelpMarker("Choose which targets the Aimbot should aim at");
			{
				static std::vector flagNames{ "Invulnerable", "Cloaked", "Dead Ringer", "Friends", "Taunting", "Vaccinator"};
				static std::vector flagValues{ 1 << 0, 1 << 1, 1 << 2, 1 << 3, 1 << 4, 1 << 5 };
				MultiFlags(flagNames, flagValues, &Vars::Aimbot::Global::IgnoreOptions.Value, "Ignored targets###AimbotIgnoredTargets");
				HelpMarker("Choose which targets should be ignored");
			}
			SectionTitle("Backtrack");
			WToggle("Active", &Vars::Backtrack::Enabled.Value); HelpMarker("If you shoot at the backtrack manually it will attempt to hit it");
			WToggle("Aimbot aims last tick", &Vars::Backtrack::LastTick.Value); HelpMarker("Aimbot aims at the last tick if visible");
			WToggle("Fake latency", &Vars::Backtrack::FakeLatency.Value); HelpMarker("Fakes your latency to hit records further in the past");
			WSlider("Amount of latency###BTLatency", &Vars::Backtrack::Latency.Value, 0.f, 800.f, "%.fms", ImGuiSliderFlags_AlwaysClamp | ImGuiSliderFlags_ClampOnInput); HelpMarker("This won't work on local servers");

			
		} EndChild();

		/* Column 2 */
		if (TableColumnChild("AimbotCol2"))
		{
			SectionTitle("Hitscan");
			WCombo("Sort method###HitscanSortMethod", &Vars::Aimbot::Hitscan::SortMethod.Value, { "FOV", "Distance" }); HelpMarker("Which method the aimbot uses to decide which target to aim at");
			if (Vars::Aimbot::Hitscan::SortMethod.Value == 1) {
				WToggle("Respect FOV", &Vars::Aimbot::Hitscan::RespectFOV.Value); HelpMarker("Respect the Aim FOV set when using distance sorting.");
			}
			WCombo("Preferred Hitbox###HitscanHitbox", &Vars::Aimbot::Hitscan::AimHitbox.Value, { "Head", "Body", "Auto" });
			WCombo("Aim method###HitscanAimMethod", &Vars::Aimbot::Hitscan::AimMethod.Value, { "Plain", "Smooth", "Silent" }); HelpMarker("Which method the aimbot uses to aim at the target");
			{
				static std::vector flagNames{ "Head", "Pelvis",  };
				static std::vector flagValues{ 0x00000001, 0x00000002 }; // 1<<1 and 1<<2 are swapped because the enum for hitboxes is weird.
				MultiFlags(flagNames, flagValues, &Vars::Aimbot::Hitscan::ScanHitboxes.Value, "Scan Hitboxes###AimbotHitboxScanning");
			}
			{
				static std::vector flagNames{ "Head", "Pelvis", };
				static std::vector flagValues{ 0x00000001, 0x00000002 }; // 1<<1 and 1<<2 are swapped because the enum for hitboxes is weird.
				MultiFlags(flagNames, flagValues, &Vars::Aimbot::Hitscan::MultiHitboxes.Value, "Multipoint Hitboxes###AimbotMultipointScanning");
			}
			WToggle("Wait for headshot", &Vars::Aimbot::Hitscan::WaitForHeadshot.Value); HelpMarker("The aimbot will wait until it can headshot (if applicable)");
			WToggle("Scan Buildings", &Vars::Aimbot::Hitscan::ScanBuildings.Value);


			SectionTitle("Crits");
			WToggle("Crit hack", &Vars::CritHack::Active.Value);  HelpMarker("Enables the crit hack (BETA)");
			MultiCombo({ "Indicators", "Avoid Random", "Always Melee" }, { &Vars::CritHack::Indicators.Value, &Vars::CritHack::AvoidRandom.Value, &Vars::CritHack::AlwaysMelee.Value }, "Misc###CrithackMiscOptions");
			HelpMarker("Misc options for crithack");
			InputKeybind("Crit key", Vars::CritHack::CritKey); HelpMarker("Will try to force crits when the key is held");

		
	
		} EndChild();

		/* Column 3 */
		if (TableColumnChild("AimbotCol3"))
		{
			SectionTitle("Projectile");
			WSlider("Prediction Time", &Vars::Aimbot::Projectile::PredictionTime.Value, 2.0f, 10.f, "%.1f");
			WSlider("Samples", &Vars::Aimbot::Projectile::StrafeChance.Value, 1.f, 100.f, "%.1f");
			WToggle("Strafe Prediction", &Vars::Aimbot::Hitscan::ExtinguishTeam.Value);
			{
				WCombo("Sort method###ProjectileSortMethod", &Vars::Aimbot::Projectile::SortMethod.Value, { "FOV", "Distance" });
				if (Vars::Aimbot::Projectile::SortMethod.Value == 1) {
					WToggle("Respect FOV", &Vars::Aimbot::Projectile::RespectFOV.Value); HelpMarker("Respect the Aim FOV set when using distance sorting.");
				}
				WCombo("Aim method###ProjectileAimMethod", &Vars::Aimbot::Projectile::AimMethod.Value, { "Plain", "Silent" });
				WCombo("Priority Hitbox###ProjectileHitboxPriority", &Vars::Aimbot::Projectile::AimPosition.Value, { "Head", "Body", "Feet", "Auto"});
				{
					static std::vector flagNames{ "Head", "Body", "Feet" };
					static std::vector flagValues{ (1<<0), (1<<1), (1<<2)}; // 1<<1 and 1<<2 are swapped because the enum for hitboxes is weird.
					MultiFlags(flagNames, flagValues, &Vars::Aimbot::Projectile::AllowedHitboxes.Value, "Allowed Hitboxes###ProjectileHitboxScanning"); HelpMarker("Controls what hitboxes the cheat is allowed to consider shooting at.");
				}

				
			}
			MultiCombo({ "Feet Aim on Ground", "Splash AI (NOT ADDED)", "Viewmodel Flipper", "Charge Loose Cannon" }, { &Vars::Aimbot::Projectile::FeetAimIfOnGround.Value, &Vars::Aimbot::Projectile::SplashPrediction.Value, &Vars::Misc::ViewmodelFlip.Value, &Vars::Aimbot::Projectile::ChargeLooseCannon.Value }, "Preferences###ProjectileAimbotPreferences");

			SectionTitle("Melee");
			{
				WCombo("Sort method###MeleeSortMethod", &Vars::Aimbot::Melee::SortMethod.Value, { "FOV", "Distance", }); HelpMarker("Which method the aimbot uses to decide which target to aim at");
				WCombo("Aim method###MeleeAimMethod", &Vars::Aimbot::Melee::AimMethod.Value, { "Plain", "Smooth", "Silent" }); HelpMarker("Which method the aimbot uses to aim at the target");
				MultiCombo({ "Range check", "Swing", }, { &Vars::Aimbot::Melee::RangeCheck.Value, &Vars::Aimbot::Melee::PredictSwing.Value }, "Preferences###Preferences");
			}
	
		} EndChild();

		/* End */
		EndTable();
	}
}

/* Tab: Trigger */
void CMenu::MenuTrigger()
{
	using namespace ImGui;
	if (BeginTable("TriggerTable", 3))
	{
		/* Column 1 */
		if (TableColumnChild("TriggerCol1"))
		{
			SectionTitle("Global");
			WToggle("Triggerbot", &Vars::Triggerbot::Global::Active.Value); HelpMarker("Global triggerbot master switch");
			InputKeybind("Trigger key", Vars::Triggerbot::Global::TriggerKey); HelpMarker("The key which activates the triggerbot");
			MultiCombo({ "Invulnerable", "Cloaked", "Friends" }, { &Vars::Triggerbot::Global::IgnoreInvlunerable.Value, &Vars::Triggerbot::Global::IgnoreCloaked.Value, &Vars::Triggerbot::Global::IgnoreFriends.Value }, "Ignored targets###TriggerIgnoredTargets");
			HelpMarker("Choose which targets should be ignored");

			SectionTitle("Autoshoot");
			WToggle("Autoshoot###AutoshootTrigger", &Vars::Triggerbot::Shoot::Active.Value); HelpMarker("Shoots if mouse is over a target");
			MultiCombo({ "Players", "Buildings" }, { &Vars::Triggerbot::Shoot::TriggerPlayers.Value, &Vars::Triggerbot::Shoot::TriggerBuildings.Value }, "Trigger targets");
			HelpMarker("Choose which target the triggerbot should shoot at");
		} EndChild();

		/* Column 2 */
		if (TableColumnChild("TriggerCol2"))
		{
			SectionTitle("Autostab");
			WToggle("Auto backstab###TriggerAutostab", &Vars::Triggerbot::Stab::Active.Value); HelpMarker("Auto backstab will attempt to backstab the target if possible");
			WToggle("Rage mode", &Vars::Triggerbot::Stab::RageMode.Value); HelpMarker("Stabs whenever possible by aiming toward the back");
			WToggle("Silent", &Vars::Triggerbot::Stab::Silent.Value); HelpMarker("Aim changes made by the rage mode setting aren't visible");
			WToggle("Disguise on kill", &Vars::Triggerbot::Stab::Disguise.Value); HelpMarker("Will apply the previous disguise after stabbing");
			WToggle("Ignore razorback", &Vars::Triggerbot::Stab::IgnRazor.Value); HelpMarker("Will not attempt to backstab snipers wearing the razorback");

			SectionTitle("Auto Detonate");
			WToggle("Autodetonate###TriggerDet", &Vars::Triggerbot::Detonate::Active.Value);
			MultiCombo({ "Players", "Buildings", "Stickies" }, { &Vars::Triggerbot::Detonate::DetonateOnPlayer.Value, &Vars::Triggerbot::Detonate::DetonateOnBuilding.Value, &Vars::Triggerbot::Detonate::DetonateOnSticky.Value }, "Targets###AutoDetonateTargets");
			WToggle("Explode stickies###TriggerSticky", &Vars::Triggerbot::Detonate::Stickies.Value); HelpMarker("Detonate sticky bombs when a player is in range");
			WToggle("Detonate flares###TriggerFlares", &Vars::Triggerbot::Detonate::Flares.Value); HelpMarker("Detonate detonator flares when a player is in range");
			
		} EndChild();

		/* Column 3 */
		if (TableColumnChild("TriggerCol3"))
		{
			SectionTitle("Autoblast");
			WToggle("Autoblast###Triggreairblast", &Vars::Triggerbot::Blast::Active.Value); HelpMarker("Auto airblast master switch");
			WToggle("Rage airblast###TriggerAirRage", &Vars::Triggerbot::Blast::Rage.Value); HelpMarker("Will airblast whenever possible, regardless of FoV");
			WToggle("Silent###triggerblastsilent", &Vars::Triggerbot::Blast::Silent.Value); HelpMarker("Aim changes made by the rage mode setting aren't visible");
		

			SectionTitle("Autouber");
			WToggle("Autouber###Triggeruber", &Vars::Triggerbot::Uber::Active.Value); HelpMarker("Auto uber master switch");
			WToggle("Only uber friends", &Vars::Triggerbot::Uber::OnlyFriends.Value); HelpMarker("Auto uber will only activate if healing steam friends");
			WToggle("Preserve self", &Vars::Triggerbot::Uber::PopLocal.Value); HelpMarker("Auto uber will activate if local player's health falls below the percentage");
			WToggle("Vaccinator resistances", &Vars::Triggerbot::Uber::AutoVacc.Value); HelpMarker("Auto uber will automatically find the best resistance and pop when needed (This doesn't work properly)");
			WSlider("Health left (%)###TriggerUberHealthLeft", &Vars::Triggerbot::Uber::HealthLeft.Value, 1.f, 99.f, "%.0f%%", 1.0f); HelpMarker("The amount of health the heal target must be below to actiavte");
			WToggle("Activate charge trigger", &Vars::Triggerbot::Uber::VoiceCommand.Value); HelpMarker("Will ubercharge regardless of anything if your target says activate charge");
		} EndChild();

		EndTable();
	}
}

/* Tab: Visuals */
void CMenu::MenuVisuals()
{
	using namespace ImGui;

	switch (CurrentVisualsTab)
	{
	// Visuals: Players
	case VisualsTab::Players:
	{
		if (BeginTable("VisualsPlayersTable", 3))
		{
			/* Column 1 */
			if (TableColumnChild("VisualsPlayersCol1"))
			{
				SectionTitle("ESP Main");
				WToggle("Player ESP###EnablePlayerESP", &Vars::ESP::Players::Active.Value); HelpMarker("Will draw useful information/indicators on players");
				WToggle("World ESP###WorldESPActive", &Vars::ESP::World::Active.Value); HelpMarker("World ESP master switch");
				WToggle("Building ESP###BuildinGESPSwioifas", &Vars::ESP::Buildings::Active.Value); HelpMarker("Will draw useful information/indicators on buildings");

			} EndChild();

			if (TableColumnChild("VisualsPlayersCol2"))
			{
				SectionTitle("Glow Main");
				WToggle("Glow", &Vars::Glow::Main::Active.Value);
				WToggle("Stencil glow", &Vars::Glow::Main::Stencil.Value);
				if (!Vars::Glow::Main::Stencil.Value) { WSlider("Glow scale", &Vars::Glow::Main::Scale.Value, 1, 10, "%d", ImGuiSliderFlags_AlwaysClamp); }
				WSlider("Glow alpha###PlayerGlowAlpha", &Vars::Glow::Players::Alpha.Value, 0.f, 1.f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
				WToggle("Player glow###PlayerGlowButton", &Vars::Glow::Players::Active.Value); HelpMarker("Player glow master switch");
				WToggle("Building glow###BuildiongGlowButton", &Vars::Glow::Buildings::Active.Value);
				WToggle("Self glow###SelfGlow", &Vars::Glow::Players::ShowLocal.Value); HelpMarker("Draw glow on the local player");
				WCombo("Ignore team###IgnoreTeamGlowp", &Vars::Glow::Players::IgnoreTeammates.Value, { "Off", "All", "Only friends" }); HelpMarker("Which teammates the glow will ignore drawing on");
				WCombo("Glow colour###GlowColour", &Vars::Glow::Players::Color.Value, { "Team", "Health" }); HelpMarker("Which colour the glow will draw");
				WToggle("Chams", &Vars::Chams::Main::Active.Value);
			} EndChild();

			if (TableColumnChild("VisualsPlayersCol3"))
			{
				SectionTitle("World & UI");
				WSlider("Field of view", &Vars::Visuals::FieldOfView.Value, 70, 150, "%d"); HelpMarker("How many degrees of field of vision you would like");
				MultiCombo({ "Scope", "Zoom", "Disguises", "Taunts", "Interpolation", "View Punch", "MOTD", "Screen Effects", "Angle Forcing", "Ragdolls", "Screen Overlays" }, { &Vars::Visuals::RemoveScope.Value, &Vars::Visuals::RemoveZoom.Value, &Vars::Visuals::RemoveDisguises.Value, &Vars::Visuals::RemoveTaunts.Value, &Vars::Misc::DisableInterpolation.Value, &Vars::Visuals::RemovePunch.Value, &Vars::Visuals::RemoveMOTD.Value, &Vars::Visuals::RemoveScreenEffects.Value, &Vars::Visuals::PreventForcedAngles.Value, &Vars::Visuals::RemoveRagdolls.Value, &Vars::Visuals::RemoveScreenOverlays.Value }, "Removals");
				WCombo("Particle tracer", &Vars::Visuals::ParticleTracer.Value, { "Off", "Machina", "C.A.P.P.E.R", "Short Circuit", "Merasmus ZAP", "Merasmus ZAP Beam 2", "Big Nasty", "Distortion Trail", "Black Ink", "Custom" });
				WCombo("DT indicator style", &Vars::Misc::CL_Move::DTBarStyle.Value, { "Off", "Nitro", }); HelpMarker("Which style to do the bar style");
				InputKeybind("Thirdperson key", Vars::Visuals::ThirdPersonKey); HelpMarker("What key to toggle thirdperson, press ESC if no bind is desired");
				WSlider("Distance", &Vars::Visuals::ThirdpersonDist.Value, 0, 150, "%d");
				WSlider("Side", &Vars::Visuals::ThirdpersonRight.Value, -40, 40, "%d");
				WSlider("Up/Down", &Vars::Visuals::ThirdpersonUp.Value, -40, 40, "%d");
				WToggle("Bypass sv_pure", &Vars::Misc::BypassPure.Value); HelpMarker("Allows you to load any custom files, even if disallowed by the sv_pure setting");
			
			}EndChild();

			EndTable();
		}
		break;
	}

	
	break;

	}
}

/* Tab: HvH */
void CMenu::MenuHvH()
{
	using namespace ImGui;
	if (BeginTable("HvHTable", 1))
	{
		/* Column 1 */
		if (TableColumnChild("HvHCol1"))
		{

			SectionTitle("Tickbase Exploits");
			WCombo("Quick stop", &Vars::Misc::AccurateMovement.Value, { "Off", "Legacy", "Instant" }); HelpMarker("Will stop you from sliding once you stop pressing movement buttons");
			WCombo("Autostrafe", &Vars::Misc::AutoStrafe.Value, { "Off", "Legit", "Directional" }); HelpMarker("Will strafe for you in air automatically so that you gain speed");
			WToggle("Break Anim", &Vars::Misc::DuckJump.Value); HelpMarker("Will duck when bhopping");
			WToggle("Bunnyhop", &Vars::Misc::AutoJump.Value); HelpMarker("Will jump as soon as you touch the ground again, keeping speed between jumps");
			WToggle("Doubletap", &Vars::Misc::CL_Move::Enabled.Value); HelpMarker("Shifts ticks when shooting for a rapid-fire effect");
			WToggle("Enable Fakelag", &Vars::Misc::CL_Move::Fakelag.Value);
			WToggle("Visualize FakeLag/Fake Model", &Vars::Aimbot::Global::showHitboxes.Value);
			InputKeybind("Recharge key", Vars::Misc::CL_Move::RechargeKey); HelpMarker("Recharges ticks for shifting");
			InputKeybind("Teleport key", Vars::Misc::CL_Move::TeleportKey); HelpMarker("Shifts ticks to warp");
			WToggle("Custom Value WARP", &Vars::AntiHack::Resolver::Resolver.Value);

			if (Vars::AntiHack::Resolver::Resolver.Value)
			{
				WSlider("WARP Factor", &Vars::Misc::CL_Move::DTs.Value, 0, 24, "%.1f", 0);
			}
			//Vars::Misc::CL_Move::SEnabled.Value
			WCombo("Fakelag Mode###FLmode", &Vars::Misc::CL_Move::FakelagMode.Value, { "Plain", "Nothing." }); HelpMarker("Controls how fakelag will be controlled.");
			WToggle("Custom Value FL", &Vars::Misc::CL_Move::RetainFakelag.Value);
			if (Vars::Misc::CL_Move::RetainFakelag.Value)
			{
				WSlider("FL Factor", &Vars::Misc::CL_Move::FakelagValue.Value, 0, 22, "%.1f", 0);
			}
			WToggle("Unchoke Fire", &Vars::Misc::CL_Move::OnFire.Value);
			WToggle("SpeedHack", &Vars::Misc::CL_Move::SEnabled.Value);
			if (Vars::Misc::CL_Move::SEnabled.Value)
			{
				WSlider("SpeedHack Factor", &Vars::Misc::CL_Move::DTTicks.Value, 0, 66, "%.1f", 0);
			}
			WToggle("Fast Accel", &Vars::Misc::CL_Move::Fast.Value);
			WToggle("IdealTick (NO Anti Warp)", &Vars::Misc::CL_Move::AntiWarp.Value);
			InputKeybind("RocketJump", Vars::Misc::CL_Move::AutoPeekKey);
			SectionTitle("Anti Aim");
			WToggle("Enable Anti-aim", &Vars::AntiHack::AntiAim::Active.Value);
			InputKeybind("Anti-aim Key", Vars::AntiHack::AntiAim::ToggleKey); 
			
			WCombo("Pitch", &Vars::AntiHack::AntiAim::Pitch.Value, { "None", "Up", "Down", "Fake up", "Fake down"}); HelpMarker("Which way to look up/down");
			WCombo("Real yaw", &Vars::AntiHack::AntiAim::YawReal.Value, { "None", "Forward", "Left", "Right", "Backwards", "Spin", "Edge", "Custom" }); HelpMarker("Which way to look horizontally");
			if (Vars::AntiHack::AntiAim::YawReal.Value == 7)
			{
				WSlider("Real First Angle", &Vars::AntiHack::AntiAim::first.Value, -180, 180, "%.1f", 0);
				WSlider("Real Second Angle", &Vars::AntiHack::AntiAim::second.Value, -180, 180, "%.1f", 0);
			}
			WCombo("Fake yaw", &Vars::AntiHack::AntiAim::YawFake.Value, { "None", "Forward", "Left", "Right", "Backwards", "Spin", "Edge", "Custom" }); HelpMarker("Which way to appear to look horizontally");
			if (Vars::AntiHack::AntiAim::YawFake.Value == 7)
			{
				WSlider("Fake First Angle", &Vars::AntiHack::AntiAim::FakeFirst.Value, -180, 180, "%.1f", 0);
				WSlider("Fake Second Angle", &Vars::AntiHack::AntiAim::FakeSecond.Value, -180, 180, "%.1f", 0);
			}

			if (Vars::AntiHack::AntiAim::Pitch.Value == 3 || Vars::AntiHack::AntiAim::Pitch.Value == 4)
			{
				WToggle("Fake Jiiter Pitch", &Vars::AntiHack::AntiAim::Roll.Value);
			}

			if (Vars::AntiHack::AntiAim::YawFake.Value == 5 || Vars::AntiHack::AntiAim::YawReal.Value == 5)
			{
				WSlider("Spin Speed", &Vars::AntiHack::AntiAim::SpinSpeed.Value, -30.f, 30.f, "%.1f", 0); 
			}


			SectionTitle("Queueing");
			WToggle("Region selector", &Vars::Misc::RegionChanger.Value);

			MultiFlags({ "Amsterdam", "Atlanta", "Mumbai", "Dubai", "Moses Lake", "Washington", "Frankfurt", "Tokyo (GNRT)", "Sao Paulo", "Hong Kong", "Virginia", "Johannesburg", "Los Angeles", "London", "Lima", "Luxembourg", "Chennai", "Madrid", "Manilla", "Oklahoma City", "Chicago", "Paris", "Santiago", "Seattle", "Singapore", "Stockholm", "Sydney", "Tokyo", "Vienna", "Warsaw" },
				{ DC_AMS,      DC_ATL,    DC_BOM,   DC_DXB,  DC_EAT,		 DC_MWH,	   DC_FRA,		DC_GNRT,		DC_GRU,		 DC_HKG,	  DC_IAD,	  DC_JNB,		  DC_LAX,		 DC_LHR,   DC_LIM, DC_LUX,		 DC_MAA,	DC_MAD,	  DC_MAN,    DC_OKC,		  DC_ORD,	 DC_PAR,  DC_SCL,     DC_SEA,	 DC_SGP,	  DC_STO,	   DC_SYD,   DC_TYO,  DC_VIE,	DC_WAW },
				&Vars::Misc::RegionsAllowed.Value,
				"Regions"
			);
			
		} EndChild();

	

		EndTable();
	}
}

#pragma endregion

/* Settings Window */
void CMenu::SettingsWindow()
{
	using namespace ImGui;
	if (!ShowSettings) { return; }

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12, 12));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(10, 10));

	if (Begin("Settings", &ShowSettings, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse))
	{
		if (ColorPicker("Menu accent", Vars::Menu::Colors::MenuAccent)) { LoadStyle(); } SameLine(); Text("Menu accent");
	
		SetNextItemWidth(100);
		InputKeybind("Extra Menu key", Vars::Menu::MenuKey, true, true);

		Dummy({ 0, 5 });

		if (Button("Open configs folder", ImVec2(200, 0)))
		{
			ShellExecuteA(NULL, NULL, g_CFG.GetConfigPath().c_str(), NULL, NULL, SW_SHOWNORMAL);
		}

		static std::string selected;
		int nConfig = 0;

		// Load config files
		for (const auto& entry : std::filesystem::directory_iterator(g_CFG.GetConfigPath()))
		{
			if (std::string(std::filesystem::path(entry).filename().string()).find(g_CFG.ConfigExtension) == std::string_view::npos)
			{
				continue;
			}
			nConfig++;
		}

		// Current config
		const std::string cfgText = "Loaded: " + g_CFG.GetCurrentConfig();
		Text(cfgText.c_str());

		// Config name field
		if (nConfig < 100)
		{
			std::string newConfigName = {};

			PushItemWidth(200);
			if (InputTextWithHint("###configname", "New config name", &newConfigName, ImGuiInputTextFlags_EnterReturnsTrue))
			{
				if (!std::filesystem::exists(g_CFG.GetConfigPath() + "\\" + newConfigName))
				{
					g_CFG.SaveConfig(newConfigName);
				}
			}
			PopItemWidth();
		}

		// Config list
		for (const auto& entry : std::filesystem::directory_iterator(g_CFG.GetConfigPath()))
		{
			if (std::string(std::filesystem::path(entry).filename().string()).find(g_CFG.ConfigExtension) == std::string_view::npos)
			{
				continue;
			}

			std::string configName = entry.path().filename().string();
			configName.erase(configName.end() - g_CFG.ConfigExtension.size(), configName.end());

			if (configName == selected)
			{
				const ImGuiStyle* style2 = &GetStyle();
				const ImVec4* colors2 = style2->Colors;
				ImVec4 buttonColor = colors2[ImGuiCol_Button];
				buttonColor.w *= .5f;
				PushStyleColor(ImGuiCol_Button, buttonColor);

				// Config name button
				if (Button(configName.c_str(), ImVec2(200, 20)))
				{
					selected = configName;
				}
				PopStyleColor();

				// Save config button
				if (Button("Save", ImVec2(61, 20)))
				{
					if (configName != g_CFG.GetCurrentConfig())
					{
						OpenPopup("Save config?");
					} else
					{
						g_CFG.SaveConfig(selected);
						selected.clear();
					}
				}

				// Load config button
				SameLine();
				if (Button("Load", ImVec2(61, 20)))
				{
					g_CFG.LoadConfig(selected);
					selected.clear();
					LoadStyle();
				}

				// Remove config button
				SameLine();
				if (Button("Remove", ImVec2(62, 20)))
				{
					OpenPopup("Remove config?");
				}

				// Save config dialog
				if (BeginPopupModal("Save config?", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
				{
					Text("Do you really want to override this config?");

					Separator();
					if (Button("Yes, override!", ImVec2(150, 0)))
					{
						g_CFG.SaveConfig(selected);
						selected.clear();
						CloseCurrentPopup();
					}

					SameLine();
					if (Button("No", ImVec2(120, 0)))
					{
						CloseCurrentPopup();
					}
					EndPopup();
				}

				// Delete config dialog
				if (BeginPopupModal("Remove config?", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
				{
					Text("Do you really want to delete this config?");

					Separator();
					if (Button("Yes, remove!", ImVec2(150, 0)))
					{
						g_CFG.RemoveConfig(selected);
						selected.clear();
						CloseCurrentPopup();
					}
					SameLine();
					if (Button("No", ImVec2(150, 0)))
					{
						CloseCurrentPopup();
					}
					EndPopup();
				}
			}
			else if (configName == g_CFG.GetCurrentConfig())
			{
				PushStyleColor(ImGuiCol_Button, GetStyle().Colors[ImGuiCol_ButtonActive]);
				std::string buttonText = "> " + configName + " <";
				if (Button(buttonText.c_str(), ImVec2(200, 20)))
				{
					selected = configName;
				}
				PopStyleColor();
			}
			else
			{
				if (Button(configName.c_str(), ImVec2(200, 20)))
				{
					selected = configName;
				}
			}
		}

		End();
	}

	PopStyleVar(2);
}

/* Debug Menu */
void CMenu::DebugMenu()
{
	#ifdef _DEBUG
	using namespace ImGui;
	if (!ShowDebugMenu) { return; }

	PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12, 12));
	PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(200, 200));

	if (Begin("Debug", &ShowDebugMenu, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse))
	{
		const auto& pLocal = g_EntityCache.GetLocal();

		Checkbox("Show Debug info", &Vars::Debug::DebugInfo.Value);
		Checkbox("Allow secure servers", I::AllowSecureServers);

		bool* m_bPendingPingRefresh = reinterpret_cast<bool*>(I::TFGCClientSystem + 828);
		Checkbox("Pending Ping Refresh", m_bPendingPingRefresh);

		// Particle tester
		if (CollapsingHeader("Particles"))
		{
			static std::string particleName = "ping_circle";

			InputText("Particle name", &particleName);
			if (Button("Dispatch") && pLocal != nullptr)
			{
				Particles::DispatchParticleEffect(particleName.c_str(), pLocal->GetAbsOrigin(), { });
			}
		}

		// Debug options
		if (CollapsingHeader("Debug options"))
		{
			Checkbox("Debug Bool", &Vars::Debug::DebugBool.Value);
		}

		End();
	}

	PopStyleVar(2);
	#endif
}


void CMenu::AddDraggable(const char* szTitle, DragBox_t& info, bool bShouldDraw, bool setSize)
{
	constexpr int titlebarheight = 20;

	if (bShouldDraw)
	{
		if (info.update)
		{
			if (setSize)
			{
				ImGui::SetNextWindowSize({ static_cast<float>(info.w), static_cast<float>(info.h) + titlebarheight }, ImGuiCond_Always);
			}
			else
			{
				ImGui::SetNextWindowSize({ 80.f, 60.f }, ImGuiCond_Always);
			}
			ImGui::SetNextWindowPos({ static_cast<float>(info.x), static_cast<float>(info.y) - titlebarheight }, ImGuiCond_Always);
			info.update = false;
		}
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.f, 0.f, 0.f, 0.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, { 50.f, 21.f });

		const auto bResize = setSize ? 0 : ImGuiWindowFlags_NoResize;
		if (ImGui::Begin(szTitle, nullptr, bResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus))
		{
			const ImVec2 winPos = ImGui::GetWindowPos();
			const ImVec2 winSize = ImGui::GetWindowSize();

			info.x = static_cast<int>(winPos.x);
			info.y = static_cast<int>(winPos.y + titlebarheight);	//	fix title bars
			if (setSize)
			{
				info.w = static_cast<int>(winSize.x);
				info.h = static_cast<int>(winSize.y - titlebarheight);	//	account for title bar fix
			}
			info.c = static_cast<int>(info.x + ((setSize ? info.w : 80.f) / 2));

			ImGui::End();
		}
		ImGui::PopStyleVar();
		ImGui::PopStyleColor();
	}
}


void CMenu::Render(IDirect3DDevice9* pDevice)
{
	if (!ConfigLoaded) { return; }

	static std::once_flag initFlag;
	std::call_once(initFlag, [&] {
		Init(pDevice);
	});

	pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0xFFFFFFFF);
	pDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	pDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	pDevice->SetRenderState(D3DRS_SRGBWRITEENABLE, false);

	// Toggle menu (default is 'insert' can be changed in menu)
	static KeyHelper menuKey{ &Vars::Menu::MenuKey.Value };
	if (menuKey.Pressed() || GetAsyncKeyState(VK_INSERT) & 0x1)
	{
		F::Menu.IsOpen = !F::Menu.IsOpen;
		I::VGuiSurface->SetCursorAlwaysVisible(F::Menu.IsOpen);
	}

	// Begin current frame
	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	ImGui::PushFont(VerdanaSmall);


	if (IsOpen)
	{
		DrawMenu();
		AddDraggable("DT Bar", Vars::Visuals::DTIndicator, Vars::Misc::CL_Move::DTBarStyle.Value, false);
		AddDraggable("Crits", Vars::Visuals::IndicatorPos, Vars::CritHack::Indicators.Value, false);
		SettingsWindow();
	
	
		
		F::PlayerList.Render();
	}



	// End frame and render
	ImGui::PopFont();
	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
	pDevice->SetRenderState(D3DRS_SRGBWRITEENABLE, true);
}

void CMenu::LoadStyle()
{
	// Style & Colors
	{
		ItemWidth = 120.f;

		// https://raais.github.io/ImStudio/
		Accent = ImGui::ColorToVec({255,255,255,255});
		AccentDark = ImColor(Accent.Value.x * 0.8f, Accent.Value.y * 0.8f, Accent.Value.z * 0.8f, Accent.Value.w);

		auto& style = ImGui::GetStyle();
		style.WindowTitleAlign = ImVec2(0.5f, 0.5f); // Center window title
		style.WindowMinSize = ImVec2(100, 100);
		style.WindowPadding = ImVec2(0, 0);
		style.FramePadding = ImVec2(4, 3);
		style.WindowBorderSize = 1.5f;
		style.ButtonTextAlign = ImVec2(0.5f, 0.5);
		style.FrameBorderSize = 1.f; // Old menu feeling
		style.FrameRounding = 5.f;
		style.ChildBorderSize = 1.5f;
		style.ChildRounding = 5.f;
		style.GrabMinSize = 10.f;
		style.GrabRounding = 5.f;
		style.ScrollbarSize = 5.f;
		style.ScrollbarRounding = 5.f;
		style.ItemSpacing = ImVec2(8.f, 4.f);

		ImVec4* colors = style.Colors;
		colors[ImGuiCol_Border] = ImColor(110, 110, 128);
		colors[ImGuiCol_WindowBg] = Background;
		colors[ImGuiCol_TitleBg] = BackgroundDark;
		colors[ImGuiCol_TitleBgActive] = BackgroundLight;
		colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.10f, 0.10f, 0.15f, 0.4f);
		colors[ImGuiCol_Button] = BackgroundLight;
		colors[ImGuiCol_ButtonHovered] = ImColor(69, 69, 77);
		colors[ImGuiCol_ButtonActive] = ImColor(82, 79, 87);
		colors[ImGuiCol_PopupBg] = BackgroundDark;
		colors[ImGuiCol_FrameBg] = ImColor(50, 50, 50);
		colors[ImGuiCol_FrameBgHovered] = ImColor(60, 60, 60);
		colors[ImGuiCol_FrameBgActive] = ImColor(70, 70, 70);
		colors[ImGuiCol_CheckMark] = Accent;
		colors[ImGuiCol_Text] = TextLight;

		colors[ImGuiCol_SliderGrab] = Accent;
		colors[ImGuiCol_SliderGrabActive] = AccentDark;
		colors[ImGuiCol_ResizeGrip] = Accent;
		colors[ImGuiCol_ResizeGripActive] = Accent;
		colors[ImGuiCol_ResizeGripHovered] = Accent;
		colors[ImGuiCol_Header] = ImColor(70, 70, 70);
		colors[ImGuiCol_HeaderActive] = ImColor(40, 40, 40);
		colors[ImGuiCol_HeaderHovered] = ImColor(60, 60, 60);

		
		style.FrameRounding = 5.f;
		style.GrabRounding = 5.f;
	}

	// Misc
	{
		TitleGradient.ClearMarks();
		TitleGradient.AddMark(0.f, ImColor(0, 0, 0, 0));
		TitleGradient.AddMark(0.3f, ImColor(0, 0, 0, 0));
		TitleGradient.AddMark(0.5f, Accent);
		TitleGradient.AddMark(0.7f, ImColor(0, 0, 0, 0));
		TitleGradient.AddMark(1.f, ImColor(0, 0, 0, 0));
	}

	{
		MainGradient.ClearMarks();
		MainGradient.AddMark(0.f, ImColor(0, 0, 0, 0));
		MainGradient.AddMark(0.3f, ImColor(0, 0, 0, 0));
		MainGradient.AddMark(0.5f, Accent);
		MainGradient.AddMark(0.7f, ImColor(0, 0, 0, 0));
		MainGradient.AddMark(1.f, ImColor(0, 0, 0, 0));
	}

	{
		TabGradient.ClearMarks();
		TabGradient.AddMark(0.f, ImColor(0, 0, 0, 0));
		TabGradient.AddMark(0.3f, ImColor(0, 0, 0, 0));
		TabGradient.AddMark(0.5f, Accent);
		TabGradient.AddMark(0.7f, ImColor(0, 0, 0, 0));
		TabGradient.AddMark(1.f, ImColor(0, 0, 0, 0));
	}
}

void CMenu::Init(IDirect3DDevice9* pDevice)
{
	// Initialize ImGui and device
	ImGui::CreateContext();
	ImGui_ImplWin32_Init(FindWindowA(nullptr, "Team Fortress 2"));
	ImGui_ImplDX9_Init(pDevice);

	// Fonts
	{
		const auto& io = ImGui::GetIO();

		auto fontConfig = ImFontConfig();
		fontConfig.OversampleH = 2;

		constexpr ImWchar fontRange[]{ 0x0020, 0x00FF,0x0400, 0x044F, 0 }; // Basic Latin, Latin Supplement and Cyrillic

		VerdanaSmall = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\verdana.ttf", 12.0f, &fontConfig, fontRange);
		Verdana = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\verdana.ttf", 14.0f, &fontConfig, fontRange);
		VerdanaBold = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\verdanab.ttf", 14.0f, &fontConfig, fontRange);

		SectionFont = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\verdana.ttf", 16.0f, &fontConfig, fontRange);
		TitleFont = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\verdanab.ttf", 20.0f, &fontConfig, fontRange);

		constexpr ImWchar iconRange[]{ ICON_MIN_MD, ICON_MAX_MD, 0 };
		ImFontConfig iconConfig;
		iconConfig.MergeMode = true;
		iconConfig.PixelSnapH = true;
		IconFont = io.Fonts->AddFontFromMemoryCompressedTTF(MaterialFont_compressed_data, MaterialFont_compressed_size, 16.f, &iconConfig, iconRange);

		io.Fonts->Build();
	}

	LoadStyle();

}

