#include "../Hooks.h"
#include "../../Features/seedprediction/seed.hpp"

MAKE_HOOK(CL_SendMove, g_Pattern.Find(L"engine.dll", L"55 8B EC 81 EC ? ? ? ? A1 ? ? ? ? 8D 4D CC 56 57 8B 3D ? ? ? ? 40 03 F8 C6 45 B0 01"), void, __cdecl,
		  void* ecx, void* edx)
{
	F::NS.askForPlayerPerf();

	byte data[4000];
	const int nextcommandnr = I::ClientState->lastoutgoingcommand + I::ClientState->chokedcommands + 1;
	CLC_Move moveMsg;
	moveMsg.m_DataOut.StartWriting(data, sizeof(data));
	moveMsg.m_nNewCommands = std::clamp(1 + I::ClientState->chokedcommands, 0, 15);
	const int extraCommands = I::ClientState->chokedcommands + 1 - moveMsg.m_nNewCommands;
	const int backupCommands = std::max(2, extraCommands);
	moveMsg.m_nBackupCommands = std::clamp(backupCommands, 0, 7);
	const int numcmds = moveMsg.m_nNewCommands + moveMsg.m_nBackupCommands;
	int from = -1;
	bool bOK = true;
	for (int to = nextcommandnr - numcmds + 1; to <= nextcommandnr; to++)
	{
		const bool isnewcmd = to >= nextcommandnr - moveMsg.m_nNewCommands + 1;
		bOK = bOK && I::Input->WriteUsercmdDeltaToBuffer(&moveMsg.m_DataOut, from, to, isnewcmd);
		from = to;
	}
	if (bOK)
	{
		I::ClientState->m_NetChannel->m_nChokedPackets -= extraCommands;
		I::ClientState->m_NetChannel->SendNetMsg(moveMsg);
	}
}

