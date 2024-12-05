<?php

define( "MYSQL_HOSTNAME", "localhost" );
define( "MYSQL_USERNAME", "insmod_forums" );
define( "MYSQL_PASSWORD", "" );
define( "MYSQL_DATABASE", "insmod_forums" );

define( "POST_TABLE", "ins_posts" );
define( "POST_ID", 60709 );

if( !mysql_connect( MYSQL_HOSTNAME, MYSQL_USERNAME, MYSQL_PASSWORD ) )
    die( "Cannot Connect" );
    
if( !mysql_select_db( MYSQL_DATABASE ) )
    die( "Cannot Select DB" );
    
$hTopic = mysql_query( "SELECT post FROM " . POST_TABLE . " WHERE pid='" . POST_ID . "' LIMIT 0,1" );

if( !$hTopic )
    die( "Bad Query" );
    
$rgTopic = mysql_fetch_array( $hTopic );

if( empty( $rgTopic ) )
    die( "Topic Returned Bad Data" );
    
echo $rgTopic[ "post" ];

?>