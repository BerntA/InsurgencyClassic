//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "voicemgr.h"
#include "ins_player_shared.h"
#include "play_team_shared.h"
#include "ins_recipientfilter.h"
#include "ins_utils.h"
#include "soundemittersystem/isoundemittersystembase.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
class CShoutRecipientFilter : public CPASAttenuationFilter
{
public:
	CShoutRecipientFilter( CINSPlayer *pPlayer, bool bForeign, int iTeamID, soundlevel_t soundlevel );
};

//=========================================================
//=========================================================
CShoutRecipientFilter::CShoutRecipientFilter( CINSPlayer *pPlayer, bool bForeign, int iTeamID, soundlevel_t soundlevel )
	: CPASAttenuationFilter( pPlayer, soundlevel )
{
	RemoveRecipientsByTeam( GetGlobalTeam( ( bForeign ? iTeamID : FlipPlayTeam( iTeamID ) ) ) );
}

//=========================================================
//=========================================================
bool CVoiceMgr::Send( int iGroupID, CINSPlayer *pPlayer, KeyValues *pData ) const
{
	bool bResult = DoSend( iGroupID, pPlayer, pData );

	// remove data
	if( pData )
		pData->deleteThis( );

	return bResult;
}

//=========================================================
//=========================================================
bool CVoiceMgr::DoSend( int iGroupID, CINSPlayer *pPlayer, KeyValues *pData ) const
{
	// check for a invalid command
	if( !IsValidGroup( iGroupID ) )
		return false;

	// check for an invalid player
	if( !pPlayer )
		return false;

	// ensure valid player
	if( !pPlayer->IsRunningAround( ) )
		return false;

	// find group
	const CVoiceGroup *pGroup = GetGroup( iGroupID );

	if( !pGroup )
	{
		Assert( false );
		return false;
	}

	// send it

	// ... shout it
	int iSendType = pGroup->SendType( );

	if( iSendType != VSENDTYPE_RADIO )
		Shout( pGroup, pPlayer, pData );

	// ... across the radio
	if( iSendType != VSENDTYPE_SHOUT )
		Radio( iGroupID, pPlayer, pData );

	return true;
}

//=========================================================
//=========================================================
void CVoiceMgr::Radio( int iGroupID, CINSPlayer *pPlayer, KeyValues *pData ) const
{
	// setup which filter to use
	CUnitRecipientFilter Filter;
	Filter.Setup( pPlayer );

	UserMessageBegin( Filter, "VoiceCmd" );

		// write basic data
		WRITE_BYTE( iGroupID );
		WRITE_BYTE( pPlayer->entindex( ) );

		// write additional data
		UTIL_SendKeyValues( pData );

	MessageEnd( );
}

//=========================================================
//=========================================================
void CVoiceMgr::Shout( const CVoiceGroup *pGroup, CINSPlayer *pPlayer, KeyValues *pData ) const
{
	// emit the sound

	// ... in local
	Shout( pGroup, pPlayer, pData, true );

	// ... in foreign
	Shout( pGroup, pPlayer, pData, false );
}

//=========================================================
//=========================================================
void CVoiceMgr::Shout( const CVoiceGroup *pGroup, CINSPlayer *pPlayer, KeyValues *pData, bool bForeign ) const
{
	int iTeamID = pPlayer->GetTeamID( );

	// find the sound to play
	VoiceSoundString_t SoundString;
	pGroup->Play( SoundString, iTeamID, bForeign ? VSOUNDTYPE_FOREIGN : VSOUNDTYPE_NORMAL, pData );

	// find sound data
	CSoundParameters SoundParams;

	if( !CBaseEntity::GetParametersForSound( SoundString, SoundParams, NULL ) )
		return;

	// set soundlevel (depending on if it's broadcast)
	SoundParams.soundlevel = ( pGroup->SendType( ) == VSENDTYPE_SHOUT ) ? SNDLVL_95dB : SNDLVL_60dB;

	// broadcast it
	CShoutRecipientFilter filter( pPlayer, bForeign, iTeamID, SoundParams.soundlevel );
	CBaseEntity::EmitSound( filter, pPlayer->entindex( ), SoundParams );
}

//=========================================================
//=========================================================
void UTIL_SendVoice( int iGroupID, CINSPlayer *pPlayer, KeyValues *pData )
{
	g_VoiceMgr.Send( iGroupID, pPlayer, pData );
}