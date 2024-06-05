#include "../Hooks.h"

MAKE_HOOK(C_BasePlayer_CalcViewModelView, g_Pattern.Find(L"client.dll", L"55 8B EC 83 EC 70 8B 55 0C 53 8B 5D 08 89 4D FC 8B 02 89 45 E8 8B 42 04 89 45 EC 8B 42 08 89 45 F0 56 57"), void, __fastcall,
		  void* ecx, void* edx, CBaseEntity* pOwner, const Vec3& vEyePosition, Vec3& vEyeAngles)
{
	
	Hook.Original<FN>()(ecx, edx, pOwner, vEyePosition, vEyeAngles);
}