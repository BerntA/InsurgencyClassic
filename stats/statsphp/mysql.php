<?php

// *** MySQL Constants
define( "MYSQL_SERVER", "localhost" );
define( "MYSQL_DBUSER", "mainuser" );
define( "MYSQL_DBPASS", "golden88" );

define( "MYSQL_SERVERPREFIX", "ins_" );
define( "MYSQL_GLOBAL", "global" );

define( "MYSQL_STATS", "stats" );
define( "MYSQL_WEAPONSTATS", "stats_weapon" );

define( "MYSQL_AWARDS", "awards" );
define( "MYSQL_SERVERS", "servers" );

// *** Connect to MySQL

// set error control
if( !DEBUG )
	error_reporting( 0 );

// connect to database
if( !mysql_connect( MYSQL_SERVER, MYSQL_DBUSER, MYSQL_DBPASS ) )
	exit( );
    
?>
