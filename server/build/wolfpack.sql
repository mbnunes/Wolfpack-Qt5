CREATE TABLE `boats` (
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
  `serial` int(11) NOT NULL default '0',
  PRIMARY KEY (`serial`)
);

CREATE TABLE `boats_itemids` (
  `a` tinyint(1)  NOT NULL default '0',
  `b` tinyint(1)  NOT NULL default '0',
  `id` smallint(6)  default '0',
  `serial` int(11) NOT NULL default '0',
  PRIMARY KEY (`serial`)
);

CREATE TABLE `boats_itemoffsets` (
  `a` tinyint(1)  NOT NULL default '0',
  `b` tinyint(1)  NOT NULL default '0',
  `c` tinyint(1)  NOT NULL default '0',
  `offset` smallint(6) default '0',
  `serial` int(11) NOT NULL default '0',
  PRIMARY KEY (`serial`)
);

CREATE TABLE `bookpages` (
  `page` tinyint(3) NOT NULL default '0',
  `serial` int(11) NOT NULL default '0',
  `text` varchar(255) NOT NULL default '',
  PRIMARY KEY (`serial`,`page`)
);

CREATE TABLE `books` (
  `author` varchar(255) NOT NULL default '',
  `pages` smallint(5)  NOT NULL default '0',
  `predefined` tinyint(1) NOT NULL default '0',
  `readonly` tinyint(1) NOT NULL default '0',
  `section` varchar(255)  NOT NULL default '',
  `serial` int(11) NOT NULL default '0',
  `title` varchar(255) NOT NULL default '',
  PRIMARY KEY (`serial`)
);

CREATE TABLE `characters` (
  `attacker` int(11) NOT NULL default '-1',
  `body` smallint(5)  NOT NULL default '0',
  `combattarget` int(11) NOT NULL default '-1',
  `creationdate` varchar(255) default NULL,
  `criminaltime` int(11)  NOT NULL default '0',
  `deaths` int(10)  NOT NULL default '0',
  `def` int(10)  NOT NULL default '0',
  `dexteritymod` smallint(6) NOT NULL default '0',
  `dexterity` smallint(6) NOT NULL default '0',
  `emotecolor` smallint(5)  NOT NULL default '0',
  `fame` int(11) NOT NULL default '0',
  `gender` tinyint(1)  NOT NULL default '0',
  `guarding` int(11) NOT NULL default '-1',
  `hitpoints` smallint(6) NOT NULL default '0',
  `hunger` int(11) NOT NULL default '0',
  `intelligencemod` smallint(6) NOT NULL default '0',
  `intelligence` smallint(6) NOT NULL default '0',
  `karma` int(11) NOT NULL default '0',
  `kills` int(10)  NOT NULL default '0',
  `mana` smallint(6) default NULL,
  `maxhitpoints` smallint(6) NOT NULL default '0',
  `maxmana` smallint(6) default NULL,
  `maxstamina` smallint(6) NOT NULL default '0',
  `murderertime` int(11)  NOT NULL default '0',
  `murderer` int(11) NOT NULL default '-1',
  `name` varchar(255) default NULL,
  `nutriment` int(10)  NOT NULL default '0',
  `orgbody` smallint(5)  NOT NULL default '0',
  `orgskin` smallint(5)  NOT NULL default '0',
  `poisoned` int(10)  NOT NULL default '0',
  `poison` int(11) NOT NULL default '0',
  `propertyflags` int(11)  NOT NULL default '0',
  `saycolor` smallint(5)  NOT NULL default '0',
  `serial` int(11) NOT NULL default '0',
  `skin` smallint(5)  NOT NULL default '0',
  `stamina` smallint(6) NOT NULL default '0',
  `strengthmod` smallint(6) NOT NULL default '0',
  `strength` smallint(6) NOT NULL default '0',
  `title` varchar(255) default NULL,
  PRIMARY KEY (`serial`)
);

CREATE TABLE `corpses` (
  `beardcolor` smallint(6)  NOT NULL default '0',
  `beardstyle` smallint(6)  NOT NULL default '0',
  `bodyid` smallint(6)  NOT NULL default '0',
  `haircolor` smallint(6)  NOT NULL default '0',
  `hairstyle` smallint(6)  NOT NULL default '0',
  `serial` int(11) NOT NULL default '0',
  PRIMARY KEY (`serial`)
);

CREATE TABLE `corpses_equipment` (
  `item` int(11) NOT NULL default '-1',
  `layer` tinyint(3)  NOT NULL default '0',
  `serial` int(11) NOT NULL default '0',
  PRIMARY KEY (`serial`,`layer`)
);

CREATE TABLE `houses` (
  `charpos_x` smallint(6) NOT NULL default '0',
  `charpos_y` smallint(6) NOT NULL default '0',
  `charpos_z` smallint(6) NOT NULL default '0',
  `nokey` tinyint(1) NOT NULL default '0',
  `serial` int(11) NOT NULL default '0',
  PRIMARY KEY (`serial`)
);

CREATE TABLE `items` (
  `amount` smallint(5)  NOT NULL default '0',
  `baseid` varchar(32) NOT NULL default '',
  `buyprice` int(11) NOT NULL default '0',
  `color` smallint(5)  NOT NULL default '0',
  `cont` int(11) NOT NULL default '-1',
  `decaytime` int(10)  NOT NULL default '0',
  `def` int(10)  NOT NULL default '0',
  `hidamage` smallint(6) NOT NULL default '0',
  `hp` smallint(6) NOT NULL default '0',
  `id` smallint(5)  NOT NULL default '0',
  `layer` tinyint(3)  NOT NULL default '0',
  `lodamage` smallint(6) NOT NULL default '0',
  `magic` tinyint(3)  NOT NULL default '0',
  `maxhp` smallint(6) NOT NULL default '0',
  `owner` int(11) NOT NULL default '-1',
  `poisoned` int(10)  NOT NULL default '0',
  `priv` tinyint(3)  NOT NULL default '0',
  `restock` smallint(5)  NOT NULL default '0',
  `sellprice` int(11) NOT NULL default '0',
  `serial` int(11) NOT NULL default '0',
  `spawnregion` varchar(255) default NULL,
  `speed` int(11) NOT NULL default '0',
  `type2` smallint(5)  NOT NULL default '0',
  `type` smallint(5)  NOT NULL default '0',
  `visible` tinyint(3)  NOT NULL default '0',
  `weight` int(11) NOT NULL default '0',
  PRIMARY KEY (`serial`)
);

CREATE TABLE `multis` (
  `coowner` int(11) NOT NULL default '-1',
  `deedsection` varchar(255) NOT NULL default '',
  `serial` int(11) NOT NULL default '0',
  PRIMARY KEY (`serial`)
);

CREATE TABLE `multis_bans` (
  `ban` int(11) NOT NULL default '-1',
  `serial` int(11) NOT NULL default '0',
  PRIMARY KEY (`serial`,`ban`)
);

CREATE TABLE `multis_friends` (
  `friend` int(11) NOT NULL default '-1',
  `serial` int(11) NOT NULL default '0',
  PRIMARY KEY (`serial`,`friend`)
);

CREATE TABLE `npcs` (
  `additionalflags` int(11)  NOT NULL default '0',
  `ai` varchar(255) default NULL,
  `carve` varchar(255) default NULL,
  `fleeat` smallint(3)  NOT NULL default '10',
  `lootlist` varchar(255) default NULL,
  `maxdamage` smallint(6)  NOT NULL default '0',
  `mindamage` smallint(6)  NOT NULL default '0',
  `owner` int(11) NOT NULL default '-1',
  `serial` int(11) NOT NULL default '0',
  `spawnregion` varchar(255) default NULL,
  `spellshigh` int(11)  NOT NULL default '0',
  `spellslow` int(11)  NOT NULL default '0',
  `stablemaster` int(11) NOT NULL default '-1',
  `summontime` int(11)  NOT NULL default '0',
  `tamingminskill` smallint(6)  NOT NULL default '0',
  `wanderradius` smallint(6) NOT NULL default '0',
  `wandertype` smallint(3) NOT NULL default '0',
  `wanderx1` smallint(6) NOT NULL default '0',
  `wanderx2` smallint(6) NOT NULL default '0',
  `wandery1` smallint(6) NOT NULL default '0',
  `wandery2` smallint(6) NOT NULL default '0',
  PRIMARY KEY (`serial`)
);

CREATE TABLE `players` (
  `account` varchar(255) default NULL,
  `additionalflags` int(10)  NOT NULL default '0',
  `fixedlight` tinyint(3)  NOT NULL default '0',
  `profile` longtext,
  `serial` int(11) NOT NULL default '0',
  `visualrange` tinyint(3)  NOT NULL default '0',
  PRIMARY KEY (`serial`)
);

CREATE TABLE `skills` (
  `cap` smallint(6) default '0',
  `locktype` tinyint(4) default '0',
  `serial` int(11) NOT NULL default '0',
  `skill` tinyint(3)  NOT NULL default '0',
  `value` smallint(6) NOT NULL default '0',
  PRIMARY KEY (`serial`,`skill`)
);

CREATE TABLE `tags` (
  `name` varchar(64) NOT NULL default '',
  `serial` int(11) NOT NULL default '0',
  `type` varchar(6) NOT NULL default '',
  `value` longtext NOT NULL,
  PRIMARY KEY (`serial`,`name`)
);

CREATE TABLE `uobject` (
  `bindmenu` varchar(255) default NULL,
  `direction` char(1) NOT NULL default '0',
  `events` varchar(255) default NULL,
  `havetags` tinyint(1) NOT NULL default '0',
  `multis` int(11) NOT NULL default '-1',
  `name` varchar(255) default NULL,
  `pos_map` tinyint(4) NOT NULL default '0',
  `pos_x` smallint(6)  NOT NULL default '0',
  `pos_y` smallint(6)  NOT NULL default '0',
  `pos_z` smallint(6) NOT NULL default '0',
  `serial` int(11) NOT NULL default '0',
  PRIMARY KEY (`serial`)
);

CREATE TABLE `uobjectmap` (
  `serial` int(11) NOT NULL default '0',
  `type` varchar(80)  NOT NULL default '',
  PRIMARY KEY (`serial`)
);

CREATE TABLE `effects` (
  `destination` int(11) NOT NULL default '-1',
  `dispellable` tinyint(4) NOT NULL default '0',
  `expiretime` int(11) NOT NULL default '0',
  `id` int(11) NOT NULL default '0',
  `objectid` varchar(64) NOT NULL default '',
  `source` int(11) NOT NULL default '-1',
  PRIMARY KEY  (`id`)
);

CREATE TABLE `effects_properties` (
 `id` int(11) NOT NULL default '0',
 `keyname` varchar(64) NOT NULL default '',
 `type` varchar(64) NOT NULL default '',
 `value` text NOT NULL,
  PRIMARY KEY  (`id`,`keyname`)
);


