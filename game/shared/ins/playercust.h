/*	deathz0rz

	== PLAYER CUSTOMIZATION ==

	You can customize your player model
	by selecting different skins and
	submodels. Which skins and submodels
	you can select is specified on a
	per-playermodel basis.

	(..insert babble..)

	NOTE: (this is what this comment is actually about.
	But just a note seemed odd) (referenced in
	CINSPlayer::ClientCommand) the skin and body
	commands take their parameters as HEX values. this
	is because calculating the max length of a hex value
	is a lot easier than calculating that of a decimal
	value.

*/

#ifndef PLAYERCUST_H
#define PLAYERCUST_H

#ifdef _WIN32
#pragma once
#endif

#include <stringlookup.h>
#include <utlvector.h>

class CINSPlayer;

DECLARE_STRING_LOOKUP_CONSTANTS(int,CUSTOMIZE_FL)

#define CUSTOMIZE_FL_DEFAULT		0x0001
#define CUSTOMIZE_FL_HIDDEN			0x0002
#define CUSTOMIZE_FL_ITEMDEPENDENT	0x0004
#define CUSTOMIZE_FL_DISPLAYSKIN	0x0008

typedef struct skincustomization_s {
	int		iFlags;
	int		iSkin;
	char*	pszName;
	char*	pszDesc;
	char*	pszModel;
	bool	bLoaded;
	skincustomization_s();
} skincustomization_t;

typedef struct bodygroupmodelcustomization_s {
	int		iFlags;
	int		iGroupValue;
	int		iGroup;
	char*	pszName;
	char*	pszDesc;
	char*	pszItemcode;
	bool	bLoaded;
	bodygroupmodelcustomization_s();
} bodygroupmodelcustomization_t;

typedef struct bodygroupcustomization_s {
	int		iFlags;
	int		iGroup;
	char*	pszName;
	char*	pszAttachment;
	char*	pszItempos;
	int		iItempos;
	CUtlVector<bodygroupmodelcustomization_t> aSubmodels;
	bool	bLoaded;
	bodygroupcustomization_s();
} bodygroupcustomization_t;

typedef struct modelcustomization_s {
	bool	bLoaded;
	int		nModelIndex;
	char*	pszVersion;
	char*	pszModelFile;
	char*	pszHeadAttachment;
	char*	pszHeadName;
	CUtlVector<skincustomization_t> aSkins;
	CUtlVector<bodygroupcustomization_t> aBodygroups;
	int		iMaxSubmodels;
} modelcustomization_t;

typedef struct playercustomization_s {
	//empty
} playercustomization_t;

modelcustomization_t& GetModelCustomization(CINSPlayer *pPlayer);
void SetupModelCustomization(const char* pszModel, modelcustomization_t& MdlCust);
#ifdef CLIENT_DLL
void SaveModelCustomization(modelcustomization_t& MdlCust, bool bSendToServer, int iPlayerBody, int iPlayerSkin, int iWeapon);
void LoadModelCustomization(modelcustomization_t& MdlCust, bool bSendToServer, int& iPlayerBody, int& iPlayerSkin, int &iWeapon);
#endif // CLIENT_DLL

#endif //PLAYERCUST_H