//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef WEAPON_BALLISTICBASE_H
#define WEAPON_BALLISTICBASE_H
#ifdef _WIN32
#pragma once
#endif

#if defined( CLIENT_DLL )
#define CWeaponBallisticBase C_WeaponBallisticBase
#endif

#include "weapon_ins_base.h"
#include "firebullets.h"

struct Clip_t;

//=========================================================
//=========================================================
#define MAX_CLIPS_PRIMARY 5
#define MAX_CLIPS_SECONDARY 3

//=========================================================
//=========================================================
struct BallisticRange_t
{
	BallisticRange_t( );

	int iEffectiveRange, iMaxRange;
	Vector vecSpread;
};

//=========================================================
//=========================================================
class BallisticWeaponInfo_t : public INSWeaponInfo_t
{
	typedef INSWeaponInfo_t BaseClass;

public:
	BallisticWeaponInfo_t( );

protected:
	virtual void Parse( KeyValues *pKeyValuesData, const char *pszWeaponName );

private:
	void CalculateRange( KeyValues *pData, const char *pszDataName, BallisticRange_t &Range, BallisticRange_t &RangeIronsights );
	void CalculateEffectiveRange( BallisticRange_t &Range );

public:
	int iClipType;

	int iMuzzleVelocity, iMuzzleVelocityRL;

	BallisticRange_t Range, RangeIronsights;
	BallisticRange_t RangeRL, RangeIronsightsRL;

	float flCycleTime;
	float flBurstCycleTime;

	int iFiremodes;

	int iTracerType;

	float flRecoilXMin, flRecoilXMax, flRecoilYMin, flRecoilYMax;
	float flRecoilXMinBipod, flRecoilXMaxBipod, flRecoilYMinBipod, flRecoilYMaxBipod;
};

//=========================================================
//=========================================================
#define BULLETS_3RNDBURST 3

//=========================================================
//=========================================================
class CWeaponBallisticBase : public CWeaponINSBase, public IFireBullets
{
	DECLARE_CLASS( CWeaponBallisticBase, CWeaponINSBase );

public:
	DECLARE_NETWORKCLASS( );
	DECLARE_PREDICTABLE( );

public:
	CWeaponBallisticBase( );

	// weapon behaviour
	virtual void Spawn( void );

	bool CanDeploy( void );
	void HandleDeploy( void );
	void OnDeployReady( void );

	virtual void ItemPostFrame( void );

	virtual Activity GetDrawActivity( void );
	virtual Activity GetDryFireActivity( void ) const;

#ifdef CLIENT_DLL

	model_t *GetShellModel( void ) const;

	bool UseFreeaim( void ) const { return true; }

#endif

	bool ShouldEmptyAnimate( void ) const;

	bool AllowLeaning( void ) { return true; }
	bool AllowBreathing( void ) { return true; }

	// attacking
	virtual bool IsEmptyAttack( void );
	virtual void EmptyAttack( void );

	bool HasPrimaryAttack( void ) { return true; }
	virtual bool CanPrimaryAttack( void );
	virtual bool StoreLastPrimaryAttack( void );
	virtual void PrimaryAttack( void );

	virtual Activity GetPrimaryAttackActivity( void ) const;
	virtual Activity GetPrimaryAttackActivityRecoil( Activity BaseAct, int iRecoilState ) const;

	// ... IFireBullet
	CBaseEntity *GetAttacker( void );
	CBaseEntity *GetInflictor( void );
	int GetBulletType( void ) const;
	int GetMuzzleVelocity( void ) const;
	Vector GetSpread( void ) const;
	int GetRange( void ) const;

	// ... overloads for weapons
	virtual int GetBullets( void ) const { return 1; }
	virtual bool IgnoreRange( void ) const { return false; }
	virtual float GetCycleTime( void );
	virtual float GetBurstCycleTime( void ) const;
	virtual bool ShowTracers( void ) const { return false; }
	virtual bool HasSlide( void ) const { return false; }

	// ... recoil
	void CreateRecoil( bool bStartedBurst );
	QAngle GetRecoil( bool bBipod );
	float GetRecoilMod( void ) const;

	// firemode control
	void ResetFiremodes( void );
	void SetFiremodes( int iFiremodes );
	void CycleFiremodes( void );
	bool IsValidFiremode( int iFiremode ) const;
	virtual int ForcedFiremode( void ) const;

	int GetActiveFiremode( void ) const { return m_iActiveFiremode; }
	bool HasMultipleFireModes( void ) const { return m_bHasMultipleFiremodes; }

	Activity GetFireModeActivity( void ) const;

	// reloading
	virtual bool CanReload( void );
	virtual void FinishReload( void );
	virtual bool IsEmptyReload( void );
	virtual void HandleEmptyReload(void);
	bool IsReloading( void ) const { return m_bInReload; }

	// clip management
	int GetClipType( void ) const;
	bool IsValidClip( void ) const;
	Clip_t *GetClipData( void ) const;

	CUtlVector< int > &GetClipList( void ) { return m_Clips; }

	bool HasClips( void ) const;
	int GetClipCount( void ) const;
	bool IsClipEmpty( void ) const;
	bool IsClipFull( void );
	int GetClipSize( void ) const;
	int TakeClip( void );
	int NextClip( void ) const;

	// bullet related
	virtual int GetShellType( void ) const;

	// ammo related
	virtual bool HasAmmo( void ) const;
	int GetAmmoCount( void ) const;

	void GiveClip( int iCount );

	// misc
	bool ShouldCock( void ) const;
	void Cock( void );

	bool IsEmpty( void ) const;
	bool IsLastBullet( void ) const;
	void StripRound( void );
	void ForceReady( void );

#ifdef CLIENT_DLL

	void PrintStats( void );
	void PrintDamageStats( int iRange );

#endif

protected:
	float m_flNextEmptySound;

	CNetworkVar( bool, m_bChamberedRound );
	CNetworkVar( int, m_iClip );

	CUtlVector< int > m_Clips;

	CNetworkVar( bool, m_bHammerDown );

	bool m_bFiremodes[ FIREMODE_COUNT ];
	bool m_bHasMultipleFiremodes;
	CNetworkVar( int, m_iActiveFiremode );
	float m_flNextFiremodeChange;

#ifdef GAME_DLL

	bool m_bFirstDeploy;

#endif
	
	CNetworkVar( float, m_flNextForcedPrimaryAttack );
	CNetworkVar( int, m_iRemainingForcedBullets );

	int m_iNumShotsFired;
	float m_flLastAttackTime;
};

//=========================================================
//=========================================================
inline CWeaponBallisticBase *ToBallisticWeapon(CBaseCombatWeapon *pWeapon)
{
	return dynamic_cast<CWeaponBallisticBase*>(pWeapon);
}

#endif // WEAPON_BALLISTICBASE_H
