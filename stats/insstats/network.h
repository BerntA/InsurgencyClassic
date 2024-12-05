//========= Copyright © 2006 - 2008, James Mansfield, All rights reserved. ============//
//
// $NoKeywords: $
//=============================================================================//

#ifndef NETWORKMGR_H
#define NETWORKMGR_H
#ifdef _WIN32
#pragma once
#endif

#include "socketw.h"

//=========================================================
//=========================================================
class CNetworkManager
{
public:
	CNetworkManager( );

	bool Init( void );

	void Execute( void );

	void AddConnection( void );
	void RemoveConnection( void );

private:
	SWInetSocket m_Listener;

	int m_iConnections;
};

//=========================================================
//=========================================================
extern CNetworkManager *NetworkManager( void );

#endif // NETWORKMGR_H