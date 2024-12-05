//========= Copyright © 2005, James "Pongles" Mansfield, All Rights Reserved. ============//
//
// Purpose: Handles all the MySQL Stuff
//
// $NoKeywords: $
//=============================================================================//

#include <stdio.h>

#include "insstats.h"
#include "md5.h"

#include "playerstats.h"


















//=========================================================
//=========================================================
//#define USE_IPB_2X

//=========================================================
//=========================================================
int ServerAuthMySQL(const char *pszUsername, const char *pszPassword, bool &bValid)
{
	// select DB
	if(mysql_select_db(g_pMySQLConnection, MYSQL_DB_GLOBAL))
		return SERVERAUTH_ERROR;
		
	// setup query
	char szString[512];
	sprintf(szString, "SELECT password FROM %s WHERE name='%s' LIMIT 0, 1",
		MYSQL_GLOBAL_SERVERS, pszUsername);

	// query
	int iResult = mysql_real_query(g_pMySQLConnection, szString, strlen(szString));

	if(iResult)
	{
	#ifdef WIN32
		MySQLError();
	#endif
		
		return SERVERAUTH_ERROR;
	}

	// grab the data
	MYSQL_RES *pResource;
	MYSQL_ROW ppRow;
	MYSQL_FIELD *pField;

	const char *pszReturnedPassword = NULL;
	int iMemberGroup = 1;

	pResource = mysql_store_result(g_pMySQLConnection);
	ppRow = mysql_fetch_row(pResource);

	if(!ppRow)
		return SERVERAUTH_INVALID;

	mysql_field_seek(pResource, 0);

	// find the data
	for(int i = 0; i < (int)mysql_num_fields(pResource); i++)
	{
		pField = mysql_fetch_field(pResource);
		const char *pszFieldName = pField->name;

		if(strcmp(pszFieldName, "password") == 0)
			pszReturnedPassword = ppRow[i];
	}

	// make sure the passwords are the same
	if(strcmp(pszReturnedPassword, pszPassword) != 0)
		return SERVERAUTH_INVALID;
	else
		bValid = true;

	return SERVERAUTH_VALID;
}

//=========================================================
//=========================================================
void CreateServerDBName(string &strName)
{
	strName.insert(0, MYSQL_DB_SERVERPREFIX);
}



//=========================================================
//=========================================================
bool PlayerStatsFromMySQL(int iMemberID, const char *pszDatabase, PlayerStats_t &PlayerStats, bool bGetWeaponStats, int iGetType)
{
	PlayerStats.Reset();

#ifdef _DEBUG
	if(iMemberID == 0)
		CreateStatsPlayerMySQL(iMemberID, pszDatabase);
#endif

	if(mysql_select_db(g_pMySQLConnection, pszDatabase))
		return false;

	char szString[512];

	// query
	const char *pszGetRows = NULL;

	if(iGetType == PLAYERSTATS_INVALID)
		pszGetRows = "*";
	else
		pszGetRows = PlayerStats.GetTypeName(iGetType);

	sprintf(szString, "SELECT * FROM %s WHERE member_id='%i' LIMIT 0,1", MYSQL_STATS, iMemberID);
	int iResult = mysql_real_query(g_pMySQLConnection, szString, strlen(szString));

	if(iResult)
	{
	#ifdef WIN32
		MySQLError();
	#endif

		return false;
	}

	// store away
	MYSQL_RES *pResource;
	MYSQL_ROW ppRow;
	MYSQL_FIELD *pField;

	pResource = mysql_store_result(g_pMySQLConnection);
	ppRow = mysql_fetch_row(pResource);

	if(!ppRow)
		return false;

	// handle data
	for(int i = 0; i < (int)mysql_num_fields(pResource); i++)
	{
		pField = mysql_fetch_field(pResource);

		const char *pszFieldName = pField->name;

		int iType = PlayerStats.GetTypeFromName(pszFieldName);

		if(iType == INVALID_STATSTYPE)
			continue;

		PlayerStats.Increment(iType, atoi(ppRow[i]));
	}

	// get the weapon stats if needed
	if(!WeaponStatsFromMySQL(iMemberID, pszDatabase, PlayerStats))
		return false;

	return true;
}

//=========================================================
//=========================================================
bool WeaponStatsFromMySQL(int iMemberID, const char *pszDatabase, PlayerStats_t &PlayerStats)
{
	if(mysql_select_db(g_pMySQLConnection, pszDatabase))
		return false;

	char szString[512];

	// query
	sprintf(szString, "SELECT * FROM %s WHERE member_id='%i'", MYSQL_STATS, iMemberID);
	int iResult = mysql_real_query(g_pMySQLConnection, szString, strlen(szString));

	if(iResult)
	{
	#ifdef WIN32
		MySQLError();
	#endif
		return false;
	}

	// store away
	MYSQL_RES *pResource;
	MYSQL_ROW ppRow;
	MYSQL_FIELD *pField;
	
	pResource = mysql_store_result(g_pMySQLConnection);

	mysql_field_seek(pResource, 0);

	// handle data
	WeaponStats_t WeaponStats;

	while((ppRow = mysql_fetch_row(pResource)) != NULL)
	{
		int iWeaponID = INVALID_WEAPON;
		WeaponStats.Reset();

		for(int i = 0; i < (int)mysql_num_fields(pResource); i++)
		{
			pField = mysql_fetch_field(pResource);

			const char *pszFieldName = pField->name;
			int iValue = atoi(ppRow[i]);

			if(strcmp(pszFieldName, "weapon_id") == 0)
				iWeaponID = iValue;

			int iType = WeaponStats.GetTypeFromName(pszFieldName);

			if(iType == INVALID_WEAPON)
				continue;

			WeaponStats.Increment(iType, iValue);
		}

		if(iWeaponID == INVALID_WEAPON)
			continue;

		PlayerStats.CombineWeaponStats(iWeaponID, WeaponStats);
	}

	return true;
}

//=========================================================
//=========================================================
bool PlayerUpdateValidMySQL(int iMemberID, const string &strServerDB)
{
	if(mysql_select_db(g_pMySQLConnection, MYSQL_DB_GLOBAL))
		return false;

	char szString[512];

	// query
	sprintf(szString, "SELECT last_server FROM %s WHERE member_id='%i' LIMIT 0,1", MYSQL_STATS, iMemberID);
	int iResult = mysql_real_query(g_pMySQLConnection, szString, strlen(szString));

	if(iResult)
	{
	#ifdef WIN32
		MySQLError();
	#endif

		return false;
	}

	// grab the data
	MYSQL_RES *pResource;
	MYSQL_ROW ppRow;
	MYSQL_FIELD *pField;

	const char *pszLastServer = NULL;

	pResource = mysql_store_result(g_pMySQLConnection);
	ppRow = mysql_fetch_row(pResource);

	if(!ppRow)
		return false;

	mysql_field_seek(pResource, 0);

	// find the data
	for(int i = 0; i < (int)mysql_num_fields(pResource); i++)
	{
		pField = mysql_fetch_field(pResource);
		const char *pszFieldName = pField->name;

		if(strcmp(pszFieldName, "last_server") == 0)
			pszLastServer = ppRow[i];
	}

	// now ensure the server name is equal to the last server
	return strcmp(pszLastServer, strServerDB.c_str());
}

//=========================================================
//=========================================================
void CreateSetQueryFromStats(const IInfoStats_t &InfoStats, string &strSetQuery)
{
	char szTempBuffer[64];

	for(int i = 0; i < InfoStats.CountTypes(); i++)
	{
		sprintf(szTempBuffer, "%s='%i'", InfoStats.GetNameFromType(i), InfoStats.GetValue(i));

		strSetQuery += szTempBuffer;

		if(i != (InfoStats.CountTypes() - 1))
			strSetQuery += ",";
	}
}




//=========================================================
//=========================================================
bool UpdateStatsMySQL(int iMemberID, string &strServer, const PlayerStats_t &PlayerStats)
{

}

//=========================================================
//=========================================================
int PlayerAvatarMySQL(int iMemberID, SACAvatar_t &Avatar, char *pszAvatar)
{
	// send avatar info
	if(mysql_select_db(g_pMySQLConnection, MYSQL_DB_FORUMS))
		return PLAYERAVATAR_ERROR;
		
	char szString[512];
	sprintf(szString, "SELECT avatar,avatar_size FROM %s WHERE member_id='%i' LIMIT 0, 1",
		MYSQL_FORUMS_MEMBERS, iMemberID);

	int iResult = mysql_real_query(g_pMySQLConnection, szString, strlen(szString));

	if(iResult)
	{
	#ifdef WIN32
		MySQLError();
	#endif

		return PLAYERAVATAR_ERROR;
	}

	MYSQL_RES *pResource;
	MYSQL_ROW ppRow;
	MYSQL_FIELD *pField;

	pResource = mysql_store_result(g_pMySQLConnection);
	ppRow = mysql_fetch_row(pResource);

	if(!ppRow)
		return PLAYERAVATAR_ERROR;

	mysql_field_seek(pResource, 0);

	char szAvatar[SAC_MAX_AVATAR_LENGTH];
	char szAvatarSize[64];

	szAvatar[0] = '\0';
	szAvatarSize[0] = '\0';

	for(int i = 0; i < (int)mysql_num_fields(pResource); i++)
	{
		pField = mysql_fetch_field(pResource);
		const char *pszFieldName = pField->name;

		if(ppRow[i] == NULL)
			continue;

		if(strcmp(pszFieldName, "avatar") == 0)
			strncpy(szAvatar, ppRow[i], sizeof(szAvatar));
		else if(strcmp(pszFieldName, "avatar_size") == 0)
			strncpy(szAvatarSize, ppRow[i], sizeof(szAvatarSize));
	}

	// send it off
	bool bValidAvatar = szAvatar[0];

	if(!bValidAvatar)
		return PLAYERAVATAR_NONE;

	strncpy(pszAvatar, szAvatar, SAC_MAX_AVATAR_LENGTH);

	Avatar.m_iURLSize = strlen(szAvatar);
	Avatar.m_iWidth = Avatar.m_iHeight = 0;

	if(szAvatarSize[0])
	{
		char *pszElement = NULL;

		Mutex m;
		m.acquire();

		strtok(pszElement, "x");

		for(int i = 0; i < 2; i++)
		{
			((i == 0) ? Avatar.m_iWidth : Avatar.m_iHeight) = atoi(pszElement);
			strtok(NULL, "x");
		}

		m.release();
	}

	return PLAYERAVATAR_VALID;
}