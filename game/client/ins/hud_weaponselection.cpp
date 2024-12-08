//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Weapon selection handling
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "vgui/ilocalize.h"
#include "hud_macros.h"
#include "vgui_entitypanel.h"
#include "clientmode_shared.h"
#include <vgui/ivgui.h>
#include "basic_colors.h"
#include "weapon_ballistic_base.h"
#include "ins_player_shared.h"
#include "hudelement.h"
#include <vgui_controls/panel.h>
#include "view.h"
#include "inshud.h"
#include "iinput.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
#define WEAPONBAR_BG_SIZERATIO ( 1024 / 256 )
#define WEAPONBAR_BG_GAP 0.4f
#define WEAPONBAR_BG_GAP_EXTRA 0.35f

#define WEAPONBAR_HEIGHT 1.0f
#define WEAPONBAR_ZDISTANCE 12.0f
#define WEAPONBAR_LEFT 5.0f

#define WEAPONBAR_NORMAL_DEPTH 5.0f
#define WEAPONBAR_SELECTED_DEPTH 3.0f

//=========================================================
//=========================================================
class CHudWeaponSelection : public CHudElement, public Panel, public IINSMeshDraw, public IINSControlListener, public IINSPlayerDeath
{
	DECLARE_CLASS_SIMPLE( CHudWeaponSelection, Panel );

public:
	CHudWeaponSelection( const char *pElementName );

	void Init( void );
	void VidInit( void );
	void LevelInit( void );
	void Reset( void );
	bool ShouldDraw( void );

	void UserCmd_Close( void );
	void UserCmd_NextWeapon( void );
	void UserCmd_PrevWeapon( void );
	void UserCmd_LastWeapon( void );
	void UserCmd_DropPrimary( void );

private:
	void ApplySchemeSettings( IScheme *pScheme );
	void Draw( void );
	void DrawWeapon( C_BaseCombatWeapon *pWeapon, const Vector &vecOrigin, bool bSelected );
	void DrawBar( CMaterialReference &Material, const Vector &vecOrigin, bool bSelected );

	void SetSelection( bool bState );
	bool IsSelecting( void ) const { return m_bSelectionVisible; }
	bool SetSelectedWeapon( C_BaseCombatWeapon *pWeapon );
	C_BaseCombatWeapon *GetSelectedWeapon( void ) const;
	void CancelWeaponSelection( void );

	void SelectWeapon( void );
	void SelectType( int iSlot );
	void CycleThroughWeapons( int iDelta );
	void SwitchToLastWeapon( void );

	void CreateTexPath( const char *pszSecondaryPath, char *pszBuffer, int iLength );

	bool IsControlActive( void );
	void DoControlClose( void );
	void Selection( void );
	void Scroll( int iType );
	void Number( int iNumber );

	bool IsWeaponSwitchingAllowed( void ) const;

	void PlayerDeath( void );

private:
	bool m_bSelectionVisible;
	float m_flSoonestNextInput;
	int m_iSelectedSlot;

	CMaterialReference m_BG;
	CMaterialReference m_Weapons[ MAX_WEAPONS ];

	CMaterialReference m_Achter;
};

//=========================================================
//=========================================================
#define WPNSELECT_PATH "HUD/wpnselect/"

//=========================================================
//=========================================================
DECLARE_HUD_COMMAND_NAME( CHudWeaponSelection, Close, "CHudWeaponSelection" );
DECLARE_HUD_COMMAND_NAME( CHudWeaponSelection, LastWeapon, "CHudWeaponSelection" );

HOOK_COMMAND( cancelselect, Close );
HOOK_COMMAND( lastinv, LastWeapon );

//=========================================================
//=========================================================
DECLARE_HUDELEMENT( CHudWeaponSelection );

CHudWeaponSelection::CHudWeaponSelection( const char *pElementName )
	: CHudElement( pElementName ), BaseClass( NULL, "HudWeaponSelection" )
{
	Panel *pParent = g_pClientMode->GetViewport( );
	SetParent( pParent );

	SetPaintBackgroundEnabled( false );
	SetHiddenBits( HIDEHUD_DEAD );

	GetINSHUDHelper( )->RegisterDefaultController( this );
}

//=========================================================
//=========================================================
void CHudWeaponSelection::Init( void )
{
	Reset( );

	//gWR.Init( );
}

//=========================================================
//=========================================================
void CHudWeaponSelection::VidInit( void )
{
	char szFullPath[ 128 ];

	// load bg tex
	CreateTexPath( "bg", szFullPath, sizeof( szFullPath ) );
	m_BG.Init( szFullPath, TEXTURE_GROUP_VGUI );

	// load weapon sprites
	char szWeaponPath[ 64 ];

	for( int i = 0; i < MAX_WEAPONS; i++ )
	{
		Assert( g_WeaponData[ i ].m_pszWeaponName );

		CMaterialReference &Material = m_Weapons[ i ];

		Q_snprintf( szWeaponPath, sizeof( szWeaponPath ), "icons/%s", g_WeaponData[ i ].m_pszWeaponName );
		CreateTexPath( szWeaponPath, szFullPath, sizeof( szFullPath ) );

		if( filesystem->FileExists( VarArgs( "materials/%s.vmt", szFullPath ) ) )
		{
			Material.Init( szFullPath, TEXTURE_GROUP_OTHER );
		}
		else
		{
			CreateTexPath( "icons/default", szFullPath, sizeof( szFullPath ) );
			Material.Init( szFullPath, TEXTURE_GROUP_OTHER );
		}
	}

	// load additional
	CreateTexPath( "aicons/achter", szFullPath, sizeof( szFullPath ) );
	m_Achter.Init( szFullPath, TEXTURE_GROUP_OTHER );

	// reset
	Reset( );
}

//=========================================================
//=========================================================
void CHudWeaponSelection::LevelInit( void )
{
	m_flSoonestNextInput = 0.0f;
}

//=========================================================
//=========================================================
void CHudWeaponSelection::Reset( void )
{
	m_bSelectionVisible = false;
	m_flSoonestNextInput = 0.0f;
	m_iSelectedSlot = WEAPON_INVALID;

	//gWR.Reset( );
}

//=========================================================
//=========================================================
void CHudWeaponSelection::ApplySchemeSettings( IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );
}

//=========================================================
//=========================================================
bool CHudWeaponSelection::ShouldDraw( void )
{
	if( !CHudElement::ShouldDraw( ) || !m_bSelectionVisible )
		return false;

	// work out if we can select etc
	if( !IsWeaponSwitchingAllowed( ) )
	{
		SetSelection( false );
		return false;
	}

	return true;
}

//=========================================================
//=========================================================
void CHudWeaponSelection::Draw( void )
{
	float flTotalGap = WEAPONBAR_HEIGHT + WEAPONBAR_BG_GAP;

	if( !ShouldDraw( ) )
		return;

	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer( );

	if( !pPlayer )
		return;

	Vector vecOrigin, vecRight;
	bool bSelected;

	vecRight = CurrentViewRight( );
	vecOrigin = CurrentViewOrigin( ) + ( WEAPONBAR_ZDISTANCE * CurrentViewForward( ) ) - ( vecRight * WEAPONBAR_LEFT );

	for( int i = 0; i < WEAPONSLOT_COUNT; i++ )
	{
		C_BaseCombatWeapon *pWeapon = pPlayer->GetWeapon( i );

		if( !pWeapon )
			continue;

		bSelected = ( i == m_iSelectedSlot );

		DrawWeapon( pWeapon, vecOrigin, bSelected );

		vecOrigin -= CurrentViewUp( ) * ( flTotalGap + ( bSelected ? WEAPONBAR_BG_GAP_EXTRA : 0.0f ) );
	}
}

//=========================================================
//=========================================================
void CHudWeaponSelection::DrawWeapon( C_BaseCombatWeapon *pWeapon, const Vector &vecOrigin, bool bSelected )
{
	// draw background
	DrawBar( m_BG, vecOrigin, bSelected );

	// draw weapon
	DrawBar( m_Weapons[ pWeapon->GetWeaponID( ) ], vecOrigin, bSelected );

	// do weapon dependant drawing
	if( pWeapon->GetWeaponClass( ) == WEAPONCLASS_RIFLE || pWeapon->GetWeaponClass( ) == WEAPONCLASS_SNIPER )
		DrawBar( m_Achter, vecOrigin, bSelected );
}

//=========================================================
//=========================================================
void CHudWeaponSelection::DrawBar( CMaterialReference &Material, const Vector &vecOrigin, bool bSelected )
{
	// define color
	Vector4D Alpha( 1.0f, 1.0f, 1.0f, bSelected ? 1.0f : 0.75f );
	float *pAlpha = Alpha.Base( );

	// work out sizes etc
	Vector vecUp, vecRight;
	vecUp = CurrentViewUp( );
	vecRight = CurrentViewRight( );

	static float  flHeight, flWidth; 
	flHeight = WEAPONBAR_HEIGHT;
	flWidth = WEAPONBAR_HEIGHT * WEAPONBAR_BG_SIZERATIO;

	Vector vecDepth = ( CurrentViewForward( ) * ( bSelected ? WEAPONBAR_SELECTED_DEPTH : WEAPONBAR_NORMAL_DEPTH ) );

	// start drawing
	materials->Bind( Material, TEXTURE_GROUP_VGUI );

	CMeshBuilder Builder;
	IMesh *pMesh = materials->GetDynamicMesh( );

	Builder.Begin( pMesh, MATERIAL_QUADS, 1 );

	// top left
	Builder.Color4fv( pAlpha );
	Builder.TexCoord2f( 0.0f, 0.0f, 0.0f );
	Builder.Position3fv( ( vecOrigin + ( vecRight * -flWidth ) + ( vecUp * flHeight ) ).Base( ) );
	Builder.AdvanceVertex( );

	// top right
	Builder.Color4fv( pAlpha );
	Builder.TexCoord2f( 0.0f, 1.0f, 0.0f );
	Builder.Position3fv( ( vecOrigin + ( vecRight * flWidth ) + ( vecUp * flHeight ) + vecDepth ).Base( ) );
	Builder.AdvanceVertex( );

	// bottom right
	Builder.Color4fv( pAlpha );
	Builder.TexCoord2f( 0.0f, 1.0f, 1.0f );
	Builder.Position3fv( ( vecOrigin + ( vecRight * flWidth ) + ( vecUp * -flHeight ) + vecDepth ).Base( ) );
	Builder.AdvanceVertex();

	// bottom left
	Builder.Color4fv( pAlpha );
	Builder.TexCoord2f( 0.0f, 0.0f, 1.0f );
	Builder.Position3fv( ( vecOrigin + ( vecRight * -flWidth ) + ( vecUp * -flHeight ) ).Base( ) );
	Builder.AdvanceVertex( );

	Builder.End( );

	pMesh->Draw( );
}

//=========================================================
//=========================================================
void CHudWeaponSelection::CancelWeaponSelection( void )
{
	if( !ShouldDraw( ) )
		return;

	SetSelection( false );

	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer( );

	if( pPlayer )
		pPlayer->EmitSound( "Player.WeaponSelectionClose" );
}

//=========================================================
//=========================================================
void CHudWeaponSelection::SetSelection( bool bState )
{
	m_bSelectionVisible = bState;

	if( bState )
		ControlTakeFocus( );
}

//=========================================================
//=========================================================
bool CHudWeaponSelection::SetSelectedWeapon( C_BaseCombatWeapon *pSelectedWeapon )
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer( );

	if( !pPlayer || !pPlayer->IsAllowedToSwitchWeapons( ) )
		return false;

	if( !IsWeaponSwitchingAllowed( ) )
		return false;

	// update selected slot
	if( pSelectedWeapon )
		m_iSelectedSlot = ToINSWeapon( pSelectedWeapon )->GetPlayerSlot( );
	else
		m_iSelectedSlot = WEAPONSLOT_INVALID;

	// start selecting if valid weapon
	if( !IsSelecting( ) && pSelectedWeapon )
		SetSelection( true );

	return true;
}

//=========================================================
//=========================================================
void CHudWeaponSelection::SelectWeapon( void )
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer( );

	if( !pPlayer )
		return;

	C_BaseCombatWeapon *pSelectedWeapon = GetSelectedWeapon( );

    if( pSelectedWeapon )
	{
		::input->MakeWeaponSelection( pSelectedWeapon );
		SetSelectedWeapon( NULL );
	}

	SetSelection( false );
}

//=========================================================
//=========================================================
void CHudWeaponSelection::SelectType( int iWeaponSlot )
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer( );

	if( !pPlayer )
		return;

	C_BaseCombatWeapon *pWeapon = pPlayer->GetWeapon( iWeaponSlot );

	if( pWeapon )
		SetSelectedWeapon( pWeapon );
}

//=========================================================
//=========================================================
void CHudWeaponSelection::CycleThroughWeapons( int iType )
{
	if( !IsWeaponSwitchingAllowed( ) )
		return;

	int iDelta = ( iType == INSHUD_SCROLLUP ) ? -1 : 1;

	if( gpGlobals->curtime <= m_flSoonestNextInput )
		return;

	// find local player
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer( );

	if( !pPlayer )
		return;

	C_BaseCombatWeapon *pNextWeapon, *pCurWeapon;
	int iStartSlot, iWpnSlot;
	bool bFreshEntry = true;
	
	pNextWeapon = pCurWeapon = NULL;

	iStartSlot = m_iSelectedSlot;

	if( iStartSlot == WEAPONSLOT_INVALID )
		iStartSlot = 0;

	iWpnSlot = iStartSlot;

	while( !pNextWeapon )
	{
		iWpnSlot += iDelta;

		// ensure a wrap around
		if( iWpnSlot >= WEAPONSLOT_COUNT )
			iWpnSlot = 0;
		else if( iWpnSlot < 0 )
			iWpnSlot = WEAPONSLOT_COUNT - 1;

		// ensure we haven't gone all the way around
		if( iWpnSlot == iStartSlot )
			break;

		bFreshEntry = false;
		pCurWeapon = pPlayer->GetWeapon( iWpnSlot );

		// select if valid
		if( pCurWeapon && pCurWeapon->CanDeploy( ) )
		{
			pNextWeapon = pCurWeapon;
			break;
		}
	}

	// start selection process
	if( !pNextWeapon )
		return;

	// ensure that the player can't scroll too fast
	m_flSoonestNextInput = gpGlobals->curtime + 0.075f;

	// set the selected weapon
	if( !SetSelectedWeapon( pNextWeapon ) )
		return;

	// print log when debugging
#ifdef _DEBUG

	Msg( "Scrolled Weapon: %s\n", pNextWeapon->GetClassname( ) );

#endif

	// emit a clicky move sound
	pPlayer->EmitSound( "Player.WeaponSelectionMoveSlot" );
}

//=========================================================
//=========================================================
void CHudWeaponSelection::SwitchToLastWeapon( void )
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer( );

	if( pPlayer )
		::input->MakeWeaponSelection( pPlayer->GetLastWeapon( ) );
}

//=========================================================
//=========================================================
C_BaseCombatWeapon *CHudWeaponSelection::GetSelectedWeapon(void) const
{
	if( m_iSelectedSlot == WEAPONSLOT_INVALID )
		return NULL;

	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer( );
	return ( pPlayer ? pPlayer->GetWeapon( m_iSelectedSlot ) : NULL );
}

//=========================================================
//=========================================================
void CHudWeaponSelection::CreateTexPath( const char *pszPath, char *pszBuffer, int iLength )
{
	Q_strncpy( pszBuffer, WPNSELECT_PATH, iLength );
	Q_strncat( pszBuffer, pszPath, iLength, COPY_ALL_CHARACTERS );
}

//=========================================================
//=========================================================
bool CHudWeaponSelection::IsControlActive( void )
{
	return IsSelecting( );
}

//=========================================================
//=========================================================
void CHudWeaponSelection::Scroll( int iType )
{
	CycleThroughWeapons( iType );
}

//=========================================================
//=========================================================
void CHudWeaponSelection::Number( int iNumber )
{
	switch( iNumber )
	{
		case 1:
			SelectType( WEAPONSLOT_PRIMARY );
			break;

		case 2:
			SelectType( WEAPONSLOT_SECONDARY );
			break;

		/*case 3:
			SelectType( WEAPONSLOT_EQUIPMENT_ONE );
			break;

		case 4:
			SelectType( WEAPONSLOT_EQUIPMENT_TWO );
			break;*/

		case 5:
			SelectType( WEAPONSLOT_MELEE );
			break;

		/*case 6:
			SelectType( WEAPONSLOT_NONE_ONE );
			break;

		case 7:
			SelectType( WEAPONSLOT_NONE_TWO );
			break;*/
	}
}

//=========================================================
//=========================================================
void CHudWeaponSelection::Selection( void )
{
	SelectWeapon( );
}

//=========================================================
//=========================================================
void CHudWeaponSelection::DoControlClose( void )
{
	CancelWeaponSelection( );
}

//=========================================================
//=========================================================
void CHudWeaponSelection::UserCmd_Close( void )
{
	CancelWeaponSelection( );
}

//=========================================================
//=========================================================
void CHudWeaponSelection::UserCmd_LastWeapon( void )
{
	if( !ShouldDraw( ) )
		return;

	SwitchToLastWeapon( );
}

//=========================================================
//=========================================================
bool CHudWeaponSelection::IsWeaponSwitchingAllowed( void ) const
{
	C_INSPlayer *pPlayer = C_INSPlayer::GetLocalPlayer( );

	if( !pPlayer || !pPlayer->IsAlive( ) )
		return false;

	if( ( pPlayer->GetPlayerFlags( ) & FL_PLAYER_IRONSIGHTS ) != 0 )
		return false;

	C_BaseCombatWeapon *pWeapon = pPlayer->GetActiveWeapon( );
	return ( !pWeapon || pWeapon->CanHolster( ) );
}

//=========================================================
//=========================================================
void CHudWeaponSelection::PlayerDeath( void )
{
	SetSelection( false );
}