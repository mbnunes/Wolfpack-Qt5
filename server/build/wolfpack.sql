CREATE TABLE `settings` (
	`option` varchar(255) NOT NULL default '',
	`value` varchar(255) NOT NULL default '',
	PRIMARY KEY (`option`)
);

CREATE TABLE `boats` (
	`serial` int(11) NOT NULL default '0',
	`autosail` tinyint(1) NOT NULL default '0',
	`boatdir` tinyint(1) NOT NULL default '0',
	`itemserial1` int(11) NOT NULL default '-1',
	`itemserial2` int(11) NOT NULL default '-1',
	`itemserial3` int(11) NOT NULL default '-1',
	`itemserial4` int(11) NOT NULL default '-1',
	`multi1` smallint(6) default '0',
	`multi2` smallint(6) default '0',
	`multi3` smallint(6) default '0',
	`multi4` smallint(6) default '0',
	PRIMARY KEY (`serial`)
);

CREATE TABLE `boats_itemids` (
	`serial` int(11) NOT NULL default '0',
	`a` tinyint(1)  NOT NULL default '0',
	`b` tinyint(1)  NOT NULL default '0',
	`id` smallint(6)  default '0',
	PRIMARY KEY (`serial`)
);

CREATE TABLE `boats_itemoffsets` (
	`serial` int(11) NOT NULL default '0',
	`a` tinyint(1)  NOT NULL default '0',
	`b` tinyint(1)  NOT NULL default '0',
	`c` tinyint(1)  NOT NULL default '0',
	`offset` smallint(6) default '0',
	PRIMARY KEY (`serial`)
);

CREATE TABLE `bookpages` (
	`serial` int(11) NOT NULL default '0',
	`page` tinyint(3) NOT NULL default '0',
	`text` varchar(255) NOT NULL default '',
	PRIMARY KEY (`serial`,`page`)
);

CREATE TABLE `books` (
	`serial` int(11) NOT NULL default '0',
	`title` varchar(255) NOT NULL default '',
	`author` varchar(255) NOT NULL default '',
	`readonly` tinyint(1) NOT NULL default '0',
	`predefined` tinyint(1) NOT NULL default '0',
	`section` varchar(255)  NOT NULL default '',
	`pages` smallint(5)  NOT NULL default '0',
	PRIMARY KEY (`serial`)
);

CREATE TABLE `characters` (
	`serial` int(11) NOT NULL default '0',
	`name` varchar(255) default NULL,
	`title` varchar(255) default NULL,
	`creationdate` varchar(255) default NULL,
	`body` smallint(5)  NOT NULL default '0',
	`orgbody` smallint(5)  NOT NULL default '0',
	`skin` smallint(5)  NOT NULL default '0',
	`orgskin` smallint(5)  NOT NULL default '0',
	`saycolor` smallint(5)  NOT NULL default '0',
	`emotecolor` smallint(5)  NOT NULL default '0',
	`strength` smallint(6) NOT NULL default '0',
	`strengthmod` smallint(6) NOT NULL default '0',
	`dexterity` smallint(6) NOT NULL default '0',
	`dexteritymod` smallint(6) NOT NULL default '0',
	`intelligence` smallint(6) NOT NULL default '0',
	`intelligencemod` smallint(6) NOT NULL default '0',
	`maxhitpoints` smallint(6) NOT NULL default '0',
	`hitpoints` smallint(6) NOT NULL default '0',
	`maxstamina` smallint(6) NOT NULL default '0',
	`stamina` smallint(6) NOT NULL default '0',
	`maxmana` smallint(6) default NULL,
	`mana` smallint(6) default NULL,
	`karma` int(11) NOT NULL default '0',
	`fame` int(11) NOT NULL default '0',
	`kills` int(10)  NOT NULL default '0',
	`deaths` int(10)  NOT NULL default '0',
	`def` int(10)  NOT NULL default '0',
	`hunger` int(11) NOT NULL default '0',
	`poison` int(11) NOT NULL default '0',
	`poisoned` int(10)  NOT NULL default '0',
	`murderertime` int(11)  NOT NULL default '0',
	`criminaltime` int(11)  NOT NULL default '0',	
	`gender` tinyint(1)  NOT NULL default '0',
	`propertyflags` int(11)  NOT NULL default '0',
	`murderer` int(11) NOT NULL default '-1',
	`guarding` int(11) NOT NULL default '-1',
	PRIMARY KEY (`serial`)
);

CREATE TABLE `corpses` (
	`serial` int(11) NOT NULL default '0',
	`bodyid` smallint(6)  NOT NULL default '0',
	`hairstyle` smallint(6)  NOT NULL default '0',
	`haircolor` smallint(6)  NOT NULL default '0',
	`beardstyle` smallint(6)  NOT NULL default '0',
	`beardcolor` smallint(6)  NOT NULL default '0',
	PRIMARY KEY (`serial`)
);

CREATE TABLE `corpses_equipment` (
	`serial` int(11) NOT NULL default '0',
	`layer` tinyint(3)  NOT NULL default '0',
	`item` int(11) NOT NULL default '-1',
	PRIMARY KEY (`serial`,`layer`)
);

CREATE TABLE `houses` (
	`serial` int(11) NOT NULL default '0',
	`nokey` tinyint(1) NOT NULL default '0',
	`charpos_x` smallint(6) NOT NULL default '0',
	`charpos_y` smallint(6) NOT NULL default '0',
	`charpos_z` smallint(6) NOT NULL default '0',
	PRIMARY KEY (`serial`)
);

CREATE TABLE `items` (
	`serial` int(11) NOT NULL default '0',
	`id` smallint(5)  NOT NULL default '0',
	`color` smallint(5)  NOT NULL default '0',
	`cont` int(11) NOT NULL default '-1',
	`layer` tinyint(3)  NOT NULL default '0',
	`type` smallint(5)  NOT NULL default '0',
	`amount` smallint(5)  NOT NULL default '0',
	`decaytime` int(10)  NOT NULL default '0',
	`weight` float NOT NULL default '0',
	`hp` smallint(6) NOT NULL default '0',
	`maxhp` smallint(6) NOT NULL default '0',
	`magic` tinyint(3)  NOT NULL default '0',
	`owner` int(11) NOT NULL default '-1',
	`visible` tinyint(3)  NOT NULL default '0',
	`spawnregion` varchar(255) default NULL,
	`priv` tinyint(3)  NOT NULL default '0',
	`sellprice` int(11) NOT NULL default '0',
	`buyprice` int(11) NOT NULL default '0',
	`restock` smallint(5)  NOT NULL default '0',
	`baseid` varchar(32) NOT NULL default '',
	PRIMARY KEY (`serial`)
);

CREATE TABLE `multis` (
	`serial` int(11) NOT NULL default '0',
	`coowner` int(11) NOT NULL default '-1',
	`deedsection` varchar(255) NOT NULL default '',
	PRIMARY KEY (`serial`)
);

CREATE TABLE `multis_bans` (
	`serial` int(11) NOT NULL default '0',
	`ban` int(11) NOT NULL default '-1',
	PRIMARY KEY (`serial`,`ban`)
);

CREATE TABLE `multis_friends` (
	`serial` int(11) NOT NULL default '0',
	`friend` int(11) NOT NULL default '-1',
	PRIMARY KEY (`serial`,`friend`)
);

CREATE TABLE `npcs` (
	`serial` int(11) NOT NULL default '0',
	`mindamage` smallint(6)  NOT NULL default '0',
	`maxdamage` smallint(6)  NOT NULL default '0',
	`tamingminskill` smallint(6)  NOT NULL default '0',
	`summontime` int(11)  NOT NULL default '0',
	`additionalflags` int(11)  NOT NULL default '0',
	`owner` int(11) NOT NULL default '-1',
	`carve` varchar(255) default NULL,
	`spawnregion` varchar(255) default NULL,
	`stablemaster` int(11) NOT NULL default '-1',
	`lootlist` varchar(255) default NULL,
	`ai` varchar(255) default NULL,
	`wandertype` smallint(3) NOT NULL default '0',
	`wanderx1` smallint(6) NOT NULL default '0',
	`wanderx2` smallint(6) NOT NULL default '0',
	`wandery1` smallint(6) NOT NULL default '0',
	`wandery2` smallint(6) NOT NULL default '0',
	`wanderradius` smallint(6) NOT NULL default '0',
	`fleeat` smallint(3)  NOT NULL default '10',
	`spellslow` int(11)  NOT NULL default '0',
	`spellshigh` int(11)  NOT NULL default '0',
	PRIMARY KEY (`serial`)
);

CREATE TABLE `players` (
	`serial` int(11) NOT NULL default '0',
	`account` varchar(255) default NULL,
	`additionalflags` int(10)  NOT NULL default '0',
	`visualrange` tinyint(3)  NOT NULL default '0',
	`profile` longtext,
	`fixedlight` tinyint(3)  NOT NULL default '0',
	`strcap` tinyint(4)  NOT NULL default '100',
	`dexcap` tinyint(4)  NOT NULL default '100',
	`intcap` tinyint(4)  NOT NULL default '100',
	`strlock` tinyint(4)  NOT NULL default '0',
	`dexlock` tinyint(4)  NOT NULL default '0',
	`intlock` tinyint(4)  NOT NULL default '0',
	PRIMARY KEY (`serial`)
);

CREATE TABLE `skills` (
	`serial` int(11) NOT NULL default '0',
	`skill` tinyint(3)  NOT NULL default '0',
	`value` smallint(6) NOT NULL default '0',
	`locktype` tinyint(4) default '0',
	`cap` smallint(6) default '0',
	PRIMARY KEY (`serial`,`skill`)
);

CREATE TABLE `tags` (
	`serial` int(11) NOT NULL default '0',
	`name` varchar(64) NOT NULL default '',
	`type` varchar(6) NOT NULL default '',
	`value` longtext NOT NULL,
	PRIMARY KEY (`serial`,`name`)
);

CREATE TABLE `uobject` (
	`name` varchar(255) default NULL,
	`serial` int(11) NOT NULL default '0',
	`multis` int(11) NOT NULL default '-1',
	`direction` char(1) NOT NULL default '0',
	`pos_x` smallint(6)  NOT NULL default '0',
	`pos_y` smallint(6)  NOT NULL default '0',
	`pos_z` smallint(6) NOT NULL default '0',
	`pos_map` tinyint(4) NOT NULL default '0',
	`events` varchar(255) default NULL,
	`bindmenu` varchar(255) default NULL,
	`havetags` tinyint(1) NOT NULL default '0',
	PRIMARY KEY (`serial`)
);

CREATE TABLE `uobjectmap` (
	`serial` int(11) NOT NULL default '0',
	`type` varchar(80)  NOT NULL default '',
	PRIMARY KEY (`serial`)
);

CREATE TABLE `effects` (
	`id` int(11) NOT NULL default '0',
	`objectid` varchar(64) NOT NULL default '',
	`expiretime` int(11) NOT NULL default '0',
	`dispellable` tinyint(4) NOT NULL default '0',
	`source` int(11) NOT NULL default '-1',
	`destination` int(11) NOT NULL default '-1',
	PRIMARY KEY  (`id`)
);

CREATE TABLE `effects_properties` (
	`id` int(11) NOT NULL default '0',
	`keyname` varchar(64) NOT NULL default '',
	`type` varchar(64) NOT NULL default '',
	`value` text NOT NULL,
	PRIMARY KEY  (`id`,`keyname`)
);

CREATE TABLE `guilds` (
	`serial` int(11) NOT NULL default '0',
	`name` varchar(255) NOT NULL default '',
	`abbreviation` varchar(255) NOT NULL default '',
	`charta` LONGTEXT NOT NULL,
	`website` varchar(255) NOT NULL default '',
	`alignment` int(2) NOT NULL default '0',
	`leader` int(11) NOT NULL default '-1',
	`founded` int(11) NOT NULL default '0',
	`guildstone` int(11) NOT NULL default '-1',
	PRIMARY KEY(`serial`)
);

CREATE TABLE `guilds_members` (
	`guild` int(11) NOT NULL default '0',
	`player` int(11) NOT NULL default '0',
	`showsign` int(1) NOT NULL default '0',
	`guildtitle` varchar(255) NOT NULL default '',
	`joined` int(11) NOT NULL default '0',
	PRIMARY KEY(`guild`,`player`)
);

CREATE TABLE `guilds_canidates` (
	`guild` int(11) NOT NULL default '0',
	`player` int(11) NOT NULL default '0',
	PRIMARY KEY(`guild`,`player`)
);
