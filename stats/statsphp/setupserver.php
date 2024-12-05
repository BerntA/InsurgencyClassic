#!/usr/bin/php -q
<?php

// *** Show User Information
echo "-- INS Offical Server Creation Script\n";
echo "-- Created by: James 'Pongles' Mansfield";
echo "-- Stats Protocol: 1\n";

// *** Show Correct Usage (if needed)
if($_SERVER["argc"] == 1 || ($_SERVER["argc"] == 2 && $_SERVER["argv"][1] != "setup" && $_SERVER["argv"][1] != "remove"))
	die("Correct Usage: ".$_SERVER["arvc"][0]." [\"setup\" or \"remove\" or [servername] [password]\n");
	
// *** Open MySQL
require("mysql.php");

// *** Define Common Tables
function GetStatsTable($strDBName)
{
	$strStatsTable = "CREATE TABLE `".MYSQL_STATS."` (
 		`id` mediumint(8) NOT NULL auto_increment,
 		`member_id` mediumint(8) NOT NULL default '0',";
 	
 	if($strDBName == MYSQL_GLOBAL)
 		$strStatsTable .= "`last_server` varchar(32) NOT NULL default '',";
 		
 	$strStatsTable .= "`killpts` mediumint(4) unsigned NOT NULL default '0',
  		`objpts` mediumint(4) unsigned NOT NULL default '0',
  		`leaderpts` mediumint(4) unsigned NOT NULL default '0',
  		`kills` mediumint(4) unsigned NOT NULL default '0',
  		`ff_kills` mediumint(4) unsigned NOT NULL default '0',
  		`deaths` mediumint(4) unsigned NOT NULL default '0',
  		`ping` mediumint(4) unsigned NOT NULL default '0',
  		`jumps` mediumint(4) unsigned NOT NULL default '0',
  		`proned` mediumint(4) unsigned NOT NULL default '0',
  		`talked` mediumint(4) unsigned NOT NULL default '0',
  		`hours`	mediumint(4) unsigned NOT NULL default '0',
  		PRIMARY KEY (`id`)
		) TYPE=MyISAM;";

	return $strStatsTable;
}

// *** Define Common Tables
$strStatsWeaponTable = "CREATE TABLE `".MYSQL_WEAPONSTATS."` (
  `id` mediumint(8) NOT NULL auto_increment,
  `member_id` mediumint(8) unsigned NOT NULL default '0',
  `weapon_id` mediumint(8) unsigned NOT NULL default '0',
  `shots` mediumint(4) unsigned NOT NULL default '0',
  `hits_head` mediumint(4) unsigned NOT NULL default '0',
  `hits_body` mediumint(4) unsigned NOT NULL default '0',
  `hits_larm` mediumint(4) unsigned NOT NULL default '0',
  `hits_rarm` mediumint(4) unsigned NOT NULL default '0',
  `hits_lleg` mediumint(4) unsigned NOT NULL default '0',
  `hits_rleg` mediumint(4) unsigned NOT NULL default '0',
  `frags` mediumint(4) unsigned NOT NULL default '0',
  PRIMARY KEY  (`id`)
) TYPE=MyISAM;";

function CreateStatsDatabase($strDBName)
{
	global $strStatsWeaponTable;
	
	echo "Creating Database ".$strDBName."\n";
	
	if(!mysql_query("CREATE DATABASE ".$strDBName))
		die("Creation of ".$strDBName." Failed\n");
		
	if(!mysql_select_db($strDBName))
		die("Unable to Select ".$strDBName);
		
	echo "Creating Table ".MYSQL_STATS."\n";
	
	if(!mysql_query(GetStatsTable($strDBName)))
		die("Creation of ".MYSQL_STATS." Failed\n");
	
	echo "Creating Table ".MYSQL_WEAPONSTATS."\n";
	
	if(!mysql_query($strStatsWeaponTable))
		die("Creation of ".MYSQL_WEAPONSTATS." Failed\n");	
}

function CreateServerString($strServerName)
{
	return MYSQL_SERVERPREFIX.$strServerName;
}

// *** Setup Global Stats	
if($_SERVER["argc"] == 2)
{
	if($_SERVER["argv"][1] == "setup")
	{
		CreateStatsDatabase(MYSQL_GLOBAL);
		
		$strAwardTable = "CREATE TABLE `awards` (
  		`id` mediumint(8) NOT NULL auto_increment,
  		`award_id` mediumint(8) unsigned NOT NULL default '0',
  		`member_id` mediumint(8) unsigned NOT NULL default '0',
  		`data` mediumint(4) unsigned NOT NULL default '0',
  		PRIMARY KEY (`id`)
		) TYPE=MyISAM;";
	
		echo "Creating Table ".MYSQL_AWARDS."\n";
	
		if(!mysql_query($strAwardTable))
			die("Creation of ".MYSQL_AWARDS." Failed\n");
		
		$strServersTable = "CREATE TABLE `".MYSQL_SERVERS."` (
	  	`id` mediumint(8) NOT NULL auto_increment,
	  	`name` varchar(32) NOT NULL default '0',
	  	`password` varchar(32) NOT NULL default '',
	  	PRIMARY KEY (`id`)
		) TYPE=MyISAM;";
	
		echo "Creating Table ".MYSQL_SERVERS."\n";
	
		if(!mysql_query($strServersTable))
			die("Creation of ".MYSQL_SERVERS." Failed\n");		
		
		echo "Setup Finished!\n";
		exit();
	}
	else /* "remove" */
	{
		if(mysql_select_db(MYSQL_GLOBAL))
		{
			echo "Removing all Servers\n";
			
			$sqlServers = mysql_query("SELECT name FROM ".MYSQL_SERVERS);
			
			if($rgServerRow = mysql_fetch_array($sqlServers))
			{
				do
				{
					$strServerName = $rgServerRow["name"];
					$strServerNameMySQL = CreateServerString($strServerName);
					
					echo "Removing ".$strServerName." (".$strServerNameMySQL.")\n";
					
					mysql_query("DROP DATABASE IF EXISTS ".$strServerNameMySQL);
					
				} while($rgServerRow = mysql_fetch_array($sqlServers));
			}
		}
		
		echo "Removing ".MYSQL_GLOBAL."\n";		
		
		mysql_query("DROP DATABASE IF EXISTS ".MYSQL_GLOBAL);		
		
		echo "Remove Finished!\n";
		exit();		
	}
}
else if($_SERVER["argv"][1] == "remove" && is_string($_SERVER["argv"][2]))
{
	$strServerName = $_SERVER["argv"][2];
	$strServerNameMySQL = CreateServerString($strServerName);
	
	if(mysql_select_db(MYSQL_GLOBAL))
		mysql_query("DELETE FROM ".MYSQL_SERVERS." WHERE name='".$strServerNameL."'");

	mysql_query("DROP DATABASE IF EXISTS ".$strServerNameMySQL);
	
	echo "Remove Finished!\n";
	exit();			
}

// *** Create Server
$strServerName = $_SERVER["argv"][1];
$strPassword = $_SERVER["argv"][2];

$strServerNameMySQL = MYSQL_SERVERPREFIX.$strServerName;

echo "Creating Server ".$strServerName." (".$strServerNameMySQL.") with Password ".$strPassword."\n";

CreateStatsDatabase($strServerNameMySQL);

if(!mysql_select_db(MYSQL_GLOBAL))
	die("Unable to Select ".MYSQL_GLOBAL." - Server has been Created but not Registered\n");
	
$strMD5Password = md5($strPassword);
	
if(!mysql_query("INSERT ".MYSQL_SERVERS." SET name='".$strServerName."',password='".$strMD5Password."'"))
	die("Unable to Register Server\n");
	
echo "Server Creation Finished!\n";
?>
