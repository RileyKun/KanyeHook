#include "Prediction.h"

int CEnginePrediction::GetTickbase(CUserCmd* pCmd, CBaseEntity* pLocal)
{
	static int nTick = 0;
	static CUserCmd* pLastCmd = nullptr;

	if (pCmd)
	{
		if (!pLastCmd || pLastCmd->hasbeenpredicted)
			nTick = pLocal->GetTickBase();

		else nTick++;

		pLastCmd = pCmd;
	}

	return nTick;
}

void CEnginePrediction::Start(CUserCmd* pCmd)
{
	CBaseEntity* pLocal = g_EntityCache.GetLocal();

	if (pLocal && pLocal->IsAlive() && I::MoveHelper)
	{
	
		pLocal->SetCurrentCmd(pCmd);

		*I::RandomSeed = MD5_PseudoRandom(pCmd->command_number) & std::numeric_limits<int>::max();

		const bool bOldIsFirstPrediction = I::Prediction->m_bFirstTimePredicted;
		const bool bOldInPrediction = I::Prediction->m_bInPrediction;

		I::Prediction->m_bFirstTimePredicted = false;
		I::Prediction->m_bInPrediction = true;
		I::EffectsClient->SetSuppressEvent(true);

		I::MoveHelper->SetHost(pLocal);

		I::Prediction->RunCommand(pLocal, pCmd, I::MoveHelper);

		I::MoveHelper->SetHost(NULL);

		I::EffectsClient->SetSuppressEvent(false);

		I::Prediction->m_bInPrediction = bOldInPrediction;
		I::Prediction->m_bFirstTimePredicted = bOldIsFirstPrediction;

	}
}

void CEnginePrediction::End(CUserCmd* pCmd)
{
	CBaseEntity* pLocal = g_EntityCache.GetLocal();

	if (pLocal && pLocal->IsAlive() && I::MoveHelper)
	{
		pLocal->SetCurrentCmd(nullptr);
		*I::RandomSeed = -1;
	}
}