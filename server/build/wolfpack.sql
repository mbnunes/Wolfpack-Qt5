
CREATE TABLE `characters` (
	`serial` int(10) unsigned NOT NULL default '0',
	`name` varchar(255) default NULL,
	`title` varchar(255) default NULL,
	`creationdate` varchar(19) default NULL,
	`body` smallint(5) unsigned NOT NULL default '0',
	`orgbody` smallint(5) unsigned NOT NULL default '0',
	`skin` smallint(5) unsigned NOT NULL default '0',
	`orgskin` smallint(5) unsigned NOT NULL default '0',
	`saycolor` smallint(5) unsigned NOT NULL default '0',
	`emotecolor` smallint(5) unsigned NOT NULL default '0',
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
	`kills` int(10) unsigned NOT NULL default '0',
	`deaths` int(10) unsigned NOT NULL default '0',
	`hunger` int(10) unsigned NOT NULL default '0',
	`poison` tinyint(2) NOT NULL default '-1',
	`murderertime` int(10) unsigned NOT NULL default '0',
	`criminaltime` int(10) unsigned NOT NULL default '0',
	`gender` tinyint(1) unsigned NOT NULL default '0',
	`propertyflags` int(11) NOT NULL default '0',
	`murderer` int(10) unsigned NOT NULL default '0',
	`guarding` int(10) unsigned NOT NULL default '0',
	`hitpointsbonus` smallint(6) NOT NULL default '0',
	`staminabonus` smallint(6) NOT NULL default '0',
	`manabonus` smallint(6) NOT NULL default '0',
	`strcap` tinyint(4) NOT NULL default '125',
	`dexcap` tinyint(4) NOT NULL default '125',
	`intcap` tinyint(4) NOT NULL default '125',
	`statcap` tinyint(4) NOT NULL default '225',
	`baseid` varchar(64) NOT NULL default '',
	`direction` tinyint(1) unsigned NOT NULL default '0',
	PRIMARY KEY  (`serial`)
);

CREATE TABLE `corpses` (
	`serial` int(10) unsigned NOT NULL default '0',
	`bodyid` smallint(5) unsigned NOT NULL default '0',
	`hairstyle` smallint(5) unsigned NOT NULL default '0',
	`haircolor` smallint(5) unsigned NOT NULL default '0',
	`beardstyle` smallint(5) unsigned NOT NULL default '0',
	`beardcolor` smallint(5) unsigned NOT NULL default '0',
	`direction` tinyint(1) unsigned NOT NULL default '0',
	`charbaseid` varchar(64) NOT NULL default '',
	`murderer` int(10) unsigned NOT NULL default '0',
	`murdertime` int(10) unsigned NOT NULL default '0',
	PRIMARY KEY  (`serial`)
);

CREATE TABLE `corpses_equipment` (
	`serial` int(10) unsigned NOT NULL default '0',
	`layer` tinyint(3) unsigned NOT NULL default '0',
	`item` int(10) unsigned NOT NULL default '0',
	PRIMARY KEY  (`serial`,`layer`)
);

CREATE TABLE `effects` (
	`id` int(10) unsigned NOT NULL default '0',
	`objectid` varchar(64) NOT NULL default '',
	`expiretime` int(10) unsigned NOT NULL default '0',
	`dispellable` tinyint(4) NOT NULL default '0',
	`source` int(10) unsigned NOT NULL default '0',
	`destination` int(10) unsigned NOT NULL default '0',
	PRIMARY KEY  (`id`)
);

CREATE TABLE `effects_properties` (
	`id` int(10) unsigned NOT NULL default '0',
	`keyname` varchar(64) NOT NULL default '',
	`type` varchar(64) NOT NULL default '',
	`value` text NOT NULL,
	PRIMARY KEY  (`id`,`keyname`)
);

CREATE TABLE `guilds` (
	`serial` int(10) unsigned NOT NULL default '0',
	`name` varchar(255) NOT NULL default '',
	`abbreviation` varchar(6) NOT NULL default '',
	`charta` longtext NOT NULL,
	`website` varchar(255) NOT NULL default 'http://www.wpdev.org',
	`alignment` tinyint(2) NOT NULL default '0',
	`leader` int(10) unsigned NOT NULL default '0',
	`founded` int(11) NOT NULL default '0',
	`guildstone` int(10) unsigned NOT NULL default '0',
	PRIMARY KEY  (`serial`)
);

CREATE TABLE `guilds_canidates` (
	`guild` int(10) unsigned NOT NULL default '0',
	`player` int(10) unsigned NOT NULL default '0',
	PRIMARY KEY  (`guild`,`player`)
);

CREATE TABLE `guilds_members` (
	`guild` int(10) unsigned NOT NULL default '0',
	`player` int(10) unsigned NOT NULL default '0',
	`showsign` tinyint(1) unsigned NOT NULL default '0',
	`guildtitle` varchar(255) NOT NULL default '',
	`joined` int(11) NOT NULL default '0',
	PRIMARY KEY  (`guild`,`player`)
);

CREATE TABLE `items` (
	`serial` int(10) unsigned NOT NULL default '0',
	`id` smallint(5) unsigned NOT NULL default '0',
	`color` smallint(5) unsigned NOT NULL default '0',
	`cont` int(10) unsigned NOT NULL default '0',
	`layer` tinyint(3) unsigned NOT NULL default '0',
	`amount` smallint(5) NOT NULL default '0',
	`hp` smallint(6) NOT NULL default '0',
	`maxhp` smallint(6) NOT NULL default '0',
	`movable` tinyint(3) NOT NULL default '0',
	`owner` int(10) unsigned NOT NULL default '0',
	`visible` tinyint(3) NOT NULL default '0',
	`priv` tinyint(3) NOT NULL default '0',
	`baseid` varchar(64) NOT NULL default '',
  PRIMARY KEY  (`serial`)
);

CREATE TABLE `npcs` (
	`serial` int(10) unsigned NOT NULL default '0',
	`summontime` int(11) NOT NULL default '0',
	`additionalflags` int(11) NOT NULL default '0',
	`owner` int(10) unsigned NOT NULL default '0',
	`stablemaster` int(10) unsigned NOT NULL default '0',
	`ai` varchar(255) default NULL,
	`wandertype` smallint(3) NOT NULL default '0',
	`wanderx1` smallint(6) NOT NULL default '0',
	`wanderx2` smallint(6) NOT NULL default '0',
	`wandery1` smallint(6) NOT NULL default '0',
	`wandery2` smallint(6) NOT NULL default '0',
	`wanderradius` smallint(6) NOT NULL default '0',
	PRIMARY KEY  (`serial`)
);

CREATE TABLE `players` (
	`serial` int(10) unsigned NOT NULL default '0',
	`account` varchar(16) default NULL,
	`additionalflags` int(10) NOT NULL default '0',
	`visualrange` tinyint(3) unsigned NOT NULL default '0',
	`profile` longtext,
	`fixedlight` tinyint(3) unsigned NOT NULL default '0',
	`strlock` tinyint(4) NOT NULL default '0',
	`dexlock` tinyint(4) NOT NULL default '0',
	`intlock` tinyint(4) NOT NULL default '0',
	PRIMARY KEY  (`serial`)
);

CREATE TABLE `settings` (
	`option` varchar(255) NOT NULL default '',
	`value` varchar(255) NOT NULL default '',
	PRIMARY KEY  (`option`)
);

CREATE TABLE `skills` (
	`serial` int(10) unsigned NOT NULL default '0',
	`skill` tinyint(2) unsigned NOT NULL default '0',
	`value` smallint(6) NOT NULL default '0',
	`locktype` tinyint(4) default '0',
	`cap` smallint(6) default '0',
	PRIMARY KEY  (`serial`,`skill`)
);

CREATE TABLE `spawnregions` (
	`spawnregion` varchar(64) NOT NULL default '',
	`serial` int(10) unsigned NOT NULL default '0',
	PRIMARY KEY  (`spawnregion`,`serial`)
);

CREATE TABLE `tags` (
	`serial` int(10) unsigned NOT NULL default '0',
	`name` varchar(64) NOT NULL default '',
	`type` varchar(6) NOT NULL default '',
	`value` longtext NOT NULL,
	PRIMARY KEY  (`serial`,`name`)
);

CREATE TABLE `uobject` (
	`name` varchar(255) default NULL,
	`serial` int(10) unsigned NOT NULL default '0',
	`multis` int(10) unsigned NOT NULL default '0',
	`pos_x` smallint(6) unsigned NOT NULL default '0',
	`pos_y` smallint(6) unsigned NOT NULL default '0',
	`pos_z` tinyint(4) NOT NULL default '0',
	`pos_map` tinyint(1) unsigned NOT NULL default '0',
	`events` varchar(255) default NULL,
	`havetags` tinyint(1) unsigned NOT NULL default '0',
	PRIMARY KEY  (`serial`)
);

CREATE TABLE `uobjectmap` (
	`serial` int(10) unsigned NOT NULL default '0',
	`type` varchar(80) NOT NULL default '',
	PRIMARY KEY  (`serial`)
);
