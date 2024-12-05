//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: MiniMap.h: interface for the CMiniMap class.
//
// $NoKeywords: $
//=============================================================================//

#if !defined HLTVPANEL_H
#define HLTVPANEL_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/panel.h>
#include <cl_dll/iviewport.h>
#include <vector.h>
#include <igameevents.h>
#include <shareddefs.h>
#include <const.h>


#define MAX_TRAIL_LENGTH	30
#define OVERVIEW_MAP_SIZE	1024	// an overview map is 1024x1024 pixels

class CMapOverview : public vgui::Panel, public IGameEventListener2
{
	DECLARE_CLASS_SIMPLE(CMapOverview, vgui::Panel);

public:	

	enum
	{
		MAP_MODE_INSET = 0,
		MAP_MODE_FULL
	};

	CMapOverview( vgui::Panel *pParent );
	virtual ~CMapOverview();

protected:	// private structures & types
	
	// list of game events the hLTV takes care of
	typedef struct MapPlayer_s {
		int		index;		// player's index
		int		userid;		// user ID on server
		//int	icon;		// players texture icon ID
		Color   color;		// players team color
		char	name[MAX_PLAYER_NAME_LENGTH];
		int		team;		// T1,T2
		int		squad;		// Squad
		bool	alive;		// alive
		Vector	position;	// current x,y pos
		QAngle	angle;		// view origin 0..360
		Vector2D trail[MAX_TRAIL_LENGTH];	// save 1 footstep each second for 1 minute
	} MapPlayer_t;

	typedef struct MapObject_s {
		int		objectID;	// unique object ID
		int		index;		// entity index if any
		int		icon;		// players texture icon ID
		Color   color;		// players team color
		char	name[MAX_PLAYER_NAME_LENGTH];	// show text under icon
		Vector	position;	// current x,y pos
		QAngle	angle;		// view origin 0..360
		float	endtime;	// time stop showing object
		float	size;		// object size
		float	status;		// green status bar [0..1], -1 = disabled
		bool	update;		// update pos
	} MapObject_t;

public: // IViewPortPanel interface:

	void Reset();
	virtual void Think();

public: // IGameEventListener

	virtual void FireGameEvent( IGameEvent *event);
	
public:	// VGUI overrides

	virtual void Paint();
	virtual void OnMousePressed( vgui::MouseCode code );
	virtual void ApplySchemeSettings(vgui::IScheme *scheme);
	
public:

	virtual float GetZoom( void );
	virtual int GetMode( void );
	
	// Player settings:
	virtual void ShowPlayerNames(bool state);
	virtual void ShowPlayerHealth(bool state);
	virtual void ShowPlayerTracks(float seconds); 
	virtual void SetPlayerPositions(int index, const Vector &position, const QAngle &angle);

	// general settings:
	virtual void UpdateMap(void);
	virtual void SetTime( float time );
	virtual void SetMode( int mode );
	virtual void SetZoom( float zoom, float time = 0.0f );
	virtual void SetFollowAngle(bool state);
	virtual void SetFollowEntity(int entindex); // 0 = off
	virtual void SetCenter( const Vector2D &mappos); 
	virtual void SetAngle( float angle);
	virtual Vector2D WorldToMap( const Vector &worldpos );

	// Object settings
	// Pongles [
	virtual int		AddObject( const char *icon, int entity, float timeToLive, bool update = true ); // returns object ID, 0 = no entity, -1 = forever
	// Pongles ]
	virtual void	SetObjectIcon( int objectID, const char *icon, float size );  // icon world size
	virtual void	SetObjectText( int objectID, const char *text, Color color ); // text under icon
	virtual void	SetObjectStatus( int objectID, float value ); // status bar under icon
	virtual void	SetObjectPosition( int objectID, const Vector &position, const QAngle &angle ); // world pos/angles
	virtual void	RemoveObject( int objectID ); 
	
	// rules that define if you can see a player on the overview or not
	virtual bool CanPlayerBeSeen(MapPlayer_t *player);

	/// allows mods to restrict names (e.g. CS when mp_playerid is non-zero)
	virtual bool CanPlayerNameBeSeen(MapPlayer_t *player);

protected:
	
	virtual void	DrawCamera();
	virtual void	DrawObjects();
	virtual void	DrawMapTexture();
	virtual void	DrawMapPlayers();
	virtual void	DrawMapPlayerTrails();
	virtual void	UpdatePlayerTrails();
	virtual void	DrawMapObjectives();
	virtual void	ResetRound();
	virtual void	InitTeamColorsAndIcons();

	bool			IsInPanel(Vector2D &pos);
	MapPlayer_t*	GetPlayerByUserID( int userID );
	int				AddIconTexture(const char *filename);
	Vector2D		MapToPanel( const Vector2D &mappos );
	int				GetPixelOffset( float height );
	void			UpdateZoom();
	void			UpdateFollowEntity();
	void			UpdatePlayers();
	void			UpdateObjects(); // objects bound to entities 
	MapObject_t*	FindObjectByID(int objectID);

	bool			DrawIcon(	int textureID,
								Vector pos,
								float scale,
								float angle,
								const Color *iconColor = NULL,
								const char *text = NULL,
								const Color *textColor = NULL );

	// Pongles [
	bool			m_bMapUpdate;
	// Pongles ]
	
	int				m_nMode;
	Vector2D		m_vPosition;
	Vector2D		m_vSize;
	float			m_flChangeSpeed;
	float			m_flIconSize;


	MapPlayer_t		m_Players[MAX_PLAYERS];
	CUtlDict< int, int> m_TextureIDs;
	CUtlVector<MapObject_t>	m_Objects;

	int		m_iTeamIcon;
	int		m_iObjectiveIcon;
	int		m_ObjectIcons[64];
	int		m_ObjectCounterID;
	vgui::HFont	m_hIconFont;

	bool m_bShowNames;
	bool m_bShowTrails;
	bool m_bShowHealth;
		
	int	 m_nMapTextureID;		// texture id for current overview image
	
	KeyValues * m_MapKeyValues; // keyvalues describing overview parameters

	Vector	m_MapOrigin;	// read from KeyValues files
	float	m_fMapScale;	// origin and scale used when screenshot was made
	bool	m_bRotateMap;	// if true roatate map around 90 degress, so it fits better to 4:3 screen ratio

	int		m_nFollowEntity;// entity number to follow, 0 = off
	float	m_fZoom;		// current zoom n = overview panel shows 1/n^2 of whole map 
	float	m_fTragetZoom;	// target value we are zooming to
	float	m_fZoomSpeed;	// zoom change per second
	float	m_fFullZoom;	// best zoom factor for full map view (1.0 is map is a square) 
	Vector2D m_ViewOrigin;	// map coordinates that are in the center of the pverview panel
	Vector2D m_MapCenter;	// map coordinates that are in the center of the pverview panel

	float	m_fNextUpdateTime;
	float	m_fViewAngle;	// rototaion of overview map
	float	m_fWorldTime;	// current world time
	float   m_fNextTrailUpdate; // next time to update player trails
	float	m_fTrailUpdateInterval; // if -1 don't show trails
	bool	m_bFollowAngle;	// if true, map rotates with view angle
	
};

#endif //
