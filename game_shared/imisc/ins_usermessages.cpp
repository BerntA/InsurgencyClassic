//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "usermessages.h"
#include "voice_gamemgr.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
void RegisterUserMessages( void )
{
	usermessages->Register( "SayText", -1 );	
	usermessages->Register( "TextMsg", -1 );
	usermessages->Register( "TextMsgFast", -1 );
	usermessages->Register( "HudMsg", -1 );
	usermessages->Register( "ShowMenu", -1 );
	usermessages->Register( "ResetHUD", 1 );	// called every respawn
	usermessages->Register( "ItemPickup", -1 );	// for item history on screen
	usermessages->Register( "Shake", 13 );		// shake view
	usermessages->Register( "Fade", 10 );	// fade HUD in/out
	usermessages->Register( "VGUIMenu", -1 );	// Show VGUI menu
	usermessages->Register( "CloseCaption", 3 ); // Show a caption (by string id number)(duration in 10th of a second)

	usermessages->Register( "SendAudio", -1 );	// play radion command

	usermessages->Register( "VoiceMask", VOICE_MAX_PLAYERS_DW*4 * 2 + 1 );
	usermessages->Register( "RequestState", 0 );

	usermessages->Register( "Damage", -1 );		// for HUD damage indicators
	usermessages->Register( "HintText", -1 );	// Displays hint text display
	
	usermessages->Register("FireMode", 1);
	usermessages->Register("FFMsg", 2);
	usermessages->Register("DeathInfo", -1);
	usermessages->Register("AngleHack", -1);
	usermessages->Register("CameraMode", -1);
	usermessages->Register("Pain", 1);
	usermessages->Register("VoiceCmd", -1 );
	usermessages->Register("VInventory", -1);
	usermessages->Register("UnitOrder", -1);
	usermessages->Register("ObjOrder", 1);
	usermessages->Register("VGUIHide", 0);

	// Used to send a sample HUD message
	usermessages->Register( "GameMessage", -1 );

	usermessages->Register( "ObjMsg", 3 );

	usermessages->Register("PlayerInfo", -1);
	usermessages->Register("PlayerLogin", 1);

    usermessages->Register( "ShowHint", 1 );

	usermessages->Register( "ReinforceMsg", 1);
	usermessages->Register( "PlayerStatus", 2 );

	usermessages->Register("MoraleNotice", -1);
}
