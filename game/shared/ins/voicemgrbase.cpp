//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "voicemgr.h"
#include "imc_config.h"
#include "team_lookup.h"
#include "play_team_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
VoiceGroupCreator_t g_VoiceGroupCreators[ VOICEGRP_COUNT ];

CVoiceMgr g_VoiceMgr;

//=========================================================
//=========================================================
SoundGroup_t::SoundGroup_t( )
{
	memset( &m_iLookupIDs, INVALID_SOUNDGROUP_LOOKUP, sizeof( m_iLookupIDs ) );
}

//=========================================================
//=========================================================
void SoundGroup_t::SetLookupID( int iTeamID, int iSoundType, int iValue )
{
	m_iLookupIDs[ TeamToPlayTeam( iTeamID ) ][ iSoundType ] = iValue;
}

//=========================================================
//=========================================================
int SoundGroup_t::GetLookupID( int iTeamID, int iSoundType ) const
{
	return m_iLookupIDs[ TeamToPlayTeam( iTeamID ) ][ iSoundType ];
}

//=========================================================
//=========================================================
SoundString_t::SoundString_t( )
{
	m_pszString = NULL;
}

SoundString_t::~SoundString_t( )
{
	if( m_pszString )
		delete [ ]m_pszString;
}

//=========================================================
//=========================================================
CVoiceGroup::CVoiceGroup( )
{
	m_iFlags = 0;
}

//=========================================================
//=========================================================
int CVoiceGroup::InitSound( const char *pszSound )
{
	// take the sound and create a group that contains the sounds
	// from both teams and the three soundtypes

	if( !pszSound )
	{
		Assert( false );
		return INVALID_SOUNDGROUP_LOOKUP;
	}

	// ... create data
	int iSoundGroupID = m_SoundGroups.AddToTail( );
	SoundGroup_t &SoundGroup = m_SoundGroups[ iSoundGroupID ];

	// ... form the different types
	for( int i = TEAM_ONE; i <= TEAM_TWO; i++ )
	{
		// PNOTE: always include normal because "local" radio
		// commands are spoken - not heard through the radio
		AddSoundGroup( SoundGroup, i, pszSound, VSOUNDTYPE_NORMAL );

		if( SendType( ) != VSENDTYPE_RADIO )
		{
			if( TeamLookup( i )->UsesForeignLanguage( ) )
				AddSoundGroup( SoundGroup, i, pszSound, VSOUNDTYPE_FOREIGN );
		}
		else if( SendType( ) != VSENDTYPE_SHOUT )
		{
			AddSoundGroup( SoundGroup, i, pszSound, VSOUNDTYPE_RADIO );
		}
	}

	return iSoundGroupID;
}

//=========================================================
//=========================================================
void CVoiceGroup::AddSoundGroup( SoundGroup_t &SoundGroup, int iTeamID, const char *pszSound, int iSoundType )
{
	char *pszBuffer = new VoiceSoundString_t;
	CreateSoundScript( pszBuffer, iTeamID, pszSound, iSoundType );

	int iSoundID = m_Sounds.AddToTail( );
	SoundString_t &SoundString = m_Sounds[ iSoundID ];

	SoundString.m_pszString = pszBuffer;

	SoundGroup.SetLookupID( iTeamID, iSoundType, iSoundID );
}

//=========================================================
//=========================================================
const char *g_pszVoiceSoundTypes[ VSOUNDTYPE_COUNT ] = {
	"n",		// VSTYPE_NORMAL
	"f",		// VSTYPE_FOREIGN
	"r",		// VSTYPE_RADIO
};

void CVoiceGroup::CreateSoundScript( char *pszBuffer, int iTeamID, const char *pszSound, int iSoundType ) const
{
	Q_snprintf( pszBuffer, MAX_VOICE_SOUNDSCRIPT, "voice.%s.%s.%s", TeamLookup( iTeamID )->GetFileName( ), pszSound, g_pszVoiceSoundTypes[ iSoundType ] );
}

//=========================================================
//=========================================================
void CVoiceGroup::Reset( void )
{
	m_SoundGroups.Purge( );
	m_Sounds.Purge( );
}

//=========================================================
//=========================================================
#ifdef GAME_DLL

void CVoiceGroup::Precache( void )
{
	for( int j = 0; j < m_Sounds.Count( ); j++ )
	{
		const char *pszSound = m_Sounds[ j ].m_pszString;

		if( !pszSound )
		{
			Assert( false );
			continue;
		}

		CBaseEntity::PrecacheScriptSound( pszSound );
	}
}

#endif

//=========================================================
//=========================================================
bool CVoiceGroup::Play( char *pszBuffer, int iTeamID, int iSoundType, KeyValues *pData ) const
{
	// if the team isn't foreign, switch to normal
	if( iSoundType == VSOUNDTYPE_FOREIGN && !TeamLookup( iTeamID )->UsesForeignLanguage( ) )
		iSoundType = VSOUNDTYPE_NORMAL;

	// load base string
	int iSoundGroupID = Play( pData );

	if( iSoundGroupID == INVALID_SOUNDGROUP_LOOKUP )
		return false;

	if( !m_SoundGroups.IsValidIndex( iSoundGroupID ) )
	{
		AssertMsg( false, "CVoiceGroup::Play, Invalid SoundGroup" );
		return false;
	}

	int iStringLookupID = m_SoundGroups[ iSoundGroupID ].GetLookupID( iTeamID, iSoundType );

	if( !m_Sounds.IsValidIndex( iStringLookupID ) )
	{
		AssertMsg( false, "CVoiceGroup::Play, Invalid SoundLookup" );
		return false;
	}

	// copy out final string
	Q_strncpy( pszBuffer, m_Sounds[ iStringLookupID ].m_pszString, MAX_VOICE_SOUNDSCRIPT );

	return true;
}

//=========================================================
//=========================================================
CTeamLookup *CVoiceGroup::TeamLookup( int iTeamID ) const
{
	CIMCTeamConfig *pTeam = IMCConfig( )->GetIMCTeam( iTeamID );
	Assert( pTeam );

	return pTeam->GetTeamLookup( );
}

//=========================================================
//=========================================================
void CVoiceMgrBase::Precache( void )
{
	for( int i = 0; i < VOICEGRP_COUNT; i++ )
	{
		VoiceGroupCreator_t GroupCreator = g_VoiceGroupCreators[ i ];
		Assert( GroupCreator );

		CVoiceGroup *pGroup = GroupCreator( );
		Assert( pGroup );

		pGroup->InitSounds( );

		m_pGroups[ i ] = pGroup;
	}

#ifdef GAME_DLL

	for( int i = 0; i < VOICEGRP_COUNT; i++ )
	{
		CVoiceGroup *pGroup = m_pGroups[ i ];
		Assert( pGroup );

		pGroup->Precache( );
	}

#endif
}

//=========================================================
//=========================================================
bool CVoiceMgrBase::IsValidGroup( int iVoiceCmd ) const
{
	return ( iVoiceCmd >= 0 && iVoiceCmd < VOICEGRP_COUNT );
}

//=========================================================
//=========================================================
CVoiceGroup *CVoiceMgrBase::GetGroup( int iGroupID )
{
	return m_pGroups[ iGroupID ];
}

//=========================================================
//=========================================================
const CVoiceGroup *CVoiceMgrBase::GetGroup( int iGroupID ) const
{
	return m_pGroups[ iGroupID ];
}

//=========================================================
//=========================================================
void CVoiceMgrBase::LevelShutdownPostEntity( void )
{
	for( int i = 0; i < VOICEGRP_COUNT; i++ )
	{
		CVoiceGroup *pGroup = m_pGroups[ i ];

		if( pGroup )
			pGroup->Reset( );
	}
}

//=========================================================
//=========================================================
void CVoiceGroup::PrintSounds( void )
{
	for( int i = 0; i < m_Sounds.Count( ); i++ )
	{
		const char *pszString = m_Sounds[ i ].m_pszString;
		Assert( pszString );

		if( pszString )
			Msg( "%s\n", pszString );
	}
}

//=========================================================
//=========================================================
void CVoiceMgrBase::PrintSounds( void )
{
	for( int i = 0; i < VOICEGRP_COUNT; i++ )
	{
		CVoiceGroup *pGroup = GetGroup( i );
		Assert( pGroup );

		if( pGroup )
			pGroup->PrintSounds( );

	}
}