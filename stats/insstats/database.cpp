

//=========================================================
//=========================================================
void MySQLError( void )
{
	LogPrefix(NULL);

	printf("%s\n",
		mysql_error(g_pMySQLConnection));
}

//=========================================================
//=========================================================
#define MYSQL_DB_FORUMS					"forums"
#define MYSQL_FORUMS_MEMBERS			"ibf_members"
#define MYSQL_FORUMS_MEMBERS_CONVERGE	"ibf_members_converge"

#define MYSQL_DB_STATSPREFIX			"stats_"

#define MYSQL_DB_GLOBAL					"global"
#define MYSQL_GLOBAL_SERVERS			"servers"

#define MYSQL_DB_SERVERPREFIX			"server_"

#define MYSQL_STATS_PLAYER				"stats_player"
#define MYSQL_STATS_WEAPON				"stats_weapon"

//=========================================================
//=========================================================
CDatabaseManagement::CDatabaseManagement( )
{
	m_pMySQLConnection = NULL;
}

//=========================================================
//=========================================================
bool CDatabaseManagement::Init( void )
{
	m_pMySQLConnection = mysql_init( NULL );

	char szConnectString[ 128 ];
	sprintf( szConnectString, "Connecting to MySQL (%s:%i) ...\n", ServerConfig.m_pszMySQLHostname, ServerConfig.m_iPort );

	StatsServer( )->Log( szConnectString );

	if( !mysql_real_connect( g_pMySQLConnection, ServerConfig.m_pszMySQLHostname, ServerConfig.pszMySQLUser, ServerConfig.pszMySQLPass, NULL, ServerConfig.m_iPort, NULL, 0 ) )
	{
		printf( "%s\n", mysql_error( g_pMySQLConnection ) );
		return false;
	}

	return true;
}

//=========================================================
//=========================================================
int CDatabaseManagement::ServerAuth( const char *pszUsername, const char *pszPassword )
{
	// sanity check
	if( pszUsername && *pszUsername != '\0 && pszPassword && *pszPassword != '\0' )
		return SERVERAUTH_ERROR;

	// select DB
	if( mysql_select_db( m_pMySQLConnection, MYSQL_DB_GLOBAL ) )
		return SERVERAUTH_ERROR;
		
	// setup query
	char szString[ 512 ];
	sprintf( szString, "SELECT password FROM %s WHERE name='%s' LIMIT 0, 1",
		MYSQL_GLOBAL_SERVERS, pszUsername );

	// query
	int iMySQLResult = mysql_real_query( m_pMySQLConnection, szString, strlen( szString ) );

	if( iMySQLResult != 0 )
	{
	#ifdef WIN32

		MySQLError( );

	#endif
		
		return SERVERAUTH_ERROR;
	}

	// grab the data
	MYSQL_RES *pResource;
	MYSQL_ROW ppRow;
	MYSQL_FIELD *pField;

	const char *pszReturnedPassword = NULL;
	int iMemberGroup = 1;

	pResource = mysql_store_result( m_pMySQLConnection );
	ppRow = mysql_fetch_row( pResource );

	if( !ppRow )
		return SERVERAUTH_INVALID;

	mysql_field_seek( pResource, 0 );

	// find the data
	for( int i = 0; i < ( int )mysql_num_fields( pResource ); i++ )
	{
		pField = mysql_fetch_field( pResource );

		const char *pszFieldName = pField->name;

		if( pszFieldName && strcmp( pszFieldName, "password" ) == 0 )
			pszReturnedPassword = ppRow[ i ];
	}

	// make sure the passwords are the same
	return ( ( strcmp( pszReturnedPassword, pszPassword ) == 0 ) ? SERVERAUTH_VALID : SERVERAUTH_INVALID );
}

//=========================================================
//=========================================================
int CDatabaseManager::PlayerAuth( const char *pszUsername, const char *pszPassword, const char *pszServerName, int &iPlayerType, int &iMemberID )
{
	// select DB
	if( mysql_select_db( m_pMySQLConnection, MYSQL_DB_FORUMS ) )
		return PLAYERAUTH_ERROR;

	// setup defaults
	iPlayerType = SAC_PLAYERTYPE_INVALID;
	iMemberID = 0;

	// setup data
	char szString[ 512 ];
	int iResult;

	const char *pszReturnedPassword = NULL;
	int iMemberGroup = 1;

	// setup query
	sprintf( szString, "SELECT id, mgroup FROM %s WHERE name='%s' LIMIT 0, 1", 
		MYSQL_FORUMS_MEMBERS, pszUsername );

	// grab the data
	MYSQL_RES *pResource;
	MYSQL_ROW ppRow;
	MYSQL_FIELD *pField;

	pResource = mysql_store_result( m_pMySQLConnection );
	ppRow = mysql_fetch_row( pResource );

	if( !ppRow )
		return PLAYERAUTH_INVALID;

	mysql_field_seek( pResource, 0 );

	// find the data
	for( int i = 0; i < ( int )mysql_num_fields( pResource ); i++ )
	{
		pField = mysql_fetch_field( pResource );

		const char *pszFieldName = pField->name;

		if( !pszFieldName )
			continue;

		if( strcmp( pszFieldName, "id" ) == 0 )
			iMemberID = atoi( ppRow[ i ] );
		else if( strcmp( pszFieldName, "mgroup" ) == 0 )
			iMemberGroup = atoi( ppRow[ i ] );
	}

	// setup query
	sprintf( szString, "SELECT converge_pass_hash, converge_pass_salt FROM %s WHERE id='%i' LIMIT 0, 1",
		MYSQL_FORUMS_MEMBERS_CONVERGE, iMemberID );

	pResource = mysql_store_result( m_pMySQLConnection );
	ppRow = mysql_fetch_row( pResource );

	if( !ppRow )
		return PLAYERAUTH_INVALID;

	mysql_field_seek( pResource, 0 );

	// find the data
	const char *pszConvergePassword, pszConvergePasswordSalt;
	pszConvergePassword = pszConvergePasswordSalt = NULL;

	for( int i = 0; i < ( int )mysql_num_fields( pResource ); i++ )
	{
		pField = mysql_fetch_field( pResource );

		const char *pszFieldName = pField->name;

		if( !pszFieldName )
			return;

		if( strcmp( pszFieldName, "converge_pass_hash" ) == 0 )
			pszConvergePassword = ppRow[ i ];
		else( strcmp( pszFieldName, "converge_pass_salt" ) == 0 )
			pszConvergePasswordSalt = ppRow[ i ];
	}

	// ensure they're not empty
	if( !pszConvergePassword || *pszConvergePassword == '\0' || !pszConvergePasswordSalt || *pszConvergePasswordSalt == '\0' )
		return PLAYERAUTH_INVALID;

	// do the salted stuff
	MD5Data_t SaltedMD5Password, SaltedMD5;
	HashPassword( pszConvergePasswordSalt, &SaltedMD5 );

	string JoinedPassword = SaltedMD5 + pszConvergePassword;
	HashPassword( JoinedPassword.c_str( ), &SaltedMD5Password );

	pszReturnedPassword = SaltedMD5Password;

	if( *pszReturnedPassword == '\0' )
		return PLAYERAUTH_INVALID;

	// make sure the passwords are the same
	if( strcmp( pszReturnedPassword, strPassword.c_str( ) ) != 0 )
		return PLAYERAUTH_INVALID;

	// ensure he's not banned
	const ServerConfig_t &ServerConfig = StatsServer( )->ServerConfig( );

	UserGroup_t BannedGroup = ServerConfig.m_UserGroups[ USERGRP_BANNED ];

	for( int i = 0; i < BannedGroup.size( ); i++ )
	{
		if( iMemberGroup == BannedGroup[ i ] )
		{
			iPlayerType = SAC_PLAYERTYPE_BANNED;

			return PLAYERAUTH_INVALID;
		}
	}

	// maybe praise the gods?
	bool bIsDeveloper = false;

	UserGroup_t DeveloperGroup = ServerConfig.m_UserGroups[ USERGRP_DEVELOPER ];

	for( int i = 0; i < DeveloperGroup.size( ); i++ )
	{
		if( iMemberGroup == DeveloperGroup[ i ] )
		{
			bIsDeveloper = true;
			break;
		}
	}

	iPlayerType = ( bIsDeveloper ? SAC_PLAYERTYPE_DEVELOPER : SAC_PLAYERTYPE_VALID );

	// create the player
	if( pszServerName )
	{
		if( !CreatePlayerStats( iMemberID, MYSQL_DB_GLOBAL ) )
			return PLAYERAUTH_ERROR;

		string strServerDBName;
		strServerDBName = *pServerName;
		CreateServerDBName(strServerDBName);

		if(!CreateStatsPlayerMySQL(iMemberID, strServerDBName.c_str()))
			return PLAYERAUTH_ERROR;

		if(mysql_select_db(g_pMySQLConnection, MYSQL_DB_GLOBAL))
			return false;

		sprintf(szString, "UPDATE %s SET last_server='%s' WHERE member_id='%i'",
			MYSQL_STATS, pServerName->c_str(), iMemberID);

		iResult = mysql_real_query(g_pMySQLConnection, szString, strlen(szString));

		if(iResult)
		{
		#ifdef WIN32
			MySQLError();
		#endif
			
			return PLAYERAUTH_ERROR;
		}
	}

	return PLAYERAUTH_VALID;
}

//=========================================================
//=========================================================
bool CDatabaseManager::CreatePlayerStats( int iMemberID, const char *pszDatabase )
{
	// do a sanity check
	if( !pszDatabase || *pszDatabase == '\0' )
		return false;

	// if the player already exists, don't bother
	if( mysql_select_db( m_pMySQLConnection, pszDatabase ) )
		return false;

	char szString[ 512 ];
	int iResult;
			
	sprintf( szString, "SELECT id FROM %s WHERE member_id='%i' LIMIT 0,1",
		MYSQL_STATS, iMemberID );

	iResult = mysql_real_query(g_pMySQLConnection, szString, strlen(szString));

	if( iResult != 0 )
	{
	#ifdef _DEBUG

		MySQLError( );

	#endif

		return false;
	}

	MYSQL_RES *pResource = mysql_store_result( m_pMySQLConnection );
	MYSQL_ROW ppRow = mysql_fetch_row( pResource );

	if( ppRow )
		return true;

	// create a new player!
	sprintf( szString, "INSERT %s SET member_id='%i'", MYSQL_STATS, iMemberID );

	iResult = mysql_real_query( m_pMySQLConnection, szString, strlen( szString ) );

	if( iResult != 0 )
	{
	#ifdef _DEBUG

		MySQLError( );

	#endif

		return false;
	}

	return true;
}

//=========================================================
//=========================================================
bool CDatabaseManager::CanPlayerUpdate( int iMemberID, const char *pszrServerName )
{
	if( mysql_select_db( m_pMySQLConnection, MYSQL_DB_GLOBAL ) )
		return false;

	char szString[ 512 ];

	// query
	sprintf( szString, "SELECT last_server FROM %s WHERE member_id='%i' LIMIT 0,1",
		MYSQL_STATS, iMemberID );

	int iResult = mysql_real_query( m_pMySQLConnection, szString, strlen( szString ) );

	if( iResult != 0 )
	{
	#ifdef _DEBUG

		MySQLError( );

	#endif

		return false;
	}

	// grab the data
	const char *pszLastServer = NULL;

	MYSQL_RES *pResource = mysql_store_result(g_pMySQLConnection);
	MYSQL_ROW ppRow = mysql_fetch_row(pResource);

	if(!ppRow)
		return false;

	mysql_field_seek( pResource, 0 );

	// find the data
	MYSQL_FIELD *pField;

	for( int i = 0; i < ( int )mysql_num_fields( pResource ); i++ )
	{
		pField = mysql_fetch_field( pResource );

		const char *pszFieldName = pField->name;

		if( pszFieldName && strcmp( pszFieldName, "last_server" ) == 0 )
			pszLastServer = ppRow[ i ];
	}

	// now ensure the server name is equal to the last server
	return ( strcmp( pszLastServer, pszrServerName ) == 0 );
}

//=========================================================
//=========================================================
bool CDatabaseManager::UpdatePlayerStats( int iMemberID, const char *pszServerName, const CPlayerStats &PlayerStats )
{
	string strDatabase = pszServerName;
	CreateDatabaseName( strDatabase );

	// create their entry in the server stats thing if needed
	if( !CreatePlayerStats( iMemberID, strDatabase.c_str( ) ) )
		return false;

	// now write it to both places!!
	WriteStatsMySQL( iMemberID, MYSQL_DB_GLOBAL, PlayerStats );
	WriteStatsMySQL( iMemberID, strServerDB.c_str(), PlayerStats );

	return true;
}

//=========================================================
//=========================================================
bool CDatabaseManager::WritePlayerStats( int iMemberID, const char *pszDatabase, const CPlayerStats &PlayerStats )
{
	if( mysql_select_db( m_pMySQLConnection, pszDatabase ) )
		return false;

	string strQuery;

	PlayerStats_t AggregatePlayerStats;

	if(!PlayerStatsFromMySQL(iMemberID, pszDatabase, AggregatePlayerStats, true))
		return false;

	AggregatePlayerStats.Combine( PlayerStats );

	// write player stats
	char szTempBuffer[128];
	sprintf(szTempBuffer, "UPDATE %s SET ", MYSQL_STATS);

	strQuery += szTempBuffer;
	CreateSetQueryFromStats(PlayerStats, strQuery);

	sprintf(szTempBuffer, " WHERE member_id='%i'", iMemberID);
	strQuery += szTempBuffer;
		
	int iResult = mysql_real_query(g_pMySQLConnection, strQuery.c_str(), strQuery.length());

	if(iResult)
	{
	#ifdef WIN32
		MySQLError();
	#endif

		return false;
	}

	// write weapon stats
	MYSQL_RES *pResource;
	MYSQL_ROW ppRow;

	for(int i = 0; i < MAX_WEAPONS; i++)
	{
		sprintf(szTempBuffer, "SELECT id FROM %s WHERE member_id='%i' AND weapon_id='%i' LIMIT 0,1",
			MYSQL_STATS_WEAPON, iMemberID, i);

		strQuery = szTempBuffer;

		iResult = mysql_real_query(g_pMySQLConnection, strQuery.c_str(), strQuery.length());

		if(iResult)
		{
		#ifdef WIN32
			MySQLError();
		#endif

			return false;
		}

		pResource = mysql_store_result(g_pMySQLConnection);
		ppRow = mysql_fetch_row(pResource);

		// create row or update, then construct
		if(!ppRow)
			strQuery = "INSERT";
		else
			strQuery = "UPDATE";

		sprintf(szTempBuffer, " %s SET ", MYSQL_STATS_WEAPON);
		strQuery += szTempBuffer;

		CreateSetQueryFromStats(PlayerStats.cGetWeaponStats(i), strQuery);

		if(!ppRow)
		{
			sprintf(szTempBuffer, ",weapon_id='%i'", i);
			strQuery += szTempBuffer;
		}
		else
		{
			sprintf(szTempBuffer, " WHERE member_id='%i' AND weapon_id='%i'", iMemberID, i);
			strQuery += szTempBuffer;
		}

		iResult = mysql_real_query(g_pMySQLConnection, strQuery.c_str(), strQuery.length());

		if(iResult)
		{
		#ifdef WIN32
			MySQLError();
		#endif

			return false;
		}
	}

	return true;
}

//=========================================================
//=========================================================
bool CDatabaseManager::ReadPlayerStats( int iMemberID, const char *pszDatabase, CPlayerStats &PlayerStats )
{
	if( mysql_select_db( m_pMySQLConnection, pszDatabase ) )
		return false;

	char szString[ 512 ];

	// read player stats
	sprintf( szString, "SELECT * FROM %s WHERE member_id='%i' LIMIT 0,1",
		MYSQL_STATS, iMemberID);

	int iResult = mysql_real_query( m_pMySQLConnection, szString, strlen( szString ) );

	if( iResult != 0 )
	{
	#ifdef _DEBUG

		MySQLError( );

	#endif

		return false;
	}

	MYSQL_RES *pResource = mysql_store_result(g_pMySQLConnection);
	ReadStats( pResource, PlayerStats );

	// read weapon stats
	sprintf( szString, "SELECT * FROM %s WHERE member_id='%i'",
		MYSQL_STATS, iMemberID );

	int iResult = mysql_real_query( m_pMySQLConnection, szString, strlen( szString ) );

	if( iResult != 0)
	{
	#ifdef _DEBUG

		MySQLError( );

	#endif

		return false;
	}

	CWeaponStats WeaponStats;

	while( ( ppRow = mysql_fetch_row( pResource ) ) != NULL )
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
bool CDatabaseManager::ReadStats( MYSQL_RES *pResource, CBaseStats &BaseStats )
{
	MYSQL_ROW ppRow = mysql_fetch_row( pResource );

	if( !ppRow )
		return false;

	// handle data
	MYSQL_FIELD *pField;

	for( int i = 0; i < ( int )mysql_num_fields( pResource ); i++ )
	{
		pField = mysql_fetch_field( pResource );

		const char *pszFieldName = pField->name;

		if( !pszFieldName )
			continue;

		int iType = BaseStats.GetTypeName( pszFieldName );

		if( iType == INVALID_STATSTYPE )
			continue;

		BaseStats.Increment( iType, atoi( ppRow[ i ] ) );
	}
}


	bool FetchWeaponStats( int iMemberID, const char *pszDatabase, PlayerStats_t &PlayerStats );

	bool IsPlayerUpdate( int iMemberID, const string &strServerDB );

	bool WriteStats( int iMemberID, const char *pszDatabase, const PlayerStats_t &PlayerStats );
	bool UpdateStats(int iMemberID, string &strServer, const PlayerStats_t &PlayerStats);

private:
	MYSQL *m_pMySQLConnection;
};
