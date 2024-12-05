//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "kbutton.h"
#include "input.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
class CINSInput : public CInput
{
public:
};

//=========================================================
//=========================================================
static CINSInput g_Input;

//=========================================================
//=========================================================

// expose this interface
IInput *input = ( IInput * )&g_Input;

