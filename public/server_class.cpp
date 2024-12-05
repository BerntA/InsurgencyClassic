//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//

#include "server_class.h"
#include "convar.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ServerClass *g_pServerClassHead=0;

// deathz0rz [

//=========================================================
//=========================================================
#ifdef TESTING

CON_COMMAND( sv_dumpclass, "dump server classes" )
{
	ServerClass *pCur = g_pServerClassHead;

	while( pCur )
	{
		Msg( "%s (%d)\n", pCur->m_pNetworkName, pCur->m_pTable->GetNumProps( ) );
		pCur = pCur->m_pNext;
	}
}

#endif

// deathz0rz ]