//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef OBJ_SHARED_H
#define OBJ_SHARED_H
#ifdef _WIN32
#pragma once
#endif

//=========================================================
//=========================================================
#ifdef GAME_DLL

#include "ins_obj.h"

#else

class C_INSObjective;

#define CINSObjective C_INSObjective
#include "c_ins_obj.h"

#endif

//=========================================================
//=========================================================
enum ObjectiveMessages_t
{
	OBJ_CAPTURE_START = 1,				// "The Bridge is being Captured by the USMC"
	//OBJ_CAPTURE_START_PERSONAL,			// "You are Capturing the Bridge"
	//OBJ_NOT_ENOUGH_PLAYERS,			// "Not Enough Players to Capture" (Send to Only Players Inside Objective)
	//OBJ_CAPTURE_UNSUCCESSFUL,			// "The Bridge has Stopped being Captured by the USMC"
	//OBJ_OUTSIDE,						// "Warning: Get Back Inside!!" (Send to the Player Who Moved Outside)
	//OBJ_NOT_OUTSIDE,					// "You have Sucessfully Re-Enetered the Objective"
	OBJ_CAPTURED,						// "The Bridge has been Captured by the USMC"
	//OBJ_NO_RECAPTURE					// "The Bridge Cannot be Re-Captured" (Send to Only Players Inside Objective)
};

//=========================================================
//=========================================================
#define OBJ_CAPTUREINVALID -1
#define OBJ_PROGRESS_MAX 50

#endif // OBJ_SHARED_H
