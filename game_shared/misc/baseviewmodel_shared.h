//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef BASEVIEWMODEL_SHARED_H
#define BASEVIEWMODEL_SHARED_H
#ifdef _WIN32
#pragma once
#endif

#include "predictable_entity.h"
#include "utlvector.h"
#include "baseplayer_shared.h"
#include "shared_classnames.h"

//=========================================================
//=========================================================

// PNOTE: 
// * reformatted
// * removed VGUI stuff
// * removed multiple viewmodel support

//=========================================================
//=========================================================
class CBaseCombatWeapon;

//=========================================================
//=========================================================
#ifdef CLIENT_DLL

#define CBaseViewModel C_BaseViewModel
#define CBaseCombatWeapon C_BaseCombatWeapon

#endif

//=========================================================
//=========================================================
class CBaseViewModel : public CBaseAnimating
{
	DECLARE_CLASS( CBaseViewModel, CBaseAnimating );
public:

	DECLARE_NETWORKCLASS( );
	DECLARE_PREDICTABLE( );

#ifdef GAME_DLL

	DECLARE_DATADESC( );

#endif

	CBaseViewModel( );
	virtual ~CBaseViewModel( );

	bool IsViewable( void ) { return false; }

	virtual void UpdateOnRemove( void );

	// weapon client handling
	virtual void SendViewModelMatchingSequence( int sequence );
	virtual void SetWeaponModel( const char *pszModelname, CBaseCombatWeapon *weapon );

	virtual void CalcViewModelView( CBasePlayer *owner, const Vector &eyePosition, const QAngle &eyeAngles ) { }

	void SetOwner( CBasePlayer *pPlayer );

	virtual void Precache( void );

	virtual void Spawn( void );

	virtual CBasePlayer *GetOwner( void ) { return m_hOwner; };

	virtual CBaseCombatWeapon *GetOwningWeapon( void );

	virtual bool IsSelfAnimating( void ) { return true; }

#ifdef GAME_DLL

	virtual int UpdateTransmitState( void );
	virtual int ShouldTransmit( const CCheckTransmitInfo *pInfo );
	virtual void SetTransmit( CCheckTransmitInfo *pInfo, bool bAlways );

#else

	virtual RenderGroup_t GetRenderGroup( void );

	virtual void FireEvent( const Vector& origin, const QAngle& angles, int event, const char *options );

	virtual void OnDataChanged( DataUpdateType_t updateType );
	virtual void PostDataUpdate( DataUpdateType_t updateType );

	virtual bool Interpolate( float currentTime );

	void UpdateAnimationParity( void );

	virtual bool ShouldDraw( void );
	virtual int DrawModel( int flags );
	int DrawOverriddenViewmodel( int flags );
	virtual int GetFxBlend( void );
	virtual bool IsTransparent( void );
	
	// should this object cast shadows?
	virtual ShadowType_t ShadowCastType( void ) { return SHADOWS_NONE; }

	// should this object receive shadows?
	virtual bool ShouldReceiveProjectedTextures( int flags ) { return false; }

	// add entity to visible view models list?
	virtual void AddEntity( void );

	virtual void GetBoneControllers( float controllers[ MAXSTUDIOBONECTRLS ] );

	// see C_StudioModel's definition of this
	virtual void UncorrectViewModelAttachment( Vector &vOrigin );

	// stuff inherited from C_BaseAnimating
	virtual void FormatViewModelAttachment( int nAttachment, Vector &vecOrigin, QAngle &angle );
	virtual bool IsViewModel( void ) const;
	
	CBaseCombatWeapon *GetWeapon( void ) const { return m_hWeapon.Get( ); }

private:
	CBaseViewModel( const CBaseViewModel & );

#endif

private:
	CNetworkHandle( CBasePlayer, m_hOwner );

	// soonest time Update will call WeaponIdle
	float m_flTimeWeaponIdle;							

	Activity m_Activity;

	// used to force restart on client, only needs a few bits
	CNetworkVar( int, m_nAnimationParity );

	// weapon art
	string_t m_sVMName;
	string_t m_sAnimationPrefix;

#ifdef CLIENT_DLL

	int m_nOldAnimationParity;

#endif

	typedef CHandle< CBaseCombatWeapon > CBaseCombatWeaponHandle;
	CNetworkVar( CBaseCombatWeaponHandle, m_hWeapon );
};

#endif // BASEVIEWMODEL_SHARED_H