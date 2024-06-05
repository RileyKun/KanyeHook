#include "Events.h"

#include "../../Features/AntiHack/Resolver.h"

#include "../../Features/CritHack/CritHack.h"

__inline void log(const char* cFunction, const char* cLog, Color_t cColour) {
	I::Cvar->ConsoleColorPrintf(cColour, "[%s] ", cFunction);
	I::Cvar->ConsoleColorPrintf({ 255, 255, 255, 255 }, "%s\n", cLog);
}



void CEventListener::Setup(const std::deque<const char*>& deqEvents)
{
	if (deqEvents.empty())
		return;

	for (auto szEvent : deqEvents) {
		I::GameEventManager->AddListener(this, szEvent, false);

		if (!I::GameEventManager->FindListener(this, szEvent))
			throw std::runtime_error(tfm::format("failed to add listener: %s", szEvent));
	}
}

void CEventListener::Destroy()
{
	I::GameEventManager->RemoveListener(this);
}

void CEventListener::FireGameEvent(CGameEvent* pEvent) {
	if (pEvent == nullptr) { return; }

	const FNV1A_t uNameHash = FNV1A::Hash(pEvent->GetName());

	if (uNameHash == FNV1A::HashConst("player_hurt"))
	{
	
	}
}

