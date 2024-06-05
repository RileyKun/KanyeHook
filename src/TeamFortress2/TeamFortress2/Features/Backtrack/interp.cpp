#include "interp.h"


float CInterp::GetLerpTime()
{
	static auto sv_minupdaterate = g_ConVars.FindVar("sv_minupdaterate");
	static auto sv_maxupdaterate = g_ConVars.FindVar("sv_maxupdaterate");
	static auto cl_updaterate = g_ConVars.FindVar("cl_updaterate");
	static auto cl_interpolate = g_ConVars.FindVar("cl_interpolate");
	static auto cl_interp_ratio = g_ConVars.FindVar("cl_interp_ratio");
	static auto cl_interp = g_ConVars.FindVar("cl_interp");
	static auto sv_client_min_interp_ratio = g_ConVars.FindVar("sv_client_min_interp_ratio");
	static auto sv_client_max_interp_ratio = g_ConVars.FindVar("sv_client_max_interp_ratio");

	int nUpdateRate = std::clamp(cl_updaterate->GetInt(), (int)sv_minupdaterate->GetFloat(), (int)sv_maxupdaterate->GetFloat());
	float flLerpTime = 0.f;

	bool bUseInterpolation = cl_interpolate->GetInt() != 0;

	if (bUseInterpolation)
	{
		float flLerpRatio = cl_interp_ratio->GetFloat();
		if (flLerpRatio == 0)
		{
			flLerpRatio = 1.f;
		}

		float flLerpAmount = cl_interp->GetFloat();

		if (sv_client_min_interp_ratio && sv_client_max_interp_ratio && sv_client_min_interp_ratio->GetFloat() != -1.f)
		{
			flLerpRatio = std::clamp(flLerpRatio, sv_client_min_interp_ratio->GetFloat(), sv_client_max_interp_ratio->GetFloat());
		}
		else
		{
			if (flLerpRatio == 0.f)
			{
				flLerpRatio = 1.f;
			}
		}

		flLerpTime = std::max(flLerpAmount, flLerpRatio / nUpdateRate);
	}
	else
	{
		flLerpTime = 0.f;
	}

	return flLerpTime;
}

float CInterp::GetLowestPossibleLerpTime(int* nUpdateRate)
{
	static auto sv_maxupdaterate = g_ConVars.FindVar("sv_maxupdaterate");
	static auto cl_interp_ratio = g_ConVars.FindVar("cl_interp_ratio");

	if (nUpdateRate)
	{
		*nUpdateRate = sv_maxupdaterate->GetInt();
	}

	return cl_interp_ratio->GetInt() / sv_maxupdaterate->GetInt();
}

int CInterp::EstimateServerArriveTick()
{
	int nTick = I::GlobalVars->tickcount + 1;

	auto pNetChan = I::EngineClient->GetNetChannelInfo();
	if (pNetChan)
	{
		nTick += TIME_TO_TICKS(pNetChan->GetLatency(FLOW_INCOMING) + pNetChan->GetLatency(FLOW_OUTGOING));
	}

	return nTick;
}

bool CInterp::CanRestoreToSimulationTime(float flSimulationTime, bool* bNeedToAdjustInterp)
{
	float correct = 0.f;

	auto pNetChan = I::EngineClient->GetNetChannelInfo();
	if (pNetChan)
	{
		correct += pNetChan->GetLatency(FLOW_OUTGOING);
	}

	int lerpTicks;

	lerpTicks = GetLerpTime();

	// not needed, we either adjust cl_interp or disable cl_interpolate[smart], except on newer src games where we cant but i add that later

	correct += TICKS_TO_TIME(TIME_TO_TICKS(lerpTicks));

	if (correct >= 1.f)
	{
		return false;
	}
	else if (correct < 0.f)
	{
		correct = 0.f;
	}

	float flDiff = TICKS_TO_TIME(EstimateServerArriveTick() - TIME_TO_TICKS(flSimulationTime));
	if (flDiff >= 1.f)
	{
		return false;
	}

	int targettick =TIME_TO_TICKS(flSimulationTime);
	float deltaTime = correct - TICKS_TO_TIME(EstimateServerArriveTick() - targettick);

	if (fabsf(deltaTime) >= TICKS_TO_TIME(15))
	{
		if (flDiff >= 0.f)
		{
			if (bNeedToAdjustInterp != nullptr)
			{
				*bNeedToAdjustInterp = true;
			}


			return true;
		}

		return false;
	}

	if (bNeedToAdjustInterp != nullptr)
	{
		*bNeedToAdjustInterp = false;
	}

	return true;
}


/*
void CInterp::GetUsableRecordsForEntity(CBaseEntity* pEntity, std::deque<TickRecord*>* vecRecords, float flMinTime, float flMaxTime, bool* bShouldLagFix)
{
	return GetUsableRecordsForIndex(pEntity->GetIndex() - 1, vecRecords, flMinTime, flMaxTime, bShouldLagFix);
}




bool  CInterp::IsDeltaTooBig(Vector& vPos1, Vector& vPos2)
{
	return (vPos1 - vPos2).Length2DSqr() > 4096.f;
}

void CInterp::GetUsableRecordsForIndex(int idx, std::deque<TickRecord>* vecRecords, float flMinTime, float flMaxTime, bool* bShouldLagFix)
{
	bool bAdjustInterp;
	auto& list = vecRecords[idx];
	if (list.size() == 0)
	{
		return;
	}

	auto pNode = list.front();
	Vector vPrevOrigin = pNode.AbsOrigin;

	bool bFirstRecordValid = false;
	if (list.size() >= 2)
	{
		if (CanRestoreToSimulationTime(pNode.SimulationTime, &bAdjustInterp))
		{
			bFirstRecordValid = !IsDeltaTooBig(vPrevOrigin, pNode.AbsOrigin);
		}
	}

	while (pNode != nullptr)
	{
		auto pPrev = pNode;

		if (flMaxTime != -1.f)
		{
			if (flMinTime == -1.f)
			{
				flMinTime = 0.f;
			}

			static auto cl_updaterate = g_ConVars.FindVar("cl_updaterate");
			float flSnapshotInterval = 1.f / cl_updaterate->GetFloat();
			float flDelta = TICKS_TO_TIME(I::GlobalVars->tickcount) - pNode->m_pThis->m_flArriveTime;

			if (flDelta < flMinTime)
			{
				if (IsDeltaTooBig(*vPrevOrigin, pNode->m_pThis->m_vecOrigin))
				{
					if (bFirstRecordValid)
					{
						vecRecords->Add(list.GetTail()->m_pThis);
					}

					if (bShouldLagFix != nullptr) // need to do more elaborate checks here
					{
						*bShouldLagFix = !bFirstRecordValid;
					}

					break;
				}

				vPrevOrigin = &pNode->m_pThis->m_vecOrigin;
				pNode = pPrev;
				continue;
			}

			if (flDelta > (flMaxTime + flSnapshotInterval))
			{
				break;
			}

			if (!vPrevOrigin)
			{
				vPrevOrigin = &pNode->m_pThis->m_vecOrigin;
			}
		}

		if (!CanRestoreToSimulationTime(pNode->m_pThis->m_flSimulationTime, &bAdjustInterp))
		{
			if (bShouldLagFix != nullptr)
			{
				*bShouldLagFix = vecRecords->GetSize() == 0;
			}

			break;
		}
#if 0
		if (IsDeltaTooBig(*vPrevOrigin, pNode->m_pThis->m_vecOrigin))
		{
			if (vecRecords->GetSize() == 0 && bFirstRecordValid)
			{
				vecRecords->Add(list.GetTail()->m_pThis);
			}

			if (bShouldLagFix != nullptr) // need to do more elaborate checks here
			{
				*bShouldLagFix = !bFirstRecordValid;
			}

			break;
		}
#endif

		vecRecords->Add(pNode->m_pThis);

		vPrevOrigin = &pNode->m_pThis->m_vecOrigin;
		pNode = pPrev;
	}
}*/