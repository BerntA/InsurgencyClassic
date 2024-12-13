//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef WEAPON_INS_BASE_H
#define WEAPON_INS_BASE_H
#ifdef _WIN32
#pragma once
#endif

#include "weapon_defines.h"
#include "ins_shared.h"

//=========================================================
//=========================================================

// TODO: Jeremy hasn't got a standard naming scheme for his weapons so
// quite a few animations are missing a "_" so this is why there are so many
// functions that handle anim names.
// Also, the way in which a weapon handles the while "_EMPTY" thing needs to be cleaned
// up because both the Pistols, SKS and RPG share this and they have the same code sooo
// thats kinda gay.

//=========================================================
//=========================================================
class CINSPlayer;
class CWeaponClip;

//=========================================================
//=========================================================
#ifdef CLIENT_DLL

#define CWeaponINSBase C_WeaponINSBase

#endif

//=========================================================
//=========================================================
class INSWeaponInfo_t : public FileWeaponInfo_t
{
	typedef FileWeaponInfo_t BaseClass;

public:
	INSWeaponInfo_t( );

	virtual void Parse( KeyValues *pKeyValuesData, const char *pszWeaponName );

public:
	int iIronsightFOV;
	float flFreeaimDistanceFactor;
	float flWeaponLagFactor;

	int iViewmodelFOV;
    int iIronsightViewmodelFOV;

	bool bHasScope;
    int iScopeFOV;

#ifdef CLIENT_DLL

	bool bPrecachedResources;

	char szAmmoTex[ MAX_WEAPON_STRING ];
	int iAmmoTexID;
	
#endif

};

//=========================================================
//=========================================================
enum WeaponState_t
{
	WIDLESTATE_NORMAL = 0,
	WIDLESTATE_DOWN,
	WIDLESTATE_CRAWL
};

//=========================================================
//=========================================================
class CWeaponINSBase : public CBaseCombatWeapon
{
	DECLARE_CLASS(CWeaponINSBase, CBaseCombatWeapon);
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

public:
	CWeaponINSBase( );

	CINSPlayer *GetINSPlayerOwner(void) const;

	virtual void Spawn( void );
	virtual void Precache( void );

	void SetPlayerSlot( int iSlot ) { m_iPlayerSlot = iSlot; }
	int GetPlayerSlot( void ) const { return m_iPlayerSlot; }

	virtual bool CanDeploy(void);
	virtual void HandleDeploy(void);
	virtual void OnDeployReady( void );

	bool Holster(CBaseCombatWeapon *pSwitchingTo = NULL);

#ifdef GAME_DLL
	void RegisterShot(void);

	void Delete( void );
#endif

	bool FindMuzzle( Vector &vecOrigin, Vector &vecForward, bool bForceEye );

	virtual bool CanReload( void );
	virtual void StartReload(void);

	virtual void DoAnimationEvent(int iEvent);

	virtual void ItemPostFrame(void);

	void ToggleIronsights(void);
	virtual void SetIronsights(bool bState, bool bForce);

	virtual bool CanAttack( void );

	bool CanTertiaryAttack( void );
	void TertiaryAttack( void );

	virtual Activity GetPrimaryAttackActivity(void) const;

	virtual Activity GetWeaponIdleActivity(void) const;

	virtual bool CanUseIronsights(void);
	virtual Activity GetIronsightInActivity(void) const;
	virtual Activity GetIronsightOutActivity(void) const;

	virtual bool AllowViewmodelInteraction( void ) { return true; }
	virtual bool LowerViewmodelInteraction( void ) { return false; }

	void CalcViewmodelInteraction( float &flFraction, Vector *pForward );
	void CalcViewmodelInteraction( Vector &vecOrigin, QAngle &angAngles );
	virtual void CalcViewmodelInteraction( float &flLength, float &flHalfWidth );

	virtual bool ShouldHideWeapon(void);

	void CalcViewmodelBob( void );

	virtual void UpdateIdleState( void );
	int GetIdleState(void) const { return m_iIdleState; }

	float GetWeaponLagFactor( bool bIronsights );

#ifdef CLIENT_DLL

	virtual bool UseFreeaim( void ) const { return false; }
	bool AllowFreeaim( void ) const;
	float GetFreeaimDistance( void ) const;

	virtual bool HasScope( void ) const;

	virtual int GetAmmoTexID( void ) const;

#endif

#ifdef GAME_DLL

	virtual bool SendIronsightsHint( void ) const { return true; }

#endif

	virtual int GetActiveFiremode( void ) const;

	virtual void ForceReady( void );

	virtual Activity GetCrawlActivity(void) const;
	virtual Activity GetDownActivity(void) const;
	virtual Activity GetNormalActivity(void) const;

	virtual void OnDrop( void );

	virtual bool AllowLeaning( void ) { return false; }

	virtual bool AllowBreathing( void ) { return false; }

	virtual bool AllowSprintStart( void ) { return true; }

	virtual bool AllowJump( void ) const { return true; }
	virtual bool AllowJumpAttack( void ) const { return false; }
	bool TestJumpAttack( void ) const;

#ifdef GAME_DLL

	CBasePlayer *GetScorer( void ) const;
	virtual int GetInflictorType( void ) const;
	virtual int GetInflictorID( void ) const;

#endif

	virtual bool AllowPlayerStance( int iFromStance, int iToStance );

	virtual bool AllowPlayerMovement( void ) const;

protected:
/*#ifdef CLIENT_DLL
	void UpdateMuzzle(void);
#endif*/

	virtual void SetIronsightsState( bool bState );

#ifdef GAME_DLL

	void SetPlayerIronsightFOV( bool bState, float flTime );

#else

	virtual void PrecacheResources( void );

	void LoadAmmoTex( const char *pszAmmoTex, int &iAmmoTexID );

	bool DoClientEffects( void ) const;

#endif

private:
	CNetworkVar( int, m_iPlayerSlot );

	CNetworkVar( int, m_iIdleState );

#ifdef CLIENT_DLL

	float m_flFreeaimDistance;
	float m_flHippedLagFactor, m_flIronsightLagFactor;

#endif
};

//=========================================================
//=========================================================
inline CWeaponINSBase *ToINSWeapon( CBaseCombatWeapon *pWeapon )
{
	// NOTE: every weapon should be a INS weapon
	return static_cast< CWeaponINSBase* >( pWeapon );
}

//=========================================================
//=========================================================
#ifdef CLIENT_DLL

extern C_WeaponINSBase *GetINSActiveWeapon( void );

#endif

#endif // WEAPON_INS_BASE_H
