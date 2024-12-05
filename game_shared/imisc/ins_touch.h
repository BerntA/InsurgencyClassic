//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef INS_TOUCH_H
#define INS_TOUCH_H
#ifdef _WIN32
#pragma once
#endif

//=========================================================
//=========================================================
#ifdef GAME_DLL

class CINSPlayer;

#endif

//=========================================================
//=========================================================
class CINSTouch : public CBaseEntity
{
public:
	DECLARE_CLASS( CINSTouch, CBaseEntity );

#ifdef GAME_DLL

protected:
	typedef void ( CINSTouch::*INSTouchCall_t )( CINSPlayer* );

public:
	virtual const char *GetTitle( void ) const { return NULL; }

protected:
	virtual void Spawn( void );

	virtual bool HideBrush( void ) const;
	virtual bool TouchSetupSpawn( void ) const { return true; }
	virtual bool NotSolidSpawn( void ) const { return true; }

	void MakeNotSolid( void );

	void EnableTouch( void );
	void DisableTouch( void );

	virtual void TouchHook( CBaseEntity *pOther, INSTouchCall_t TouchCall );

	virtual void PlayerStartTouch( CINSPlayer *pPlayer ) = 0;
	virtual void PlayerEndTouch( CINSPlayer *pPlayer ) = 0;

private:
	void StartTouch( CBaseEntity *pOther );
	void EndTouch( CBaseEntity *pOther );

#endif
};

#endif // INS_TOUCH_H
