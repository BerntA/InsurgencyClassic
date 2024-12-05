//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#ifndef CLIENT_H
#define CLIENT_H

extern void ClientActive( edict_t *pEdict, bool bLoadGame );
extern void ClientPutInServer( edict_t *pEdict, const char *playername );
extern void ClientCommand( CBasePlayer *player );
extern void ClientPrecache( void );
// Game specific precaches
extern void ClientGamePrecache( void );
//extern const char *GetGameDescription( void );
// Pongles [
//void ClientKill( edict_t *pEdict );
void Host_Say(CBasePlayer *pPlayer);
// Pongles ]

class CUserCmd;


#endif		// CLIENT_H
