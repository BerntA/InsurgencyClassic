//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#include "cbase.h"
#include "pain_helper.h"
#include "c_ins_player.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
#ifdef _DEBUG

ConVar forcemotionblur( "cl_forcemotionblur", "0" );

#endif

//=========================================================
//=========================================================
void CPainElements::Reset( void )
{
	m_flAlphaLevel = m_flBlurLevel = m_flHazeLevel = 0.0f;
	m_bUseMotionBlur = false;
}

//=========================================================
//=========================================================
typedef const char *Parameter_t;

bool ParameterLess( const Parameter_t &Left, const Parameter_t &Right )
{
	return Q_strcmp( Left, Right );
}

CPainType::CPainType( )
{
	m_Parameters.SetLessFunc( ParameterLess );
}

//=========================================================
//=========================================================
void CPainType::Reset( void )
{
	CPainElements::Reset( );

	m_bFresh = true;

	m_Parameters.RemoveAll( );
}

//=========================================================
//=========================================================
void CPainType::SetParameter( const char *pszName, int iValue )
{
	m_Parameters.Insert( pszName, iValue );
}

//=========================================================
//=========================================================
bool CPainType::GetParameter( const char *pszName, int &iValue )
{
	int iElementID = m_Parameters.Find( pszName );

	if( !m_Parameters.IsValidIndex( iElementID ) )
		return false;

	iValue = m_Parameters[ iElementID ];

	return true;
}

//=========================================================
//=========================================================
void CPainType::Init( void )
{
	m_bFresh = false;

	DoInit( );
}

//=========================================================
//=========================================================
void CPainType::Update( void )
{
	Reset( );
}

//=========================================================
//=========================================================
PainTypeHelper_t g_PainHelpers[ MAX_PAINTYPES ];
CPainHelper g_PainHelper;

//=========================================================
//=========================================================
bool CPainHelper::Init( void )
{
	for( int i = 0; i < MAX_PAINTYPES; i++ )
	{
		Assert( g_PainHelpers[ i ] );
		m_pPainTypes[ i ] = g_PainHelpers[ i ]( );
	}

	return true;
}

//=========================================================
//=========================================================
void CPainHelper::Shutdown( void )
{
	for( int i = 0; i < MAX_PAINTYPES; i++ )
		delete m_pPainTypes[ i ];
}

//=========================================================
//=========================================================
CPainType *CPainHelper::CreatePain( int iTypeID )
{
	// check that we're reseting
	if( iTypeID == PAINTYPE_RESET )
	{
		m_Active.RemoveAll( );
		return NULL;
	}

	// ensure valid pain
	if( iTypeID < 0 || iTypeID >= MAX_PAINTYPES )
		return NULL;

	// find it
	CPainType *pPain = m_pPainTypes[ iTypeID ];

	if( !pPain )
	{
		AssertMsg( false, "CPainHelper::CreatePain, Missing PainType" );
		return NULL;
	}

	// reset
	pPain->Reset( );

	// add it to the active list
	if( m_Active.Find( iTypeID ) == m_Active.InvalidIndex( ) )
		m_Active.AddToTail( iTypeID );

	return m_pPainTypes[ iTypeID ];
}

//=========================================================
//=========================================================
#ifdef TESTING

ConVar r_inseffects( "r_inseffects", "1", FCVAR_CLIENTDLL, "Show INS Effects" );

#endif

bool CPainHelper::AttemptUpdate(void)
{
#ifdef TESTING

	if( !r_inseffects.GetBool( ) )
		return false;

#endif

	bool bCanUpdate = ( m_Active.Count( ) != 0 && CanDrawPain( ) );

#ifdef _DEBUG

	if( !bCanUpdate )
		bCanUpdate = forcemotionblur.GetBool( );

#endif

	if( bCanUpdate )
	{
		Update( );

		return true;
	}

	return false;
}

//=========================================================
//=========================================================
bool CPainHelper::CanDrawPain( void )
{
	C_INSPlayer *pPlayer = C_INSPlayer::GetLocalPlayer( );
	return ( pPlayer && !pPlayer->IsRealObserver( ) && ( ( pPlayer->GetEffects( ) & EF_NODRAW ) == 0 ) );
}

//=========================================================
//=========================================================
#ifdef _DEBUG

//#define ENABLE_PH_PRINTVALUES

void EnsureValidPainValue( float flValue )
{
	Assert( flValue >= 0.0f && flValue <= 1.0f );
}

#endif

void CPainHelper::Update( void )
{
	CUtlVector< int > m_DeadList;

	// clear out our elements
	Reset( );

	// calucate each pain type
	for( int i = 0; i < m_Active.Count( ); i++ )
	{
		CPainType *pPainType = m_pPainTypes[ m_Active[ i ] ];

		// ensure it's valid
		if( !pPainType )
		{
			Assert( false );
			m_DeadList.AddToTail( i );

			continue;
		}

		// ensure it's fresh
		if( pPainType->IsFresh( ) )
			pPainType->Init( );

		// check wether or not the levels want to die
		if( !pPainType->ShouldUpdate( )  )
		{
			m_DeadList.AddToTail( i );
			continue;
		}

		// update and find the biggest value
		pPainType->Update( );

		SetAlphaLevel( max( GetAlphaLevel( ), pPainType->GetAlphaLevel( ) ) );
		SetBlurLevel( max( GetBlurLevel( ), pPainType->GetBlurLevel( ) ) );
		SetHazeLevel( max( GetHazeLevel( ), pPainType->GetHazeLevel( ) ) );

		// if we're not using motion blur, set their value
		if( !UsingMotionBlur( ) )
			SetMotionBlur( pPainType->UsingMotionBlur( ) );
	}

#ifdef _DEBUG

	EnsureValidPainValue( GetAlphaLevel( ) );
	EnsureValidPainValue( GetBlurLevel( ) );
	EnsureValidPainValue( GetHazeLevel( ) );

	if( forcemotionblur.GetBool( ) )
    {
        SetMotionBlur( true );
        SetAlphaLevel( 0.5f );
        SetHazeLevel( 0.3f );
    }

#endif

	// clean up the dead
	for( int i = 0; i < m_DeadList.Count( ); i++ )
		m_Active.Remove( m_DeadList[ i ] );

#ifdef ENABLE_PH_PRINTVALUES

	Msg( "1. Alpha: %f\n", GetAlphaLevel( ) );
	Msg( "2. Blur: %f\n", GetBlurLevel( ) );
	Msg( "3. Haze: %f\n", GetHazeLevel( ) );
	Msg( "4. MotionBlur: %s\n", UsingMotionBlur( ) ? "Yes" : "No" );

#endif
}

//=========================================================
//=========================================================
void CPainHelper::FudgeValues( void )
{
	SetAlphaLevel( 1.0f );
	SetBlurLevel( 1.0f );
	SetHazeLevel( 1.0f );
	SetMotionBlur( true );
}