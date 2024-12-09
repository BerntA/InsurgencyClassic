//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "hint_helper.h"

#ifdef GAME_DLL

#include "ins_player_shared.h"
#include "ins_recipientfilter.h"

#else

#include "hud_macros.h"

#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
HintCreator_t g_HintCreators[ HINT_COUNT ];

//=========================================================
//=========================================================
CHintHelperBase::CHintHelperBase( )
{
	memset( &m_pHintData, NULL, sizeof( m_pHintData ) );
}

//=========================================================
//=========================================================
void CHintHelperBase::Init( void )
{
	for( int i = 0; i < HINT_COUNT; i++ )
	{
		HintCreator_t HintCreator = g_HintCreators[ i ];
		Assert( HintCreator );

		m_pHintData[ i ] = HintCreator( );
	}
}

//=========================================================
//=========================================================
bool CHintHelperBase::IsValidHint( int iHintID )
{
	return ( iHintID >= 0 && iHintID < HINT_COUNT );
}

//=========================================================
//=========================================================
IHint *CHintHelperBase::GetHint( int iHintID ) const
{
	Assert( IsValidHint( iHintID ) );
	return m_pHintData[ iHintID ];
}

//=========================================================
//=========================================================
CHintHelper g_HintHelper;

//=========================================================
//=========================================================
#ifdef GAME_DLL

void CHintHelper::LevelInitPreEntity( void )
{
	memset( &m_bSentHints, false, sizeof( m_bSentHints ) );
}

//=========================================================
//=========================================================
void CHintHelper::SendHint( CINSPlayer *pPlayer, int iHintID )
{
	if( !pPlayer )
		return;

	// ensure its a valid hint
	if( !IsValidHint( iHintID ) )
	{
		AssertMsg( false, "Invalid Hint Requested for Sending" );
		return;
	}

	// ensure the hint is designed for sending
	IHint *pHint = m_pHintData[ iHintID ];

	if( pHint->ClientOnly( ) )
	{
		AssertMsg( false, "Hint not Designed for Sending" );
		return;
	}

	// ensure it hasn't been sent before
	bool &bSentHint = m_bSentHints[ iHintID ][ pPlayer->entindex( ) ];

	if( bSentHint )
		return;

	if( pPlayer->SendHints( ) )
	{
		// send the message
	    CReliablePlayerRecipientFilter Filter( pPlayer );

	    UserMessageBegin( Filter, "ShowHint" );
	
			WRITE_BYTE( iHintID );

		MessageEnd( );
	}

	// mark it as sent
	bSentHint = true;
}

//=========================================================
//=========================================================
bool CHintHelper::Init( void )
{
	CHintHelperBase::Init( );

	return true;
}

#endif

//=========================================================
//=========================================================
#ifdef CLIENT_DLL

//=========================================================
//=========================================================
CUtlVector< IHintListener* > g_HintListeners;

//=========================================================
//=========================================================
IHintListener::IHintListener( )
{
	g_HintListeners.AddToTail( this );
}

//=========================================================
//=========================================================
IHintListener::~IHintListener( )
{
	g_HintListeners.FindAndRemove( this );
}

//=========================================================
//=========================================================
void __MsgFunc_ShowHint( bf_read &msg )
{
	g_HintHelper.ShowHint( msg.ReadByte( ) );
}

//=========================================================
//=========================================================
CHintHelper::CHintHelper( void )
{
}

//=========================================================
//=========================================================
bool CHintHelper::Init( void )
{
	HOOK_MESSAGE( ShowHint );

	return true;
}

//=========================================================
//=========================================================
void CHintHelper::ShowHint( int iHintID )
{
	if( !IsValidHint( iHintID ) )
	{
		AssertMsg( false, "Invalid Hint" );
		return;
	}

	for( int i = 0; i < g_HintListeners.Count( ); i++ )
		g_HintListeners[ i ]->OnHint( iHintID );
}

#endif