//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "voicemgr.h"
#include "keyvalues.h"
#include "hud_macros.h"
#include "ins_utils.h"
#include "c_playerresource.h"
#include "hud_messages.h"
#include "play_team_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
void __MsgFunc_VoiceCmd( bf_read &msg )
{
	g_VoiceMgr.HandleCmd( msg );
}

//=========================================================
//=========================================================
CVoiceMgr::CVoiceMgr( void )
{
}

//=========================================================
//=========================================================
bool CVoiceMgr::Init( void )
{
	HOOK_MESSAGE( VoiceCmd );

	return true;
}

//=========================================================
//=========================================================
void CVoiceMgr::HandleCmd( bf_read &msg )
{
	int iGroupID, iEntityIndex;

	// read in group and entity
	iGroupID = msg.ReadByte( );
	iEntityIndex = msg.ReadByte( );

	// ensure valid group
	if( !IsValidGroup( iGroupID ) )
	{
		Assert( false );
		return;
	}

	// start parsing message
	KeyValues *pData = NULL;

	if( msg.ReadByte( ) )
	{
		pData = new KeyValues( "voicedata" );

		UTIL_ReadKeyValues( msg, pData );
	}

	// start playing
	Radio( iGroupID, iEntityIndex, pData );

	// remove data
	if( pData )
		pData->deleteThis( );
}

//=========================================================
//=========================================================
void CVoiceMgr::Radio( int iGroupID, int iEntityIndex, KeyValues *pData )
{
	// this is only for playing radio commands - expect for when it's the local player
	// because then it's normal sound (they're not hearing themselves over radio)

	CVoiceGroup *pGroup = GetGroup( iGroupID );

	if( !pGroup )
	{
		Assert( false );
		return;
	}

	C_INSPlayer *pPlayer = C_INSPlayer::GetLocalPlayer( );

	if( !pPlayer || !pPlayer->OnPlayTeam( ) )
	{
		Assert( false );
		return;
	}

	int iTeamID = g_PR->GetTeam( iEntityIndex );

	if( !IsPlayTeam( iTeamID ) )
	{
		Assert( false );
		return;
	}

	int iSoundType = ( ( iEntityIndex == pPlayer->entindex( ) ) ? VSOUNDTYPE_NORMAL : VSOUNDTYPE_RADIO );

	// print to HUD
	Print( pGroup, iEntityIndex, iTeamID, pData );

	// start to play it
	Play( pGroup, iTeamID, iSoundType, pData );

	// do the action
	pGroup->Action( pData );
}

//=========================================================
//=========================================================
void CVoiceMgr::Print( CVoiceGroup *pGroup, int iEntityIndex, int iTeamID, KeyValues *pData )
{
	char szGeneratedText[ MAX_VOICE_TEXT ];

	if( !pGroup->Text( szGeneratedText, iTeamID, pData ) )
		return;

	g_pPlayerChat->PrintRadio( szGeneratedText, iEntityIndex );
}

//=========================================================
//=========================================================
void CVoiceMgr::Play( CVoiceGroup *pGroup, int iTeamID, int iSoundType, KeyValues *pData )
{
	if( !pGroup )
		return;

	VoiceSoundString_t SoundString;
	pGroup->Play(SoundString, iTeamID, iSoundType, pData);

	C_INSPlayer *pPlayer = C_INSPlayer::GetLocalPlayer( );
	if( pPlayer )
		pPlayer->EmitSound( SoundString );
}

//=========================================================
//=========================================================
CON_COMMAND( ins_listvoicescipts, "Print the Loaded Sounds for Voice" )
{
	g_VoiceMgr.PrintSounds( );
}