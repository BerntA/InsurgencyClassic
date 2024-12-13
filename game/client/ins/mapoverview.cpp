//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: MiniMap.cpp: implementation of the CMiniMap class.
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "mapoverview.h"
#include <vgui/isurface.h>
#include <vgui/ilocalize.h>
#include <filesystem.h>
#include <keyvalues.h>
#include <convar.h>
#include "mathlib/mathlib.h"
#include <game/client/iviewport.h>
#include <igameresources.h>
#include "spectatorgui.h"
#include "c_playerresource.h"
#include "c_ins_obj.h"
#include "mapname_utils.h"
#include "ins_player_shared.h"
#include "ins_gamerules.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static ConVar overview_health( "overview_health", "1", 0, "Show player's health in map overview.\n" );
static ConVar overview_names ( "overview_names",  "1", 0, "Show player's names in map overview.\n" );
static ConVar overview_tracks( "overview_tracks", "1", 0, "Show player's tracks in map overview.\n" );
static ConVar overview_locked( "overview_locked", "1", 0, "Locks map angle, doesn't follow view angle.\n" );
static ConVar overview_alpha( "overview_alpha",  "1.0", 0, "Overview map translucency.\n" );

static int AdjustValue( int curValue, int targetValue, int amount )
{
	if ( curValue > targetValue )
	{
		curValue -= amount;

		if ( curValue < targetValue )
			curValue = targetValue;
	}
	else if ( curValue < targetValue )
	{
		curValue += amount;

		if ( curValue > targetValue )
			curValue = targetValue;
	}

	return curValue;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


using namespace vgui;

CMapOverview::CMapOverview( vgui::Panel *pParent )
	: BaseClass( pParent, PANEL_OVERVIEW )
{
	SetBounds( 0,0, 256, 256 );
	SetPaintBackgroundEnabled( true );
	SetVisible( false );

	// Pongles [
	m_bMapUpdate = true;
	// Pongles ]

	// Make sure we actually have the font...
	vgui::IScheme *pScheme = vgui::scheme()->GetIScheme( GetScheme() );
	
	m_hIconFont = pScheme->GetFont( "DefaultSmall" );
	
	m_nMapTextureID = -1;
	m_MapKeyValues = NULL;

	m_MapOrigin = Vector( 0, 0, 0 );
	m_fMapScale = 1.0f;
	m_bFollowAngle = false;
	SetMode(MAP_MODE_INSET);

	m_fZoom = 3.0f;
	m_MapCenter = Vector2D( 512, 512 );
	m_ViewOrigin = Vector2D( 512, 512 );
	m_fViewAngle = 0;
	m_fTrailUpdateInterval = 1.0f;

	m_bShowNames = true;
	m_bShowHealth = true;
	m_bShowTrails = true;

	m_flChangeSpeed = 1000;
	m_flIconSize = 64.0f;

	m_ObjectCounterID = 1;

	Reset();
	
	Q_memset( m_Players, 0, sizeof(m_Players) );

	InitTeamColorsAndIcons();

	// register for events as client listener
	gameeventmanager->AddListener( this, "game_newmap", false );
	//gameeventmanager->AddListener( this, "round_start", false );
	gameeventmanager->AddListener( this, "player_connect", false );
	gameeventmanager->AddListener( this, "player_info", false );
	gameeventmanager->AddListener( this, "player_team", false );
	gameeventmanager->AddListener( this, "player_spawn", false );
	gameeventmanager->AddListener( this, "player_death", false );
	gameeventmanager->AddListener( this, "player_disconnect", false );
}

#define ICON_TEAM_PATH "VGUI/teams/global/icon"
#define ICON_OBJECTIVE_PATH "sprites/overview/obj_icon"

void CMapOverview::InitTeamColorsAndIcons()
{
	Q_memset( m_ObjectIcons, 0, sizeof(m_ObjectIcons) );

	m_iTeamIcon = AddIconTexture(ICON_TEAM_PATH);
	m_iObjectiveIcon = AddIconTexture(ICON_OBJECTIVE_PATH);

	m_TextureIDs.RemoveAll();
}

int CMapOverview::AddIconTexture(const char *filename)
{
	int index = m_TextureIDs.Find( filename );

    if ( m_TextureIDs.IsValidIndex( index ) )
	{
		// already known, return texture ID
		return m_TextureIDs.Element(index);
	}

	index = surface()->CreateNewTextureID();
	surface()->DrawSetTextureFile( index , filename, true, false);

	m_TextureIDs.Insert( filename, index );

	return index;
}

void CMapOverview::ApplySchemeSettings(vgui::IScheme *scheme)
{
	BaseClass::ApplySchemeSettings( scheme );

	SetBgColor( Color( 0,0,0,255 ) );
	SetPaintBackgroundEnabled( true );
}

CMapOverview::~CMapOverview()
{
	if ( m_MapKeyValues )
		m_MapKeyValues->deleteThis();

	gameeventmanager->RemoveListener(this);

	//TODO release Textures ? clear lists
}

void CMapOverview::UpdatePlayers()
{
	for ( int i = 1; i<= gpGlobals->maxClients; i++)
	{
		// update from global player resources
		if ( g_PR->IsConnected(i) )
		{
			MapPlayer_t *player = &m_Players[i-1];

			player->alive = GameResources()->IsAlive( i );

			if ( player->team != GameResources()->GetTeam( i ) )
			{
				player->team = GameResources()->GetTeam( i );
				player->color = INSRules()->TeamColor(player->team);
			}
		}

		C_BasePlayer *pPlayer = UTIL_PlayerByIndex( i );

		if ( !pPlayer )
			continue;
		
		// don't update if player is dormant
		if ( pPlayer->IsDormant() )
			continue;

		// update position of active players in our PVS
		Vector position = pPlayer->EyePosition();
		QAngle angles = pPlayer->EyeAngles();

		SetPlayerPositions( i-1, position, angles );
	}
}

void CMapOverview::UpdatePlayerTrails()
{
	if ( m_fNextTrailUpdate > m_fWorldTime )
		return;

	m_fNextTrailUpdate = m_fWorldTime + 1.0f; // update once a second

	for (int i=0; i<MAX_PLAYERS; i++)
	{
		MapPlayer_t *p = &m_Players[i];
		
		// no trails for spectators or dead players
		if ( (p->team <= TEAM_SPECTATOR) || !p->alive )
		{
			continue;
		}

		// move old trail points 
		for ( int j=MAX_TRAIL_LENGTH-1; j>0; j--)
		{
			p->trail[j]=p->trail[j-1];
		}

		p->trail[0] = WorldToMap ( p->position );
	}
}

void CMapOverview::UpdateZoom()
{
    if ( m_fZoom == m_fTragetZoom )
		return;

	if ( m_fZoom < m_fTragetZoom )
	{
		m_fZoom += gpGlobals->frametime * m_fZoomSpeed;

		if ( m_fZoom > m_fTragetZoom )
			m_fZoom = m_fTragetZoom;
	}
	else 
	{
		m_fZoom -= gpGlobals->frametime * m_fZoomSpeed;

		if ( m_fZoom < m_fTragetZoom )
			m_fZoom = m_fTragetZoom;
	}
}

void CMapOverview::UpdateFollowEntity()
{
	if ( m_nFollowEntity != 0 )
	{
		C_BaseEntity *ent = ClientEntityList().GetEnt( m_nFollowEntity );

		if ( ent )
		{
			Vector position = ent->EyePosition();
			QAngle angle = ent->EyeAngles();

			if ( m_nFollowEntity <= MAX_PLAYERS )
			{
				SetPlayerPositions( m_nFollowEntity-1, position, angle );
			}

			SetCenter( WorldToMap(position) );
			SetAngle( angle[YAW] );
		}
	}
	else
	{
		SetCenter( Vector2D(OVERVIEW_MAP_SIZE/2,OVERVIEW_MAP_SIZE/2) );
		SetAngle( 0 );
	}
}

void CMapOverview::Paint()
{
	UpdateZoom();

	UpdateFollowEntity();

	UpdateObjects();

	UpdatePlayers();

	UpdatePlayerTrails();

	DrawMapTexture();

	DrawMapPlayerTrails();

	DrawObjects();

	DrawMapPlayers();

	DrawMapObjectives();

	DrawCamera();

	BaseClass::Paint();
}

bool CMapOverview::CanPlayerBeSeen(MapPlayer_t *player)
{
	C_INSPlayer *localPlayer = C_INSPlayer::GetLocalPlayer();

	if ( !localPlayer || !player )
		return false;

	// don't draw ourself
	if ( localPlayer->entindex() == (player->index+1) )
		return false;

	// if local player is on spectator team, he can see everyone
	if ( localPlayer->GetTeamID() <= TEAM_SPECTATOR )
		return true;

	// we never track unassigned or real spectators
	if ( player->team <= TEAM_SPECTATOR )
		return false;

	// if observer is an active player, check mp_forcecamera:

	// Pongles [
	if ( INSRules( )->GetDeadCamMode( ) == OBS_ALLOWMODE_NONE )
		return false;

	int iDeadCamTargets = INSRules( )->GetDeadCamTargets( );

	if ( iDeadCamTargets == OBS_ALLOWTARGETS_TEAM || iDeadCamTargets == OBS_ALLOWTARGETS_SQUAD )
	{
		// true if both players are on the same team
		return (localPlayer->GetTeamID() == player->team );

		if(iDeadCamTargets == OBS_ALLOWTARGETS_SQUAD)
		{
		}
			// NOTE: Check Squads
	}
	// Pongles ]

	// by default we can see all players
	return true;
}

// usually name rule is same as health rule
bool CMapOverview::CanPlayerNameBeSeen(MapPlayer_t *player)
{
	C_INSPlayer *localPlayer = C_INSPlayer::GetLocalPlayer();

	if ( !localPlayer )
		return false;

	// real spectators can see everything
	if ( localPlayer->GetTeamID() <= TEAM_SPECTATOR )
		return true;

	// Pongles [
	int iDeadCamTargets = INSRules( )->GetDeadCamTargets( );

	if ( iDeadCamTargets == OBS_ALLOWTARGETS_TEAM || iDeadCamTargets == OBS_ALLOWTARGETS_SQUAD )
	{
		// true if both players are on the same team
		return (localPlayer->GetTeamID() == player->team );

		if(iDeadCamTargets == OBS_ALLOWTARGETS_SQUAD)
		{
			// NOTE: Check Squads
		}
	}
	// Pongles ]

	return true;
}

void CMapOverview::SetPlayerPositions(int index, const Vector &position, const QAngle &angle)
{
	MapPlayer_t *p = &m_Players[index];

	p->angle = angle;
	p->position = position;
}

void CMapOverview::Think( void )
{
	UpdateMap();

	if(m_fNextUpdateTime >= gpGlobals->curtime)
		return;

	// update settings
	m_bShowNames = overview_names.GetBool();
	m_bShowHealth = overview_health.GetBool();
	m_bFollowAngle = !overview_locked.GetBool();
	m_fTrailUpdateInterval = overview_tracks.GetInt();

	m_fWorldTime = gpGlobals->curtime;

	m_fNextUpdateTime = gpGlobals->curtime + 0.2f; // update 5 times a second

	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();

	if ( !pPlayer )
		return;

	int specmode = GetSpectatorMode();

	if ( specmode == OBS_MODE_IN_EYE || specmode == OBS_MODE_CHASE )
	{
		// follow target
		SetFollowEntity( GetSpectatorTarget() );
	}
	else 
	{
		// follow ourself otherwise
		SetFollowEntity( pPlayer->entindex() );
	}
}

void CMapOverview::Reset( void )
{
	m_fNextUpdateTime = 0;
}


CMapOverview::MapPlayer_t* CMapOverview::GetPlayerByUserID( int userID )
{
	for (int i=0; i<MAX_PLAYERS; i++)
	{
		MapPlayer_t *player = &m_Players[i];

		if ( player->userid == userID )
			return player;
	}

	return NULL;
}

bool CMapOverview::IsInPanel(Vector2D &pos)
{
	int x,y,w,t;

	GetBounds( x,y,w,t );

	return ( pos.x >= 0 && pos.x < w && pos.y >= 0 && pos.y < t );
}

void CMapOverview::DrawMapTexture()
{
	// now draw a box around the outside of this panel
	int x0, y0, x1, y1;
	int wide, tall;

	GetSize(wide, tall);
	x0 = 0; y0 = 0; x1 = wide - 2; y1 = tall - 2 ;

	if ( m_nMapTextureID < 0 )
		return;
	
	Vertex_t points[4] =
	{
		Vertex_t( MapToPanel ( Vector2D(0,0) ), Vector2D(0,0) ),
		Vertex_t( MapToPanel ( Vector2D(OVERVIEW_MAP_SIZE-1,0) ), Vector2D(1,0) ),
		Vertex_t( MapToPanel ( Vector2D(OVERVIEW_MAP_SIZE-1,OVERVIEW_MAP_SIZE-1) ), Vector2D(1,1) ),
		Vertex_t( MapToPanel ( Vector2D(0,OVERVIEW_MAP_SIZE-1) ), Vector2D(0,1) )
	};

	int alpha = 255.0f * overview_alpha.GetFloat(); clamp( alpha, 1, 255 );
	
	surface()->DrawSetColor( 255,255,255, alpha );
	surface()->DrawSetTexture( m_nMapTextureID );
	surface()->DrawTexturedPolygon( 4, points );
}

void CMapOverview::DrawMapPlayerTrails()
{
	if ( m_fTrailUpdateInterval <= 0 )
		return; // turned off

	for (int i=0; i<MAX_PLAYERS; i++)
	{
		MapPlayer_t *player = &m_Players[i];

		if ( !CanPlayerBeSeen(player) )
			continue;
		
		player->trail[0] = WorldToMap ( player->position );

		for ( int i=0; i<(MAX_TRAIL_LENGTH-1); i++)
		{
			if ( player->trail[i+1].x == 0 && player->trail[i+1].y == 0 )
				break;

			Vector2D pos1 = MapToPanel( player->trail[i] );
			Vector2D pos2 = MapToPanel( player->trail[i+1] );

			int intensity = 255 - float(255.0f * i) / MAX_TRAIL_LENGTH;

			Vector2D dist = pos1 - pos2;
			
			// don't draw too long lines, player probably teleported
			if ( dist.LengthSqr() < (128*128) )
			{	
				surface()->DrawSetColor( player->color[0], player->color[1], player->color[2], intensity );
				surface()->DrawLine( pos1.x, pos1.y, pos2.x, pos2.y );
			}
		}
	}
}

void CMapOverview::DrawObjects( )
{
	surface()->DrawSetTextFont( m_hIconFont );

	for (int i=0; i<m_Objects.Count(); i++)
	{
		MapObject_t *obj = &m_Objects[i];


		const char *text = NULL;

		if ( Q_strlen(obj->name) > 0 )
			text = obj->name;

		// draw icon
		if ( !DrawIcon( obj->icon, obj->position, obj->size, obj->angle[YAW], NULL, text, &obj->color ) )
			continue;

		// draw text
	}

}

bool CMapOverview::DrawIcon( int textureID, Vector pos, float scale, float angle, const Color *iconColor, const char *text, const Color *textColor )
{
	if(pos == vec3_origin)
		return true;

	Vector offset;	offset.z = 0;
	
	Vector2D pospanel = WorldToMap( pos );
	pospanel = MapToPanel( pospanel );

	if ( !IsInPanel( pospanel ) )
		return false; // player is not within overview panel

	offset.x = -scale;	offset.y = scale;
	VectorYawRotate( offset, angle, offset );
	Vector2D pos1 = WorldToMap( pos + offset );

	offset.x = scale;	offset.y = scale;
	VectorYawRotate( offset, angle, offset );
	Vector2D pos2 = WorldToMap( pos + offset );

	offset.x = scale;	offset.y = -scale;
	VectorYawRotate( offset, angle, offset );
	Vector2D pos3 = WorldToMap( pos + offset );

	offset.x = -scale;	offset.y = -scale;
	VectorYawRotate( offset, angle, offset );
	Vector2D pos4 = WorldToMap( pos + offset );

	Vertex_t points[4] =
	{
		Vertex_t( MapToPanel ( pos1 ), Vector2D(0,0) ),
		Vertex_t( MapToPanel ( pos2 ), Vector2D(1,0) ),
		Vertex_t( MapToPanel ( pos3 ), Vector2D(1,1) ),
		Vertex_t( MapToPanel ( pos4 ), Vector2D(0,1) )
	};

	Color _iconColor;

	if(!iconColor)
		_iconColor.SetColor(255, 255, 255, 255);
	else
		_iconColor = *iconColor;

	surface()->DrawSetTexture( textureID );
	surface()->DrawSetColor( _iconColor );
	surface()->DrawTexturedPolygon( 4, points );

	int d = GetPixelOffset( scale);

	pospanel.y += d + 4;

	if ( text && textColor )
	{
		wchar_t iconText[ MAX_PLAYER_NAME_LENGTH*2 ];

		g_pVGuiLocalize->ConvertANSIToUnicode( text, iconText, sizeof( iconText ) );

		int wide, tall;
		surface()->GetTextSize( m_hIconFont, iconText, wide, tall );

		int x = pospanel.x-(wide/2);
		int y = pospanel.y;

		// draw black shadow text
		surface()->DrawSetTextColor( 0, 0, 0, 255 );
		surface()->DrawSetTextPos( x+1, y );
		surface()->DrawPrintText( iconText, wcslen(iconText) );

		// draw name in color 
		surface()->DrawSetTextColor( textColor->r(), textColor->g(), textColor->b(), 255 );
		surface()->DrawSetTextPos( x, y );
		surface()->DrawPrintText( iconText, wcslen(iconText) );
	}

	return true;
}

int CMapOverview::GetPixelOffset( float height )
{
	Vector2D pos2 = WorldToMap( Vector( height,0,0) );
	pos2 = MapToPanel( pos2 );

	Vector2D pos3 = WorldToMap( Vector(0,0,0) );
	pos3 = MapToPanel( pos3 );

	int a = pos2.y-pos3.y; 
	int b = pos2.x-pos3.x;

	return (int)sqrt((float)(a*a+b*b)); // number of panel pixels for "scale" units in world
}

void CMapOverview::DrawMapPlayers()
{
	surface()->DrawSetTextFont( m_hIconFont );

	Color colorGreen( 0, 255, 0, 255 );	// health bar color
	
	for (int i=0; i<MAX_PLAYERS; i++)
	{
		MapPlayer_t *player = &m_Players[i];

		if ( !CanPlayerBeSeen( player ) )
			continue;

		// don't draw dead players / spectators
		if ( !player->alive )
			continue;
		
		const char *name = NULL;

		if ( m_bShowNames && CanPlayerNameBeSeen( player ) )
			name = player->name;

		DrawIcon( m_iTeamIcon, player->position, m_flIconSize, player->angle[YAW], &player->color, name, &player->color );
	}
}

#define OBJECTIVE_ICON_SIZE 72

void CMapOverview::DrawMapObjectives()
{
	const CUtlVector< C_INSObjective* > &Objectives = C_INSObjective::GetObjectiveList( );

	for(int i = 0; i < Objectives.Count(); i++)
	{
		C_INSObjective *pObjective = C_INSObjective::GetObjective(i);

		if(!pObjective)
			continue;

		Color ObjColor(pObjective->GetColor());
		ObjColor[3] = 180;

		DrawIcon(m_iObjectiveIcon, pObjective->GetMarker(), OBJECTIVE_ICON_SIZE, 0.0f, &ObjColor, pObjective->GetName(), &ObjColor);
	}
}

Vector2D CMapOverview::WorldToMap( const Vector &worldpos )
{
	Vector2D offset( worldpos.x - m_MapOrigin.x, worldpos.y - m_MapOrigin.y);

	offset.x /=  m_fMapScale;
	offset.y /= -m_fMapScale;

	return offset;
}


Vector2D CMapOverview::MapToPanel( const Vector2D &mappos )
{
	int pwidth, pheight; 
	Vector2D panelpos;
	float viewAngle = m_fViewAngle - 90.0f;

	GetSize(pwidth, pheight);

	Vector offset;
	offset.x = mappos.x - m_MapCenter.x;
	offset.y = mappos.y - m_MapCenter.y;
	offset.z = 0;

	if ( !m_bFollowAngle )
	{
		if ( m_bRotateMap )
			viewAngle = 90.0f;
		else
			viewAngle = 0.0f;
	}

	VectorYawRotate( offset, viewAngle, offset );

	offset.x *= m_fZoom/OVERVIEW_MAP_SIZE;
	offset.y *= m_fZoom/OVERVIEW_MAP_SIZE;

	//offset.x *= 1.0f/OVERVIEW_MAP_SIZE;
	//offset.y *= 1.0f/OVERVIEW_MAP_SIZE;

	panelpos.x = (pwidth * 0.5f) + (pheight * offset.x);
	panelpos.y = (pheight * 0.5f) + (pheight * offset.y);

	return panelpos;
}

void CMapOverview::SetTime( float time )
{
	m_fWorldTime = time;
}

// Pongles [
void CMapOverview::UpdateMap(void)
{
	if(!m_bMapUpdate)
		return;

	m_bMapUpdate = true;

	char szMapName[MAX_MAPNAME_LENGTH];
	GetVGUIMapName(szMapName);

	// load new KeyValues

	if ( m_MapKeyValues && Q_strcmp( szMapName, m_MapKeyValues->GetName() ) == 0 )
	{
		return;	// map didn't change
	}

	if ( m_MapKeyValues )
		m_MapKeyValues->deleteThis();

	m_MapKeyValues = new KeyValues( szMapName );

	char tempfile[MAX_PATH];
	Q_snprintf( tempfile, sizeof( tempfile ), "resource/overviews/%s.txt", szMapName );
	
	if (!m_MapKeyValues->LoadFromFile(filesystem, tempfile, "GAME"))
	{
		DevMsg( 1, "Error! CMapOverview::SetMap: couldn't load file %s.\n", tempfile );
		m_nMapTextureID = -1;
		return;
	}

	// TODO release old texture ?

	m_nMapTextureID = surface()->CreateNewTextureID();

	//if we have not uploaded yet, lets go ahead and do so
	surface()->DrawSetTextureFile( m_nMapTextureID, m_MapKeyValues->GetString("material"), true, false);

	int wide, tall;

	surface()->DrawGetTextureSize( m_nMapTextureID, wide, tall );

	if ( wide != tall )
	{
		DevMsg( 1, "Error! CMapOverview::SetMap: map image must be a square.\n" );
		m_nMapTextureID = -1;
		return;
	}

	m_MapOrigin.x	= m_MapKeyValues->GetInt("pos_x");
	m_MapOrigin.y	= m_MapKeyValues->GetInt("pos_y");
	m_fMapScale		= m_MapKeyValues->GetFloat("scale", 1.0f);
	m_bRotateMap	= m_MapKeyValues->GetInt("rotate")!=0;
	m_fFullZoom		= m_MapKeyValues->GetFloat("zoom", 1.0f );
	// remove all objects and reset players
	m_Objects.RemoveAll();

	
	m_fNextTrailUpdate = m_fWorldTime;
}
// Pongles ]

void CMapOverview::ResetRound()
{
	/*for (int i=0; i<MAX_PLAYERS; i++)
	{
		MapPlayer_t *p = &m_Players[i];
		
		if ( p->team > TEAM_SPECTATOR )
		{
			p->health = 100;
		}

		Q_memset( p->trail, 0, sizeof(p->trail) );

		p->position = Vector( 0, 0, 0 );
	}

	m_Objects.RemoveAll();*/
}

void CMapOverview::OnMousePressed( MouseCode code )
{
	
}

void CMapOverview::DrawCamera()
{
	// draw a red center point
	surface()->DrawSetColor( 255,0,0,255 );
	Vector2D center = MapToPanel( m_ViewOrigin );
	surface()->DrawFilledRect( center.x-2, center.y-2, center.x+2, center.y+2);
}

void CMapOverview::FireGameEvent( IGameEvent *event )
{
	const char * type = event->GetName();

	if ( Q_strcmp(type, "game_newmap") == 0 )
	{
		// Pongles [
		m_bMapUpdate = true;
		// Pongles ]
	}

	/*else if ( Q_strcmp(type, "round_start") == 0 )
	{
		ResetRound();
	}*/

	else if ( Q_strcmp(type,"player_connect") == 0 )
	{
		int index = event->GetInt("index"); // = entity index-1 

		if ( index < 0 || index >= MAX_PLAYERS )
			return;

		MapPlayer_t *player = &m_Players[index];
		
		player->index = index;
		player->userid = event->GetInt("userid");
		Q_strncpy( player->name, event->GetString("name","unknown"), sizeof(player->name) );

		// Reset settings
		Q_memset( player->trail, 0, sizeof(player->trail) );
		player->team = TEAM_UNASSIGNED;
		player->alive = false;
	}

	else if ( Q_strcmp(type,"player_info") == 0 )
	{
		int index = event->GetInt("index"); // entindex

		if ( index < 0 || index >= MAX_PLAYERS )
			return;

		MapPlayer_t *player = &m_Players[index];
		
		player->index = index;
		player->userid = event->GetInt("userid");
		Q_strncpy( player->name, event->GetString("name","unknown"), sizeof(player->name) );
	}

	else if ( Q_strcmp(type,"player_team") == 0 )
	{
		MapPlayer_t *player = GetPlayerByUserID( event->GetInt("userid") );

		if ( !player )
			return;

		player->team = event->GetInt("team");
		player->color = INSRules()->TeamColor(player->team);
	}

	else if ( Q_strcmp(type,"player_death") == 0 )
	{
		MapPlayer_t *player = GetPlayerByUserID( event->GetInt("userid") );

		if ( !player )
			return;

		player->alive = false;
		Q_memset( player->trail, 0, sizeof(player->trail) ); // clear trails
	}

	else if ( Q_strcmp(type,"player_spawn") == 0 )
	{
		MapPlayer_t *player = GetPlayerByUserID( event->GetInt("userid") );

		if ( !player )
			return;

		player->alive = true;
		Q_memset( player->trail, 0, sizeof(player->trail) ); // clear trails
	}

	else if ( Q_strcmp(type,"player_disconnect") == 0 )
	{
		MapPlayer_t *player = GetPlayerByUserID( event->GetInt("userid") );
		
		if ( !player )
			return;

		Q_memset( player, 0, sizeof(MapPlayer_t) ); // clear player field
	}
}

void CMapOverview::SetMode(int mode)
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();

	m_flChangeSpeed = 0; // change size instantly

	/*if ( mode == MAP_MODE_OFF )
	{
		gViewPortInterface->ShowPanel( this, false );		
	}
	else */if ( mode == MAP_MODE_INSET )
	{
		//if ( m_nMode != MAP_MODE_OFF )
		m_flChangeSpeed = 1000; // zoom effect

		m_fZoom = 1.0f; // zoom in effect
		SetZoom( 3.0f, 1.0f );

		if(pPlayer)
			SetFollowEntity( CBasePlayer::GetLocalPlayer()->entindex() );

		//gViewPortInterface->ShowPanel( this, true );		
	}
	else if ( mode == MAP_MODE_FULL )
	{
		//if ( m_nMode != MAP_MODE_OFF )
			m_flChangeSpeed = 1000; // zoom effect

		SetZoom( -1.0f, 0.5 ); // zoom -1 means best full zoom
		SetFollowEntity( 0 );

		//gViewPortInterface->ShowPanel( this, true );		
	}

	// finally set mode
	m_nMode = mode;
}

void CMapOverview::SetCenter(const Vector2D &mappos)
{
	int width, height;

	GetSize( width, height);

	m_ViewOrigin = mappos;
	m_MapCenter = mappos;

	width = OVERVIEW_MAP_SIZE / (m_fZoom*2);
	height = OVERVIEW_MAP_SIZE / (m_fZoom*2);

	if ( m_MapCenter.x < width )
		m_MapCenter.x = width;

	if ( m_MapCenter.x > (OVERVIEW_MAP_SIZE-width) )
		m_MapCenter.x = (OVERVIEW_MAP_SIZE-width);

	if ( m_MapCenter.y < height )
		m_MapCenter.y = height;

	if ( m_MapCenter.y > (OVERVIEW_MAP_SIZE-height) )
		m_MapCenter.y = (OVERVIEW_MAP_SIZE-height);

	//center if in full map mode
	if ( m_fZoom <= m_fFullZoom )
	{
		m_MapCenter.x = OVERVIEW_MAP_SIZE/2;
		m_MapCenter.y = OVERVIEW_MAP_SIZE/2;
	}
}

void CMapOverview::SetFollowAngle(bool state)
{
	m_bFollowAngle = state;
}

void CMapOverview::SetFollowEntity(int entindex)
{
	m_nFollowEntity = entindex;
}

float CMapOverview::GetZoom( void )
{
	return m_fTragetZoom;
}

int CMapOverview::GetMode( void )
{
	return m_nMode;
}

void CMapOverview::SetZoom( float zoom, float time )
{
	m_fTragetZoom = zoom;

	if ( m_fTragetZoom == -1.0f )
	{
		if ( m_nMapTextureID > 0 )
		{
			m_fTragetZoom = m_fFullZoom;
		}
		else
		{
			m_fTragetZoom = 1.0f;
		}
	}
	else if ( m_fTragetZoom < 0.5f )
	{
		m_fTragetZoom = 0.5f;
	}
	else if ( m_fTragetZoom > 5.0f )
	{
		m_fTragetZoom = 5.0f;
	}

	if ( time <= 0 )
	{
		// instantly set new zoom
		m_fZoom = m_fTragetZoom;
		m_fZoomSpeed = 0;
	}
	else
	{
		m_fZoomSpeed = fabs( m_fZoom - m_fTragetZoom )/time;
	}
}

void CMapOverview::SetAngle(float angle)
{
	m_fViewAngle = angle;
}

void CMapOverview::ShowPlayerNames(bool state)
{
	m_bShowNames = state;
}


void CMapOverview::ShowPlayerHealth(bool state)
{
	m_bShowHealth = state;
}

void CMapOverview::ShowPlayerTracks(float seconds)
{
	m_fTrailUpdateInterval = seconds;
}

CMapOverview::MapObject_t* CMapOverview::FindObjectByID(int objectID)
{
	for ( int i = 0; i < m_Objects.Count(); i++ )
	{
		if ( m_Objects[i].objectID == objectID )
			return &m_Objects[i];
	}

	return NULL;
}

int	CMapOverview::AddObject( const char *icon, int entity, float timeToLive, bool update )
{
	MapObject_t obj; Q_memset( &obj, 0, sizeof(obj) );

	obj.objectID = m_ObjectCounterID++;
	obj.index = entity;
	obj.icon = AddIconTexture( icon );
	obj.size = m_flIconSize;
	obj.status = -1;
	obj.update = update;
	
	if ( timeToLive > 0 )
		obj.endtime = gpGlobals->curtime + timeToLive;
	else
		obj.endtime = -1;

	m_Objects.AddToTail( obj );

	return obj.objectID;
}

void CMapOverview::SetObjectText( int objectID, const char *text, Color color )
{
	MapObject_t* obj = FindObjectByID( objectID );

	if ( !obj )
		return;

	if ( text )
	{
		Q_strncpy( obj->name, text, sizeof(obj->name) );
	}
	else
	{
		Q_memset( obj->name, 0, sizeof(obj->name) );
	}

	obj->color = color;
}

void CMapOverview::SetObjectStatus( int objectID, float status )
{
	MapObject_t* obj = FindObjectByID( objectID );

	if ( !obj )
		return;

	obj->status = status;
}

void CMapOverview::SetObjectIcon( int objectID, const char *icon, float size )
{
	MapObject_t* obj = FindObjectByID( objectID );

	if ( !obj )
		return;

	obj->icon = AddIconTexture( icon );
	obj->size = size;
}

void CMapOverview::SetObjectPosition( int objectID, const Vector &position, const QAngle &angle )
{
	MapObject_t* obj = FindObjectByID( objectID );

	if ( !obj )
		return;

	obj->angle = angle;
	obj->position = position;
}

void CMapOverview::RemoveObject( int objectID )
{
	for ( int i = 0; i < m_Objects.Count(); i++ )
	{
		if ( m_Objects[i].objectID == objectID )
		{
			m_Objects.Remove( i );
			return;
		}
	}
}

void CMapOverview::UpdateObjects()
{
	for ( int i = 0; i < m_Objects.Count(); i++ )
	{
		MapObject_t *obj = &m_Objects[i];

		if ( obj->endtime > 0 && obj->endtime < gpGlobals->curtime )
		{
			m_Objects.Remove( i );
			i--;
			continue;
		}
		
		if ( obj->index <= 0 )
			continue;

		C_BaseEntity *entity = ClientEntityList().GetEnt( obj->index );

		if ( !entity )
			continue;

		if(obj->update)
		{
			obj->position = entity->GetAbsOrigin();
			obj->angle = entity->GetAbsAngles();
		}
	}
}