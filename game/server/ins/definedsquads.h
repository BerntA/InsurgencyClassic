//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
//=============================================================================//

#ifndef DEFINEDSQUADS_H
#define DEFINEDSQUADS_H
#ifdef _WIN32
#pragma once
#endif

#include "imc_format.h"

extern bool LoadDefinedSquad(const char *pszFileName, int iTeamID, char *pszName, SlotData_t &Data);
extern bool LoadDefinedSquadData(KeyValues *pSquad, int iTeamID, char *pszSquadName, SlotData_t &Data);

#endif // DEFINEDSQUADS_H
