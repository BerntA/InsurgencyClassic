//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: INS specific input handling
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "kbutton.h"
#include "input.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CINSInput : public CInput
{
public:
};

static CINSInput g_Input;

IInput *input = ( IInput * )&g_Input;