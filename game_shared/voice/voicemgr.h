//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
//
//=============================================================================//

#ifndef VOICE_COMMANDS_H
#define VOICE_COMMANDS_H
#ifdef _WIN32
#pragma once
#endif

class CINSPlayer;
class CTeamLookup;

//=========================================================
//=========================================================
#define VOICEGRP_INVALID -1

enum VoiceSendTypes_t
{
	VSENDTYPE_SHOUT = 0,		// shout it just outloud
	VSENDTYPE_RADIO,			// send over radio
	VSENDTYPE_SCREAM			// send across radio and shout
};

enum VoiceSoundTypes_t
{
	VSOUNDTYPE_NORMAL = 0,		// in english
	VSOUNDTYPE_FOREIGN,			// in the teams actual language
	VSOUNDTYPE_RADIO,			// in english, but with radio distortion
	VSOUNDTYPE_COUNT
};

#define MAX_VOICE_SOUNDSCRIPT 64
#define MAX_VOICE_TEXT 92

typedef char VoiceSoundString_t[ MAX_VOICE_SOUNDSCRIPT ];

class SoundGroup_t
{
public:
	SoundGroup_t( );

	void SetLookupID( int iTeamID, int iSoundType, int iValue );
	int GetLookupID( int iTeamID, int iSoundType ) const;

private:
	int m_iLookupIDs[ MAX_PLAY_TEAMS ][ VSOUNDTYPE_COUNT ];
};

struct SoundString_t
{
	SoundString_t( );
	~SoundString_t( );

	char *m_pszString;
};

#define INVALID_SOUNDGROUP_LOOKUP -1

//=========================================================
//=========================================================
enum VoiceGroups_t
{
	VOICEGRP_REINFORCE = 0,

	VOICEGRP_ORDER_OBJ,
	VOICEGRP_ORDER_UNIT,

	VOICEGRP_ORDER_PLAYER,
	VOICEGRP_ORDER_PLAYER_RESPONSES,

	VOICEGRP_PLAYER_ACTION,

	VOICEGRP_PLAYER_STATUS,

	VOICEGRP_PLAYER_HELP,

	VOICEGRP_MOVEOUT,

	VOICEGRP_COUNT
};

//=========================================================
//=========================================================
class CVoiceGroup
{
public:
	CVoiceGroup( );

	virtual void InitSounds( void ) = 0;

	void Reset( void );

#ifdef GAME_DLL

	void Precache( void );

#endif

	virtual int SendType( void ) const = 0;

	bool Play( char *pszBuffer, int iTeamID, int iSoundType, KeyValues *pData ) const;

#ifdef CLIENT_DLL

	virtual bool Text( char *pszBuffer, int iTeamID, KeyValues *pData ) { return false; }

	virtual void Action( KeyValues *pData ) { }

#endif

	void PrintSounds( void );

protected:
	int InitSound( const char *pszSound );

	virtual int Play( KeyValues *pData ) const = 0;

private:
	void AddSoundGroup( SoundGroup_t &SoundGroup, int iTeamID, const char *pszSound, int iSoundType );
	void CreateSoundScript( char *pszBuffer, int iTeamID, const char *pszSound, int iSoundType ) const;

	CTeamLookup *TeamLookup( int iTeamID ) const;

private:
	int m_iFlags;

	CUtlVector< SoundString_t > m_Sounds;

	CUtlVector< SoundGroup_t > m_SoundGroups;
};

//=========================================================
//=========================================================
class CVoiceMgrBase : public CAutoGameSystem
{
public:
	void Precache( void );

	void PrintSounds( void );

protected:
	bool IsValidGroup( int iGroupID ) const;

	CVoiceGroup *GetGroup( int iGroupID );
	const CVoiceGroup *GetGroup( int iGroupID ) const;

private:
	void LevelShutdownPostEntity( void );

protected:
	CVoiceGroup *m_pGroups[ VOICEGRP_COUNT ];
};

//=========================================================
//=========================================================
#ifdef GAME_DLL

class CVoiceMgr : public CVoiceMgrBase
{
public:
	bool Send( int iGroupID, CINSPlayer *pPlayer, KeyValues *pData ) const;

private:
	bool DoSend( int iGroupID, CINSPlayer *pPlayer, KeyValues *pData ) const;

	void Radio( int iGroupID, CINSPlayer *pPlayer, KeyValues *pData ) const;

	void Shout( const CVoiceGroup *pGroup, CINSPlayer *pPlayer, KeyValues *pData ) const;
	void Shout( const CVoiceGroup *pGroup, CINSPlayer *pPlayer, KeyValues *pData, bool bForeign ) const;
};

#endif

//=========================================================
//=========================================================
#ifdef CLIENT_DLL

class CVoiceMgr : public CVoiceMgrBase
{
public:
	CVoiceMgr( );

	void HandleCmd( bf_read &msg );

	bool Init( void );

private:
	void Radio( int iGroupID, int iEntityIndex, KeyValues *pData );

	void Print( CVoiceGroup *pGroup, int iEntityIndex, int iTeamID, KeyValues *pData );
	void Play( CVoiceGroup *pGroup, int iTeamID, int iSoundType, KeyValues *pData );
};

#endif

//=========================================================
//=========================================================
extern CVoiceMgr g_VoiceMgr;

//=========================================================
//=========================================================
typedef CVoiceGroup *( *VoiceGroupCreator_t )( void );
extern VoiceGroupCreator_t g_VoiceGroupCreators[ VOICEGRP_COUNT ];

#define DECLARE_VOICEGROUP( id, classname ) \
	CVoiceGroup *CreateVoiceGroup__##id( void ) { \
		return new classname; } \
	class CVoiceGroup__##id { \
	public: \
	CVoiceGroup__##id( ) { \
	g_VoiceGroupCreators[ id ] = CreateVoiceGroup__##id; } \
	} g_VoiceGroup__##id;

//=========================================================
//=========================================================
extern void UTIL_SendVoice( int iGroupID, CINSPlayer *pPlayer, KeyValues *pData );
	
#endif // VOICE_COMMANDS_H