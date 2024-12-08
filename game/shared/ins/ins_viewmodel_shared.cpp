//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "ins_viewmodel_shared.h"
#include "ins_player_shared.h"
#include "play_team_shared.h"
#include "ins_utils.h"
#include "weapon_ins_base.h"

#ifdef CLIENT_DLL

#include "iprediction.h"
#include "prediction.h"
#include "ivieweffects.h"

#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
#ifdef CLIENT_DLL

#define CINSViewModel C_INSViewModel

#endif

//=========================================================
//=========================================================
class CINSViewModel : public CBaseViewModel
{
	DECLARE_CLASS( CINSViewModel, CBaseViewModel );

public:
	DECLARE_NETWORKCLASS( );

	CINSViewModel( );
	~CINSViewModel( );

private:
	CINSViewModel( const CINSViewModel & );

	CWeaponINSBase *GetINSOwningWeapon( void );

	void SetWeaponModel( const char *pszModelname, CBaseCombatWeapon *weapon );
							
	void CalcViewModelView( CBasePlayer *owner, const Vector &eyePosition, const QAngle &eyeAngles );

#ifdef CLIENT_DLL

	void CalcViewModelLag( CWeaponINSBase *weapon, bool ironsights, Vector &origin, QAngle &angles, QAngle &original_angles );
	void CalcViewModelOffset( CINSPlayer *player, Vector &origin );
	
	bool ShouldPredict( void );

#endif

private:
	
#ifdef CLIENT_DLL

	CInterpolatedVar< QAngle > m_LagAnglesHistory;
	QAngle m_vLagAngles;

#endif
};

//=========================================================
//=========================================================
LINK_ENTITY_TO_CLASS( predicted_viewmodel, CINSViewModel );

//=========================================================
//=========================================================
IMPLEMENT_NETWORKCLASS_ALIASED( INSViewModel, DT_INSViewModel )

BEGIN_NETWORK_TABLE( CINSViewModel, DT_INSViewModel )

END_NETWORK_TABLE( )

//=========================================================
//=========================================================
CINSViewModel::CINSViewModel( )
{
#ifdef CLIENT_DLL

	m_vLagAngles.Init( );
	m_LagAnglesHistory.Setup( &m_vLagAngles, 0 );

#endif
}

//=========================================================
//=========================================================
CINSViewModel::~CINSViewModel( )
{
}

//=========================================================
//=========================================================
CWeaponINSBase *CINSViewModel::GetINSOwningWeapon( void )
{
	return static_cast< CWeaponINSBase* >( GetOwningWeapon( ) );
}

//=========================================================
//=========================================================
#define ARMS_GROUP "arms"

void CINSViewModel::SetWeaponModel( const char *pszModelname, CBaseCombatWeapon *weapon )
{
	BaseClass::SetWeaponModel( pszModelname, weapon );

	int iArmsGroup = FindBodygroupByName( ARMS_GROUP );

	if( iArmsGroup == -1 )
		return;

	CBasePlayer *pPlayer = GetOwner( );

	if( !pPlayer )
		return;

	CPlayTeam *pTeam = ToINSPlayer( pPlayer )->GetPlayTeam( );
	AssertMsg( pTeam, "CBaseViewModel::UpdateArms, Player has a ViewModel when not on a PlayTeam" );
	
	if( pTeam )
		SetBodygroup( iArmsGroup, pTeam->GetTeamLookupID( ) );
}

//=========================================================
//=========================================================
extern ConVar cl_pitchup;
extern ConVar cl_pitchdown;

void CINSViewModel::CalcViewModelView( CBasePlayer *owner, const Vector &eyePosition, const QAngle &eyeAngles )
{
	Assert( owner );

	QAngle vmangoriginal = eyeAngles;
	QAngle vmangles = eyeAngles;
	Vector vmorigin = eyePosition;

#ifdef CLIENT_DLL

	CINSPlayer *player = ToINSPlayer( owner );

	// minus head angles
	vmangles -= player->HeadAngles( );

	// allow weapon to change
	CWeaponINSBase *pWeapon = GetINSOwningWeapon( );

	if( !prediction->InPrediction( ) )
	{
		// handle lagging and interaction etc
		if( pWeapon != NULL )
		{
			bool ironsights = ( player->GetPlayerFlags( ) & FL_PLAYER_IRONSIGHTS ) != 0;

			// adjust vmangles
			vmangles += player->GetFreeAimAngles( ) / ( ironsights ? 5.0f : 2.5f );
			vmangles += player->GetBreathingAdjust( );

			if( ironsights )
				vmangles[ PITCH ] -= player->GetLookOver( );

			// clamp vmangles
			vmangles[ PITCH ] = clamp( vmangles[ PITCH ], -cl_pitchup.GetFloat( ), cl_pitchdown.GetFloat( ) );

			// apply other adjustments
			pWeapon->AddViewmodelBob( this, vmorigin, vmangles );
			CalcViewModelLag( pWeapon, ironsights, vmorigin, vmangles, vmangoriginal );

			CalcViewModelOffset( player, vmorigin );
			pWeapon->CalcViewmodelInteraction( vmorigin, vmangles );
		}

		// let the viewmodel shake at about 10% of the amplitude of the player's view
		vieweffects->ApplyShake( vmorigin, vmangles, 0.1f );	
	}
	
#endif

	SetLocalOrigin( vmorigin );
	SetLocalAngles( vmangles );
}

//=========================================================
//=========================================================
#ifdef CLIENT_DLL

//=========================================================
//=========================================================
#define WEAPONSWAY_INTERP 0.1

void CINSViewModel::CalcViewModelLag( CWeaponINSBase *weapon, bool ironsights, Vector& origin, QAngle& angles, QAngle& original_angles )
{
	Assert( weapon );

	// calculate our drift
	Vector	forward, right, up;
	AngleVectors( angles, &forward, &right, &up );
		
	// add an entry to the history
	m_vLagAngles = angles;
	m_LagAnglesHistory.NoteChanged( gpGlobals->curtime, WEAPONSWAY_INTERP );
		
	// interpolate back 100ms
	m_LagAnglesHistory.Interpolate( gpGlobals->curtime, WEAPONSWAY_INTERP );
		
	// now take the 100ms angle difference and figure out how far the forward vector moved in local space
	Vector vLaggedForward;
	QAngle angleDiff = m_vLagAngles - angles;
	AngleVectors( -angleDiff, &vLaggedForward, 0, 0 );
	Vector vForwardDiff = Vector( 1, 0 ,0 ) - vLaggedForward;

	// now offset the origin using that
	vForwardDiff *= weapon->GetWeaponLagFactor( ironsights );
	origin += forward * vForwardDiff.x + right * -vForwardDiff.y + up * vForwardDiff.z;
}

//=========================================================
//=========================================================
#define VM_TRANS_PLAYERMOVING 125

void CINSViewModel::CalcViewModelOffset( CINSPlayer *player, Vector &origin )
{
	if( ( player->GetPlayerFlags( ) & ( FL_PLAYER_IRONSIGHTS | FL_PLAYER_BIPOD ) ) != 0 )
		return;

	int iLastStance, iCurrentStance;
	iLastStance = player->GetLastStance( );
	iCurrentStance = player->GetCurrentStance( );

	Vector vecCurrentStance;
	vecCurrentStance = UTIL_ViewModelStanceOffset( iCurrentStance );

	// adjust the origin

	// ... by speed
	float flPlayerSpeed = player->GetAbsVelocity( ).Length2D( );

	if( flPlayerSpeed > 0.0f )
	{
		Vector vecSpeedOrigin = UTIL_ViewModelMovingOffset( iCurrentStance );
		vecSpeedOrigin *= clamp( flPlayerSpeed / VM_TRANS_PLAYERMOVING, 0.0f, 1.0f );

		origin -= vecSpeedOrigin;
	}

	// ... by stance
	if( player->InStanceTransition( ) && ( ( iLastStance == STANCE_STAND && iCurrentStance == STANCE_CROUCH ) || ( iLastStance == STANCE_CROUCH && iCurrentStance == STANCE_STAND ) ) )
	{
		float flStanceTransitionMarker, flTransitionTime, flTransitionFraction;

		flStanceTransitionMarker = player->GetStanceTransitionMarker( );
		flTransitionTime = UTIL_StanceTransitionTime( iLastStance, iCurrentStance );

		flTransitionFraction = 1.0f - max( 0.0f, ( gpGlobals->curtime - flStanceTransitionMarker ) / flTransitionTime );

		origin -= ( UTIL_ViewModelStanceOffset( iLastStance ) * flTransitionFraction ) + ( vecCurrentStance * ( 1.0f - flTransitionFraction ) );
	}
	else
	{
		origin -= vecCurrentStance;
	}
}

//=========================================================
//=========================================================
bool CINSViewModel::ShouldPredict( void )
{
	if ( GetOwner( ) && GetOwner( ) == C_BasePlayer::GetLocalPlayer( ) )
		return true;

	return BaseClass::ShouldPredict( );
}

#endif
