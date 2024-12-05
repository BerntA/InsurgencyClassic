//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#include "stdio.h"

#include "statsman.h"
#include "statsman_shared.h"
#include "ins_stats_shared.h"

#include "md5.h"

#include <string>
#include <vector>

using namespace std;
using namespace ZThread;

//=========================================================
//=========================================================
void CStatsMan::Init( CINSStats *pStats )
{
	m_pStats = pStats;
}