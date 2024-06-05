#include "Hooks/HookManager.h"
#include "Hooks/PatchManager/PatchManager.h"


#include "Features/Visuals/Visuals.h"
#include "Features/Misc/Misc.h"
#include "Features/Vars.h"
#include <windows.h>
#include "Features/Menu/Menu.h"

#include "Features/Menu/ConfigManager/ConfigManager.h"
#include "Features/Commands/Commands.h"
#include "Features/CritHack/CritHack.h"

#include "SDK/Includes/Enums.h"
#include "Utils/MinHook/MinHook.h"



void Sleep(int ms)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

int StringToWString(std::wstring& ws, const std::string& s)
{
	const std::wstring wsTmp(s.begin(), s.end());
	ws = wsTmp;

	return 0;
}

void Loaded()
{
	const int dxLevel = g_ConVars.FindVar("mat_dxlevel")->GetInt();
	if (dxLevel < 90)
	{
		MessageBoxA(nullptr, _("Your DirectX version is too low!\nPlease use dxlevel 90 or higher"), _("dxlevel too low"), MB_OK | MB_ICONWARNING);
	}

}




void Initialize()
{
	g_SteamInterfaces.Init();
	g_Interfaces.Init();
	g_NetVars.Init();
	g_HookManager.Init();
	g_PatchManager.Init();
	g_ConVars.Init();
	g_Draw.RemakeFonts
	({
		{ 0x0, "calibri", 22, 0, FONTFLAG_OUTLINE | FONTFLAG_ANTIALIAS}, //name esp
		{ 0x0, "calibri", 22, 0, FONTFLAG_OUTLINE | FONTFLAG_ANTIALIAS}, //name esp
	{ 0x0, "calibri", 22, 0, FONTFLAG_OUTLINE | FONTFLAG_ANTIALIAS}, //name esp
	{ 0x0, "calibri", 22, 0, FONTFLAG_OUTLINE | FONTFLAG_ANTIALIAS}, //name esp
		{ 0x0, "Verdana", 12, 800, FONTFLAG_OUTLINE}, //menu esp
		{ 0x0, "Verdana", 12, 800, FONTFLAG_OUTLINE}, //indicators esp
		{ 0x0, "Verdana", 12, 800, FONTFLAG_OUTLINE}, //idk yet
		{ 0x0, "Verdana", 12, 800, FONTFLAG_OUTLINE}, //idk yet x2
		});

	//I::EngineClient->ClientCmd_Unrestricted("playvideo intro");
}
	

void Uninitialize()
{
	g_HookManager.Release();
	g_PatchManager.Restore();
}




void LoadDefaultConfig()
{
	if (std::filesystem::exists(g_CFG.GetConfigPath() + "\\" + g_CFG.GetCurrentConfig() + ".fw")) {
		g_CFG.LoadConfig(g_CFG.GetCurrentConfig());
	}

	g_Draw.RemakeFonts
	({
		{ 0x0, Vars::Fonts::FONT_ESP::szName.c_str(), Vars::Fonts::FONT_ESP::nTall.Value, Vars::Fonts::FONT_ESP::nWeight.Value, Vars::Fonts::FONT_ESP::nFlags.Value},
		{ 0x0, Vars::Fonts::FONT_ESP_NAME::szName.c_str(), Vars::Fonts::FONT_ESP_NAME::nTall.Value, Vars::Fonts::FONT_ESP_NAME::nWeight.Value, Vars::Fonts::FONT_ESP_NAME::nFlags.Value },
		{ 0x0, Vars::Fonts::FONT_ESP_COND::szName.c_str(), Vars::Fonts::FONT_ESP_COND::nTall.Value, Vars::Fonts::FONT_ESP_COND::nWeight.Value, Vars::Fonts::FONT_ESP_COND::nFlags.Value },
		{ 0x0, Vars::Fonts::FONT_ESP_PICKUPS::szName.c_str(), Vars::Fonts::FONT_ESP_PICKUPS::nTall.Value, Vars::Fonts::FONT_ESP_PICKUPS::nWeight.Value, Vars::Fonts::FONT_ESP_PICKUPS::nFlags.Value },
		{ 0x0, Vars::Fonts::FONT_MENU::szName.c_str(), Vars::Fonts::FONT_MENU::nTall.Value, Vars::Fonts::FONT_MENU::nWeight.Value, Vars::Fonts::FONT_MENU::nFlags.Value},
		{ 0x0, Vars::Fonts::FONT_INDICATORS::szName.c_str(), Vars::Fonts::FONT_INDICATORS::nTall.Value, Vars::Fonts::FONT_INDICATORS::nWeight.Value, Vars::Fonts::FONT_INDICATORS::nFlags.Value},
		{ 0x0, "Verdana", 18, 1600, FONTFLAG_ANTIALIAS},
		});
	F::Menu.ConfigLoaded = true;
}


LONG WINAPI UnhandledExFilter(PEXCEPTION_POINTERS ExPtr);

DWORD WINAPI MainThread(LPVOID lpParam)
{
	//AddVectoredExceptionHandler(0, UnhandledExFilter);

	//"mss32.dll" being one of the last modules to be loaded
	//So wait for that before proceeding, after it's up everything else should be too
	//Allows us to correctly use autoinject and just start the game.
	while (!WinAPI::GetModuleHandleW(_(L"mss32.dll")) || !WinAPI::GetModuleHandleW(_(L"ntdll.dll")) || !WinAPI::GetModuleHandleW(_(L"stdshader_dx9.dll")) || !WinAPI::GetModuleHandleW(_(L"materialsystem.dll"))) {
		Sleep(5000);
	}

	Initialize();
	LoadDefaultConfig();

	Loaded();

	while (!GetAsyncKeyState(VK_F11) || F::Menu.IsOpen) {
		Sleep(20);
	}

	Uninitialize();

	WinAPI::FreeLibraryAndExitThread(static_cast<HMODULE>(lpParam), EXIT_SUCCESS);
	return EXIT_SUCCESS;
}


BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	if (fdwReason == DLL_PROCESS_ATTACH)
	{	
		Utils::RemovePEH(hinstDLL);
		if (const auto hMainThread = WinAPI::CreateThread(nullptr, 0, MainThread, hinstDLL, 0, nullptr))
		{
			WinAPI::CloseHandle(hMainThread);
		}
	}

	return TRUE;
}

//https://www.unknowncheats.me/forum/c-and-c-/63409-write-mindump-crash.html

#include <dbghelp.h>
#include <shlobj.h>
#include <tchar.h>

LONG WINAPI UnhandledExFilter(PEXCEPTION_POINTERS ExPtr)
{
	BOOL(WINAPI * pMiniDumpWriteDump)(IN HANDLE hProcess, IN DWORD ProcessId, IN HANDLE hFile, IN MINIDUMP_TYPE DumpType, IN CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam, OPTIONAL IN CONST PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam, OPTIONAL IN CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam OPTIONAL) = NULL;

	HMODULE hLib = LoadLibrary(_T("dbghelp"));
	if (hLib)
		*(void**)&pMiniDumpWriteDump = (void*)GetProcAddress(hLib, "MiniDumpWriteDump");

	TCHAR buf[MAX_PATH], buf2[MAX_PATH];

	if (pMiniDumpWriteDump)
	{
		SHGetFolderPath(NULL, CSIDL_DESKTOPDIRECTORY, NULL, SHGFP_TYPE_CURRENT, buf);
		int rnd;
		__asm push edx
		__asm rdtsc
		__asm pop edx
		__asm mov rnd, eax
		rnd &= 0xFFFF;
		wsprintfW(buf2, _T("%s\\Fedoraware_CrashDump_%x%x%x.dmp"), buf, rnd, rnd, rnd);
		HANDLE hFile = CreateFile(buf2, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

		if (hFile != INVALID_HANDLE_VALUE)
		{
			MINIDUMP_EXCEPTION_INFORMATION md;
			md.ThreadId = GetCurrentThreadId();
			md.ExceptionPointers = ExPtr;
			md.ClientPointers = FALSE;
			BOOL win = pMiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &md, 0, 0);

			if (!win)
				wsprintfW(buf, _T("MiniDumpWriteDump failed. Error: %u \n(%s)"), GetLastError(), buf2);
			else
				wsprintfW(buf, _T("Minidump created:\n%s"), buf2);
			CloseHandle(hFile);

		}
		else
		{
			wsprintfW(buf, _T("Could not create minidump:\n%s"), buf2);
		}
	}
	else
	{
		wsprintf(buf, _T("Could not load dbghelp"));
	}
	WinAPI::MessageBoxW(NULL, buf, _T("Dump file on desktop, make an issue"), MB_OK | MB_ICONERROR);
	abort();
}
