//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#ifndef INS_HUD_H
#define INS_HUD_H
#ifdef _WIN32
#pragma once
#endif

#include "ins_player_defines.h"
#include "imc_format.h"
#include "commander_shared.h"
#include "ins_utils.h"

//=========================================================
//=========================================================
class C_INSSquad;
class C_PlayTeam;

//=========================================================
//=========================================================
class IINSHUDElement
{
public:
	virtual void Init( void ) { }

	virtual bool ShouldDraw( void );

private:
	int m_iHiddenFlags;
};

class CINSHUDElementHelper
{
	typedef IINSHUDElement *( *CreateElement_t )( void );

public:
	CINSHUDElementHelper( CreateElement_t Helper );

	static void CreateAllElements( void );

	CINSHUDElementHelper *GetNext( void ) { return m_pNext; }

public:
	static CINSHUDElementHelper *m_pHelpers;

private:
	CINSHUDElementHelper *m_pNext;
	CreateElement_t m_Helper;
};

#define DECLARE_INSHUDELEMENT( className ) \
	static IINSHUDElement *Create_##className(void) { \
			return new className; \
		}; \
	static CINSHUDElementHelper g_##className##_Helper( Create_##className );

//=========================================================
//=========================================================
class IINSMeshDraw
{
public:
	IINSMeshDraw( );
	virtual ~IINSMeshDraw( );

	virtual void Draw( void ) = 0;
};

//=========================================================
//=========================================================
class IINSOrderListener
{
public:
	IINSOrderListener( );
	virtual ~IINSOrderListener( );

	virtual void ObjOrder( void ) { }
	virtual void UnitOrder( void ) { }
};

//=========================================================
//=========================================================
class IINSTeamListener
{
public:
	IINSTeamListener( );
	virtual ~IINSTeamListener( );

	virtual void TeamUpdate( C_PlayTeam *pTeam ) = 0;
};

//=========================================================
//=========================================================
class IINSDamageListener
{
public:
	IINSDamageListener( );
	virtual ~IINSDamageListener( );

	virtual void DamageTaken( int iAmount, int iBits, Vector &vecFrom ) = 0;
};

//=========================================================
//=========================================================

// TODO: need a "canclose" for CHudMenu

class IINSControlListener
{
public:
	void ControlTakeFocus( void );
	virtual bool IsControlActive( void ) = 0;
	void ControlClose( void );

	virtual void Selection( void ) { }
	virtual void Scroll( int iType ) { }
	virtual void Number( int iNumber ) { }

protected:
	virtual void DoControlClose( void ) = 0;
};

//=========================================================
//=========================================================
class IINSObjListener
{
public:
	IINSObjListener( );
	virtual ~IINSObjListener( );

	virtual void ObjUpdate( C_INSObjective *pObjective ) = 0;
};

//=========================================================
//=========================================================
class IINSPostRenderListener
{
public:
	IINSPostRenderListener( );
	virtual ~IINSPostRenderListener( );

	virtual void PostRender( void ) = 0;
};

//=========================================================
//=========================================================
class IINSChatListener
{
public:
	IINSChatListener( );
	virtual ~IINSChatListener( );

	virtual void PrintChat( CColoredString &ChatMessage, int iType ) = 0;
};

//=========================================================
//=========================================================
class IINSFireMode
{
public:
	IINSFireMode( );
	virtual ~IINSFireMode( );

	virtual void FireMode( int iMode ) = 0;
};

//=========================================================
//=========================================================
class IINSPlayerDeath
{
public:
	IINSPlayerDeath( );
	virtual ~IINSPlayerDeath( );

	virtual void PlayerDeath( void ) = 0;
};

//=========================================================
//=========================================================
class IINSReinforcement
{
public:
	IINSReinforcement( );
	virtual ~IINSReinforcement( );

	virtual void ReinforcementDeployed( int iRemaining ) { }

	virtual void EmergencyDeployment( void ) { }
};

//=========================================================
//=========================================================
#define HIDEHUD_ALIVE			( 1<<1 )	// hide when the local player is alive
#define HIDEHUD_DEAD			( 1<<2 )	// hide when the local player is dead
#define HIDEHUD_NOTCOMMANDER	( 1<<3 )	// hide when the local player is not a commander
#define HIDEHUD_ROUNDCOLD		( 1<<4 )	// hide when the round hasn't fully started

//=========================================================
//=========================================================
#define INSHUD_SCROLLUP 0
#define INSHUD_SCROLLDOWN 1

//=========================================================
//=========================================================
#define INSHUD_TIMER_LENGTH 8

//=========================================================
//=========================================================
enum INSRoundTimer_t
{
	ROTIMERTYPE_INVALID = 0,
	ROTIMERTYPE_NORMAL,
	ROTIMERTYPE_NONE,
	ROTIMERTYPE_RED,
	ROTIMERTYPE_FLASH
};

//=========================================================
//=========================================================
enum INSReinforcementTimer_t
{
	RFTIMERTYPE_INVALID = 0,
	RFTIMERTYPE_PREPARE,
	RFTIMERTYPE_COUNTDOWN,
	RFTIMERTYPE_NOTSTARTED,
	RFTIMERTYPE_NONE,
	RFTIMERTYPE_EMERGENCY,
	RFTIMERTYPE_TKPUNISH
};

//=========================================================
//=========================================================
class IINSUnitOrder
{
public:
	virtual void StartOrder( void ) = 0;
};

//=========================================================
//=========================================================
class IINSWeaponInfo
{
public:
	virtual int CreateAmmoTexID( const char *pszName ) = 0;

	virtual void ResetInfo( void ) = 0;

	virtual void ShowAmmoInfo( float flTime, bool bAddDefault ) = 0;
	virtual void ShowAmmoInfo( void ) = 0;

	virtual void ShowROFInfo( float flTime, bool bAddDefault ) = 0;
	virtual void ShowROFInfo( void ) = 0;
};

//=========================================================
//=========================================================
class IINSChatMessages
{
public:
	virtual void PrintChat( CColoredString &String, int iType ) = 0;
	virtual void PrintChat( const char *pszMessage ) = 0;

	virtual void PrintRadio( const char *pszMessage, int iEntID ) = 0;
};

//=========================================================
//=========================================================
class CINSHUDHelper : public IINSDamageListener, public IINSOrderListener
{
public:
	static CINSHUDHelper *GetINSHUDHelper( void );

	void Init( void );

	void LevelShutdown( void );

	void ProcessInput( void );

	// Register Interfaces
	void RegisterUnitOrder( IINSUnitOrder *pElement );
	void RegisterWeaponInfo( IINSWeaponInfo *pElement );
	void RegisterChatMessages( IINSChatMessages *pElement );

	// Mesh Drawing
	void DrawHUDMesh( void );

	// Listener Update
	void SendTeamUpdate( C_PlayTeam *pTeam );
	void SendObjOrderUpdate( void );
	void SendUnitOrderUpdate( void );
	void SendDamageUpdate( bf_read &msg );
	void SendPainUpdate( bf_read &msg );
	void SendObjUpdate( C_INSObjective *pObjective );
	void SendPostRender( void );
	void SendChatUpdate( bf_read &msg );
	void SendFireModeUpdate( bf_read &msg );
	void SendDeathUpdate( void );
	void SendReinforcementUpdate( bf_read &msg );
	void SendEmergencyUpdate( void );

	// Controller
	inline void RegisterDefaultController( IINSControlListener *pController );
	inline bool IsActiveController( IINSControlListener *pController );
	void TakeControlFocus( IINSControlListener *pController );
	void RemoveControlFocus( void );

	void SendScrollUpdate( int iType );
	void SendNumberUpdate( int iNumber );

	// Server Messages
	void ReinforcementCall( void );
	void ObjectiveOrder( int iObjID );
	void UnitOrder( int iTypeID, const Vector &vecPosition );
	void PlayerOrder( int iOrderID );
	void PlayerOrderResponse( int iResponseID );
	void StatusBroadcast( void );
	void PlayerHelp( void );
	void SendChat( const char *pszText, int iType );

	// Interfaces
	void UnitOrderStart( void );

	IINSWeaponInfo *WeaponInfo( void ) const;
	IINSChatMessages *ChatMessages( void ) const;

	// Misc
	void AddHUDElement( IINSHUDElement *pElement );

	bool IsHUDElementHidden( int iFlags ) const;

	int CreateRoundTimer( char *pszBuffer, int iLength );

	int GetReinforcementTimer( int &iSeconds, bool bPrepareOffset, bool bIgnoreTK );
	bool CreateReinforcementCommanderTimer( char *pszBuffer, int iLength );
	bool CreateReinforcementVGUITimer( char *pszBuffer, int iLength );

	bool ReinforcementData( int &iLeft, int &iPool );
	bool EmergencyDeployments( int &iLeft );
	bool SquadLife( int &iLeft, int &iPool );

	const Color &GetHealthColor( int iType ) const;
	const Color &GetBrightHealthColor( int iType ) const;

	void GetRotation( const Vector &vecDelta, float *flRotation );

	void DamageTaken( int iAmount, int iBits, Vector &vecFrom );

	void UnitOrder( void );

private:
	CINSHUDHelper( );

	// Controller
	void UpdateActiveController( void );

	// Misc
	void CreateTimer( int iSeconds, char *pszBuffer, int iLength );

	int RoundLeft( int iRoundLength );

private:
	CUtlVector< IINSHUDElement* > m_HUDElements;

	Color m_HealthColors[ HEALTHTYPE_COUNT ];

	IINSUnitOrder *m_pUnitOrder;
	IINSWeaponInfo *m_pWeaponInfo;
	IINSChatMessages *m_pChatMessages;

	IINSControlListener *m_pDefaultController, *m_pActiveController;

	C_BaseAnimating *m_pUnitOrderModel;
};

//=========================================================
//=========================================================
void CINSHUDHelper::RegisterDefaultController( IINSControlListener *pController )
{
	m_pDefaultController = pController;
}

//=========================================================
//=========================================================
bool CINSHUDHelper::IsActiveController( IINSControlListener *pController )
{
	return ( m_pActiveController == pController );
}

//=========================================================
//=========================================================
inline CINSHUDHelper *GetINSHUDHelper( void )
{
	return CINSHUDHelper::GetINSHUDHelper( );
}

#endif // INS_HUD_H
