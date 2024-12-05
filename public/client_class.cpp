//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//

#include "client_class.h"
#include "convar.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ClientClass *g_pClientClassHead=0;

// deathz0rz [

//=========================================================
//=========================================================
#ifdef TESTING

CON_COMMAND( cl_dumpclass, "dump client classes" )
{
	ClientClass *pCur = g_pClientClassHead;

	while( pCur )
	{
		Msg( "%s (%d)\n", pCur->m_pNetworkName, pCur->m_pRecvTable->GetNumProps( ) );
		pCur = pCur->m_pNext;
	}
}

#endif

// deathz0rz ]