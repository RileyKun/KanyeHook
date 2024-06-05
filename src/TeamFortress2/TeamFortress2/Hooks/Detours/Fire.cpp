#include "../Hooks.h"
#include "../../Features/Commands/Commands.h"
#include "../../Features/Visuals/Visuals.h"

struct FireBulletsInfo_t
{
	FireBulletsInfo_t()
	{
		m_iShots = 1;
		m_vecSpread.Clear();
		m_flDistance = 8192;
		m_iTracerFreq = 4;
		m_flDamage = 0;
		m_iPlayerDamage = 0;
		m_pAttacker = nullptr;
		m_nFlags = 0;
		m_pAdditionalIgnoreEnt = nullptr;
		m_flDamageForceScale = 1.0f;
		m_bPrimaryAttack = true;
		m_bUseServerRandomSeed = false;
	}

	FireBulletsInfo_t(int nShots, const Vec3& vecSrc, const Vec3& vecDir, const Vec3& vecSpread, float flDistance,
		int nAmmoType, bool bPrimaryAttack = true)
	{
		m_iShots = nShots;
		m_vecSrc = vecSrc;
		m_vecDirShooting = vecDir;
		m_vecSpread = vecSpread;
		m_flDistance = flDistance;
		m_iAmmoType = nAmmoType;
		m_iTracerFreq = 2;
		m_flDamage = 0;
		m_iPlayerDamage = 0;
		m_pAttacker = nullptr;
		m_nFlags = 0;
		m_pAdditionalIgnoreEnt = nullptr;
		m_flDamageForceScale = 1.0f;
		m_bPrimaryAttack = bPrimaryAttack;
		m_bUseServerRandomSeed = false;
	}

	int m_iShots = 0;
	Vec3 m_vecSrc = {};
	Vec3 m_vecDirShooting = {};
	Vec3 m_vecSpread = {};
	float m_flDistance = 0.f;
	int m_iAmmoType = 0;
	int m_iTracerFreq = 0;
	float m_flDamage = 0.f;
	int m_iPlayerDamage = 0; // Damage to be used instead of m_flDamage if we hit a player
	int m_nFlags = 0; // See FireBulletsFlags_t
	float m_flDamageForceScale = 0.f;
	CBaseEntity* m_pAttacker = nullptr;
	CBaseEntity* m_pAdditionalIgnoreEnt = nullptr;
	bool m_bPrimaryAttack = false;
	bool m_bUseServerRandomSeed = false;
};

using LookupAttachment_t = int(*)(CBaseEntity*, const char*);

void DrawBeam(const Vector& source, const Vector& end)
{
	BeamInfo_t beamInfo;


	beamInfo.m_nType = 0;
	beamInfo.m_pszModelName = Vars::Visuals::Beans::UseCustomModel.Value ? Vars::Visuals::Beans::Model.c_str() : "sprites/purplelaser1.vmt";
	beamInfo.m_nModelIndex = -1; // will be set by CreateBeamPoints if its -1
	beamInfo.m_flHaloScale = 0.0f;
	beamInfo.m_flLife = 2;
	beamInfo.m_flWidth = 2;
	beamInfo.m_flEndWidth = 2;
	beamInfo.m_flFadeLength = 0;
	beamInfo.m_flAmplitude = 2;
	beamInfo.m_flBrightness = 255;
	beamInfo.m_flSpeed = 2.0;
	beamInfo.m_nStartFrame = 0;
	beamInfo.m_flFrameRate = 0;
	beamInfo.m_flRed = 255;
	beamInfo.m_flGreen = 255;
	beamInfo.m_flBlue = 255;
	beamInfo.m_nSegments = Vars::Visuals::Beans::Segments.Value;
	beamInfo.m_bRenderable = true;
	beamInfo.m_nFlags = Vars::Visuals::Beans::Flags.Value;
	beamInfo.m_vecStart = source;
	beamInfo.m_vecEnd = end;

	Beam_t* coolBeam = I::ViewRenderBeams->CreateBeamPoints(beamInfo);
	if (coolBeam)
	{
		I::ViewRenderBeams->DrawBeam(coolBeam);
	}
}

#include "../../Features/Prediction/Prediction.h"


MAKE_HOOK(C_BaseEntity_FireBullets, g_Pattern.Find(L"client.dll", L"55 8B EC 81 EC ? ? ? ? 53 56 57 8B F9 8B 5D"), void, __fastcall,
	void* ecx, void* edx, CBaseCombatWeapon* pWeapon, const FireBulletsInfo_t& info, bool bDoEffects, int nDamageType, int nCustomDamageType)
{


	if (!pWeapon || (!Vars::Visuals::ParticleTracer.Value && !Vars::Visuals::BulletTracer.Value && !Vars::Visuals::Beans::Active.Value))
	{
		return Hook.Original<FN>()(ecx, edx, pWeapon, info, bDoEffects, nDamageType, nCustomDamageType);
	}
	if (const auto& pLocal = g_EntityCache.GetLocal() )
	{
		const Vec3 vStart = info.m_vecSrc;
		const Vec3 vEnd = vStart + info.m_vecDirShooting * info.m_flDistance;

		CGameTrace trace = {};
		CTraceFilterHitscan filter = {};
		filter.pSkip = pLocal;

		/* if ur shooting thru stuff, change MASK_SHOT to MASK_SOLID - myzarfin */
		Utils::Trace(vStart, vEnd, MASK_VISIBLE, &filter, &trace);

		//I::Prediction->RunCommand(pLocal, G::LastUserCmd, I::MoveHelper); //maybe?

		const int iAttachment = pWeapon->LookupAttachment("muzzle");
		pWeapon->GetAttachment(iAttachment, trace.vStartPos);


		if (!pLocal->IsInValidTeam())
		{
			return;
		}

		const int team = pLocal->GetTeamNum();
		switch (Vars::Visuals::ParticleTracer.Value)
		{
			//Machina
		case 1:
		{
			Particles::ParticleTracer("dxhr_sniper_rail", trace.vStartPos,
				trace.vEndPos, pLocal->GetIndex(), iAttachment, true);
			break;
		}
			//C.A.P.P.E.R
		case 2:
			
				Particles::ParticleTracer(team == TEAM_RED ? "bullet_tracer_raygun_red_crit" : "bullet_tracer_raygun_blue_crit", trace.vStartPos,
					trace.vEndPos, pLocal->GetIndex(), iAttachment, true);
			
			break;


			//Short circuit
		case 3:
		
				Particles::ParticleTracer(team == TEAM_RED ? "dxhr_lightningball_hit_zap_red" : "dxhr_lightningball_hit_zap_blue",
					trace.vStartPos, trace.vEndPos, pLocal->GetIndex(), iAttachment, true);
			
			break;

			//Merasmus ZAP
		case 4:
		
				Particles::ParticleTracer("bullet_tracer02_blue", trace.vStartPos, trace.vEndPos, pLocal->GetIndex(), iAttachment, true);
			
			break;

		case 5:
			if (!pLocal->IsCritBoostedNoMini())
			{
				Particles::ParticleTracer(team == TEAM_RED ? "bullet_bignasty_tracer01_blue" : "bullet_bignasty_tracer01_red",
					trace.vStartPos, trace.vEndPos, pLocal->GetIndex(), iAttachment, true);
			}
			break;

		case 6:
			if (!pLocal->IsCritBoostedNoMini())
			{
				Particles::ParticleTracer("tfc_sniper_distortion_trail", trace.vStartPos, trace.vEndPos, pLocal->GetIndex(),
					iAttachment, true);
			}
			break;

		case 7: // black_ink, demo'd: https://youtu.be/Ba0lcMOfm9w 
			if (!pLocal->IsCritBoostedNoMini())
			{
				Particles::ParticleTracer("merasmus_zap_beam01", trace.vStartPos, trace.vEndPos, pLocal->GetIndex(), iAttachment,
					true);
			}
			break;

		case 8:
			// custom particle tracer, def not pasted from deathpole or anything. list @ dump_particlemanifest or @ https://github.com/tf2cheater2013/particles.txt
			if (!pLocal->IsCritBoostedNoMini())
			{
				Particles::ParticleTracer(Vars::Visuals::ParticleName.c_str(), trace.vStartPos, trace.vEndPos, pLocal->GetIndex(), iAttachment,
					true);
			}
			break;
		}

		if (Vars::Visuals::Beans::Active.Value)
		{
			DrawBeam(trace.vStartPos, trace.vEndPos);
		}
	}
	

}