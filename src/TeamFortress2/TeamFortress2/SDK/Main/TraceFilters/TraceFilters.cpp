#include "TraceFilters.h"


bool CTraceFilterHitscan::ShouldHitEntity(void* pEntityHandle, int nContentsMask) {
	CBaseEntity* pEntity = reinterpret_cast< CBaseEntity* >( pEntityHandle );
	CBaseEntity* pLocal = reinterpret_cast< CBaseEntity* >( this->pSkip );

	switch ( pEntity->GetClassID( ) ) {
		case ETFClassID::CFuncAreaPortalWindow:
		case ETFClassID::CFuncRespawnRoomVisualizer:
		case ETFClassID::CSniperDot:
		case ETFClassID::CTFMedigunShield:
		case ETFClassID::CTFReviveMarker: {
			return false;
		}
	}

	if ( pLocal && pLocal->GetClassNum( ) == CLASS_SNIPER ) {
		switch ( pEntity->GetClassID( ) ) {
			case ETFClassID::CTFPlayer:
			case ETFClassID::CObjectSentrygun:
			case ETFClassID::CObjectDispenser:
			case ETFClassID::CObjectTeleporter:
			{
				if ( pLocal->GetTeamNum( ) == pEntity->GetTeamNum( ) )
					return false;
			} break;
			default:
			break;
		}
	}

	return ( pEntityHandle != pSkip );
}

ETraceType CTraceFilterHitscan::GetTraceType( ) const {
	return TRACE_EVERYTHING;
}
bool CTraceFilterWorldAndPropsOnly::ShouldHitEntity(void* pEntityHandle, int nContentsMask) {
	CBaseEntity* pEntity = reinterpret_cast<CBaseEntity*>(pEntityHandle);

	switch (pEntity->GetClassID()) {
	case ETFClassID::CFuncAreaPortalWindow:
	case ETFClassID::CFuncRespawnRoomVisualizer:
	case ETFClassID::CSniperDot:
	case ETFClassID::CTFMedigunShield:
	case ETFClassID::CTFReviveMarker:
	case ETFClassID::CTFAmmoPack:
	case ETFClassID::CPhysicsProp:
	case ETFClassID::CPhysicsPropMultiplayer:
	case ETFClassID::CDynamicProp:
	case ETFClassID::CTFObjectiveResource:
	case ETFClassID::CObjectSentrygun:
	case ETFClassID::CObjectDispenser:
	case ETFClassID::CObjectTeleporter:
	case ETFClassID::CTFProjectile_Arrow:
	case ETFClassID::CTFProjectile_BallOfFire:
	case ETFClassID::CTFProjectile_Cleaver:
	case ETFClassID::CTFProjectile_Jar:
	case ETFClassID::CTFProjectile_JarMilk:
	case ETFClassID::CTFProjectile_EnergyBall:
	case ETFClassID::CTFProjectile_EnergyRing:
	case ETFClassID::CTFProjectile_Flare:
	case ETFClassID::CTFProjectile_HealingBolt:
	case ETFClassID::CTFProjectile_MechanicalArmOrb:
	case ETFClassID::CTFProjectile_Rocket:
	case ETFClassID::CTFProjectile_SentryRocket:
	case ETFClassID::CTFGrenadePipebombProjectile:
	case ETFClassID::CTFPlayer:
	case ETFClassID::CMerasmus:
	case ETFClassID::CMerasmusDancer:
	case ETFClassID::CEyeballBoss:
	case ETFClassID::CHeadlessHatman:
	case ETFClassID::CZombie:
	case ETFClassID::CBaseDoor:
	case ETFClassID::CBaseObject:
	case ETFClassID::CBasePropDoor:
	case ETFClassID::CBaseProjectile:
	case ETFClassID::CBreakableProp:
	case ETFClassID::CBreakableSurface:
	case ETFClassID::CObjectCartDispenser:
	{
		return false;
	} break;
	default:
		break;
	}

	return !(pEntityHandle == pSkip);
}

ETraceType CTraceFilterWorldAndPropsOnly::GetTraceType() const {
	return TRACE_EVERYTHING;
}


bool CTraceFilterStuff::ShouldHitEntity(void* pEntityHandle, int nContentsMask) {
	CBaseEntity* pEntity = reinterpret_cast<CBaseEntity*>(pEntityHandle);

	switch (pEntity->GetClassID()) {
	case ETFClassID::CFuncAreaPortalWindow:
	case ETFClassID::CFuncRespawnRoomVisualizer:
	case ETFClassID::CSniperDot:
	case ETFClassID::CTFMedigunShield:
	case ETFClassID::CTFReviveMarker:
	case ETFClassID::CTFAmmoPack:
	case ETFClassID::CPhysicsProp:
	case ETFClassID::CPhysicsPropMultiplayer:
	case ETFClassID::CDynamicProp:
	case ETFClassID::CTFObjectiveResource:
	case ETFClassID::CObjectSentrygun:
	case ETFClassID::CObjectDispenser:
	case ETFClassID::CObjectTeleporter:
	case ETFClassID::CTFProjectile_Arrow:
	case ETFClassID::CTFProjectile_BallOfFire:
	case ETFClassID::CTFProjectile_Cleaver:
	case ETFClassID::CTFProjectile_Jar:
	case ETFClassID::CTFProjectile_JarMilk:
	case ETFClassID::CTFProjectile_EnergyBall:
	case ETFClassID::CTFProjectile_EnergyRing:
	case ETFClassID::CTFProjectile_Flare:
	case ETFClassID::CTFProjectile_HealingBolt:
	case ETFClassID::CTFProjectile_MechanicalArmOrb:
	case ETFClassID::CTFProjectile_Rocket:
	case ETFClassID::CTFProjectile_SentryRocket:
	case ETFClassID::CTFGrenadePipebombProjectile:
	case ETFClassID::CTFPlayer:
	case ETFClassID::CMerasmus:
	case ETFClassID::CMerasmusDancer:
	case ETFClassID::CEyeballBoss:
	case ETFClassID::CHeadlessHatman:
	case ETFClassID::CZombie:
	case ETFClassID::CBaseDoor:
	case ETFClassID::CBaseObject:
	case ETFClassID::CBasePropDoor:
	case ETFClassID::CBaseProjectile:
	case ETFClassID::CBreakableProp:
	case ETFClassID::CBreakableSurface:
	case ETFClassID::CObjectCartDispenser:
	{
		return false;
	} break;
	default:
		break;
	}

	return !(pEntityHandle == pSkip);
}

ETraceType CTraceFilterStuff::GetTraceType() const {
	return TRACE_WORLD_ONLY;
}







