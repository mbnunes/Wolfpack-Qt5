# phpMyAdmin MySQL-Dump
# version 2.3.0-rc2
# http://phpwizard.net/phpMyAdmin/
# http://www.phpmyadmin.net/ (download page)
#
# Host: localhost:3306
# Generation Time: Jun 11, 2003 at 03:32 PM
# Server version: 4.00.12
# PHP Version: 4.3.1
# Database : `wolfpack`
# --------------------------------------------------------

#
# Table structure for table `boats`
#

CREATE TABLE boats (
  serial int(11) NOT NULL default '0',
  autosail tinyint(1) unsigned NOT NULL default '0',
  boatdir tinyint(1) unsigned NOT NULL default '0',
  itemserial1 int(11) NOT NULL default '-1',
  itemserial2 int(11) NOT NULL default '-1',
  itemserial3 int(11) NOT NULL default '-1',
  itemserial4 int(11) NOT NULL default '-1',
  multi1 smallint(6) unsigned default '0',
  multi2 smallint(6) unsigned default '0',
  multi3 smallint(6) unsigned default '0',
  multi4 smallint(6) unsigned default '0',
  PRIMARY KEY  (serial),
  UNIQUE KEY serial (serial)
) TYPE=MyISAM COMMENT='class cBoat';
# --------------------------------------------------------

#
# Table structure for table `boats_itemids`
#

CREATE TABLE boats_itemids (
  serial int(11) NOT NULL default '0',
  a tinyint(1) unsigned NOT NULL default '0',
  b tinyint(1) unsigned NOT NULL default '0',
  id smallint(6) unsigned default '0',
  KEY serial (serial)
) TYPE=MyISAM COMMENT='ItemIDs of cBoat Components';
# --------------------------------------------------------

#
# Table structure for table `boats_itemoffsets`
#

CREATE TABLE boats_itemoffsets (
  serial int(11) NOT NULL default '0',
  a tinyint(1) unsigned NOT NULL default '0',
  b tinyint(1) unsigned NOT NULL default '0',
  c tinyint(1) unsigned NOT NULL default '0',
  offset smallint(6) default '0',
  KEY serial (serial)
) TYPE=MyISAM COMMENT='Itemoffsets of cBoat Components';
# --------------------------------------------------------

#
# Table structure for table `bookpages`
#

CREATE TABLE bookpages (
  serial int(11) NOT NULL default '0',
  page tinyint(3) unsigned NOT NULL default '0',
  text varchar(255) binary NOT NULL default '',
  KEY page (page),
  KEY serial (serial)
) TYPE=MyISAM;
# --------------------------------------------------------

#
# Table structure for table `books`
#

CREATE TABLE books (
  serial int(11) NOT NULL default '0',
  title varchar(255) binary default NULL,
  author varchar(255) binary NOT NULL default '',
  readonly tinyint(1) NOT NULL default '0',
  predefined tinyint(1) NOT NULL default '0',
  section varchar(255) binary NOT NULL default '',
  pages smallint(5) unsigned NOT NULL default '0',
  PRIMARY KEY  (serial),
  UNIQUE KEY serial (serial)
) TYPE=MyISAM COMMENT='cBook class';
# --------------------------------------------------------

#
# Table structure for table `characters`
#

CREATE TABLE characters (
  serial int(11) NOT NULL default '0',
  name varchar(255) default NULL,
  title varchar(255) default NULL,
  creationdate varchar(255) default NULL,
  dir char(1) NOT NULL default '',
  body smallint(5) unsigned NOT NULL default '0',
  orgbody smallint(5) unsigned NOT NULL default '0',
  skin smallint(5) unsigned NOT NULL default '0',
  orgskin smallint(5) unsigned NOT NULL default '0',
  saycolor smallint(5) unsigned NOT NULL default '0',
  emotecolor smallint(5) unsigned NOT NULL default '0',
  strength smallint(6) NOT NULL default '0',
  strengthmod smallint(6) NOT NULL default '0',
  dexterity smallint(6) NOT NULL default '0',
  dexteritymod smallint(6) NOT NULL default '0',
  intelligence smallint(6) NOT NULL default '0',
  intelligencemod smallint(6) NOT NULL default '0',
  maxhitpoints smallint(6) NOT NULL default '0',
  hitpoints smallint(6) NOT NULL default '0',
  maxstamina smallint(6) NOT NULL default '0',
  stamina smallint(6) NOT NULL default '0',
  maxmana smallint(6) default NULL,
  mana smallint(6) default NULL,
  karma int(11) NOT NULL default '0',
  fame int(11) NOT NULL default '0',
  kills int(10) unsigned NOT NULL default '0',
  deaths int(10) unsigned NOT NULL default '0',
  def int(10) unsigned NOT NULL default '0',
  hunger int(11) NOT NULL default '0',
  poison int(11) NOT NULL default '0',
  poisoned int(10) unsigned NOT NULL default '0',
  murderertime int(11) unsigned NOT NULL default '0',
  criminaltime int(11) unsigned NOT NULL default '0',
  nutriment int(10) unsigned NOT NULL default '0',
  stealthsteps int(11) unsigned NOT NULL default '0',
  gender tinyint(1) unsigned NOT NULL default '0',
  propertyflags int(11) unsigned NOT NULL default '0',
  attacker int(11) NOT NULL default '-1',
  combattarget int(11) NOT NULL default '-1',
  murderer int(11) NOT NULL default '-1',
  guarding int(11) NOT NULL default '-1',
  PRIMARY KEY  (serial),
  UNIQUE KEY serial (serial)
) TYPE=MyISAM COMMENT='cBaseChar';
# --------------------------------------------------------

#
# Table structure for table `corpses`
#

CREATE TABLE corpses (
  serial int(11) NOT NULL default '0',
  bodyid smallint(6) unsigned NOT NULL default '0',
  hairstyle smallint(6) unsigned NOT NULL default '0',
  haircolor smallint(6) unsigned NOT NULL default '0',
  beardstyle smallint(6) unsigned NOT NULL default '0',
  beardcolor smallint(6) unsigned NOT NULL default '0',
  PRIMARY KEY  (serial),
  UNIQUE KEY serial (serial)
) TYPE=MyISAM COMMENT='class cCorpse';
# --------------------------------------------------------

#
# Table structure for table `corpses_equipment`
#

CREATE TABLE corpses_equipment (
  serial int(11) NOT NULL default '0',
  item int(11) NOT NULL default '-1',
  layer tinyint(3) unsigned NOT NULL default '0'
) TYPE=MyISAM COMMENT='Equipment of corpses.';
# --------------------------------------------------------

#
# Table structure for table `guildstones`
#

CREATE TABLE guildstones (
  serial int(11) NOT NULL default '0',
  guildname varchar(255) NOT NULL default '',
  abbreviation varchar(255) NOT NULL default '',
  guildtype tinyint(1) NOT NULL default '0',
  charter longtext NOT NULL,
  webpage varchar(255) NOT NULL default 'http://www.wpdev.org',
  master int(11) NOT NULL default '-1',
  priv int(11) NOT NULL default '-1',
  PRIMARY KEY  (serial),
  UNIQUE KEY serial (serial)
) TYPE=MyISAM COMMENT='class cGuildStone';
# --------------------------------------------------------

#
# Table structure for table `guildstones_members`
#

CREATE TABLE guildstones_members (
  serial int(11) NOT NULL default '0',
  member int(11) NOT NULL default '-1'
) TYPE=MyISAM COMMENT='Members in this guild.';
# --------------------------------------------------------

#
# Table structure for table `guildstones_recruits`
#

CREATE TABLE guildstones_recruits (
  serial int(11) NOT NULL default '0',
  recruit int(11) NOT NULL default '-1'
) TYPE=MyISAM COMMENT='Recruitees of this guild.';
# --------------------------------------------------------

#
# Table structure for table `guildstones_war`
#

CREATE TABLE guildstones_war (
  serial int(11) NOT NULL default '0',
  enemy int(11) NOT NULL default '-1'
) TYPE=MyISAM COMMENT='Other guildstones this guild is at war with.';
# --------------------------------------------------------

#
# Table structure for table `houses`
#

CREATE TABLE houses (
  serial int(11) NOT NULL default '0',
  nokey tinyint(1) NOT NULL default '0',
  charpos_x smallint(6) NOT NULL default '0',
  charpos_y smallint(6) NOT NULL default '0',
  charpos_z smallint(6) NOT NULL default '0',
  PRIMARY KEY  (serial),
  UNIQUE KEY serial (serial)
) TYPE=MyISAM COMMENT='class cHouse';
# --------------------------------------------------------

#
# Table structure for table `items`
#

CREATE TABLE items (
  serial int(11) NOT NULL default '0',
  id smallint(5) unsigned NOT NULL default '0',
  name varchar(255) binary default NULL,
  name2 varchar(255) binary default NULL,
  creator varchar(255) binary default NULL,
  sk_name tinyint(3) unsigned NOT NULL default '0',
  color smallint(5) unsigned NOT NULL default '0',
  cont int(11) NOT NULL default '-1',
  layer tinyint(3) unsigned NOT NULL default '0',
  type smallint(5) unsigned NOT NULL default '0',
  type2 smallint(5) unsigned NOT NULL default '0',
  offspell tinyint(3) unsigned NOT NULL default '0',
  more1 tinyint(3) unsigned NOT NULL default '0',
  more2 tinyint(3) unsigned NOT NULL default '0',
  more3 tinyint(3) unsigned NOT NULL default '0',
  more4 tinyint(3) unsigned NOT NULL default '0',
  moreb1 tinyint(3) unsigned NOT NULL default '0',
  moreb2 tinyint(3) unsigned NOT NULL default '0',
  moreb3 tinyint(3) unsigned NOT NULL default '0',
  moreb4 tinyint(3) unsigned NOT NULL default '0',
  morex int(10) unsigned NOT NULL default '0',
  morey int(10) unsigned NOT NULL default '0',
  morez int(10) unsigned NOT NULL default '0',
  amount smallint(5) unsigned NOT NULL default '0',
  doordir tinyint(3) unsigned NOT NULL default '0',
  dye tinyint(3) unsigned NOT NULL default '0',
  decaytime int(10) unsigned NOT NULL default '0',
  att int(10) unsigned NOT NULL default '0',
  def int(10) unsigned NOT NULL default '0',
  hidamage smallint(6) NOT NULL default '0',
  lodamage smallint(6) NOT NULL default '0',
  st smallint(6) NOT NULL default '0',
  time_unused int(10) unsigned NOT NULL default '0',
  weight int(11) NOT NULL default '0',
  hp smallint(6) NOT NULL default '0',
  maxhp smallint(6) NOT NULL default '0',
  rank int(11) NOT NULL default '0',
  st2 smallint(6) NOT NULL default '0',
  dx smallint(6) NOT NULL default '0',
  dx2 smallint(6) NOT NULL default '0',
  intelligence smallint(6) NOT NULL default '0',
  intelligence2 smallint(6) NOT NULL default '0',
  speed int(11) NOT NULL default '0',
  poisoned int(10) unsigned NOT NULL default '0',
  magic tinyint(3) unsigned NOT NULL default '0',
  owner int(11) NOT NULL default '-1',
  visible tinyint(3) unsigned NOT NULL default '0',
  spawn int(11) NOT NULL default '-1',
  dir tinyint(3) unsigned NOT NULL default '0',
  priv tinyint(3) unsigned NOT NULL default '0',
  sellprice int(11) NOT NULL default '0',
  buyprice int(11) NOT NULL default '0',
  price int(11) NOT NULL default '0',
  restock smallint(5) unsigned NOT NULL default '0',
  disabled int(10) unsigned NOT NULL default '0',
  spawnregion varchar(255) binary default NULL,
  good int(11) NOT NULL default '0',
  description varchar(255) default NULL,
  carve varchar(255) default NULL,
  accuracy smallint(5) unsigned NOT NULL default '0',
  PRIMARY KEY  (serial),
  UNIQUE KEY serial (serial)
) TYPE=MyISAM COMMENT='cItem';
# --------------------------------------------------------

#
# Table structure for table `multis`
#

CREATE TABLE multis (
  serial int(11) NOT NULL default '0',
  coowner int(11) NOT NULL default '-1',
  deedsection varchar(255) NOT NULL default '',
  PRIMARY KEY  (serial),
  UNIQUE KEY serial (serial)
) TYPE=MyISAM COMMENT='class cMulti';
# --------------------------------------------------------

#
# Table structure for table `multis_bans`
#

CREATE TABLE multis_bans (
  serial int(11) NOT NULL default '0',
  ban int(11) NOT NULL default '-1',
  KEY serial (serial)
) TYPE=MyISAM COMMENT='Bans of cMulti';
# --------------------------------------------------------

#
# Table structure for table `multis_friends`
#

CREATE TABLE multis_friends (
  serial int(11) NOT NULL default '0',
  friend int(11) NOT NULL default '-1',
  KEY serial (serial)
) TYPE=MyISAM COMMENT='Friends of cMulti';
# --------------------------------------------------------

#
# Table structure for table `npcs`
#

CREATE TABLE npcs (
  serial int(11) NOT NULL default '0',
  mindamage smallint(6) unsigned NOT NULL default '0',
  maxdamage smallint(6) unsigned NOT NULL default '0',
  tamingminskill smallint(6) unsigned NOT NULL default '0',
  summontime int(11) unsigned NOT NULL default '0',
  additionalflags int(11) unsigned NOT NULL default '0',
  owner int(11) NOT NULL default '-1',
  carve varchar(255) default NULL,
  spawnregion varchar(255) default NULL,
  stablemaster int(11) NOT NULL default '-1',
  lootlist varchar(255) default NULL,
  ai varchar(255) default NULL,
  PRIMARY KEY  (serial),
  UNIQUE KEY serial (serial)
) TYPE=MyISAM COMMENT='cNPC';
# --------------------------------------------------------

#
# Table structure for table `pages`
#

CREATE TABLE pages (
  charserial int(11) NOT NULL default '-1',
  pagetype tinyint(4) NOT NULL default '0',
  pagetime varchar(64) NOT NULL default '',
  pagepos varchar(32) NOT NULL default '0,0,0,0',
  content mediumtext NOT NULL,
  category int(11) NOT NULL default '0',
  pageorder int(11) NOT NULL default '0'
) TYPE=MyISAM COMMENT='class cPage';
# --------------------------------------------------------

#
# Table structure for table `players`
#

CREATE TABLE players (
  serial int(11) NOT NULL default '0',
  account varchar(255) default NULL,
  additionalflags int(10) unsigned NOT NULL default '0',
  visualrange tinyint(3) unsigned NOT NULL default '0',
  profile longtext,
  fixedlight tinyint(3) unsigned NOT NULL default '0',
  PRIMARY KEY  (serial),
  UNIQUE KEY serial (serial)
) TYPE=MyISAM COMMENT='cPlayer';
# --------------------------------------------------------

#
# Table structure for table `skills`
#

CREATE TABLE skills (
  serial int(11) NOT NULL default '0',
  skill tinyint(3) unsigned NOT NULL default '0',
  value smallint(6) NOT NULL default '0',
  locktype tinyint(4) default '0',
  cap smallint(6) default '0',
  KEY skill (skill),
  KEY serial (serial)
) TYPE=MyISAM PACK_KEYS=0 COMMENT='Skills from characters';
# --------------------------------------------------------

#
# Table structure for table `spellbooks`
#

CREATE TABLE spellbooks (
  serial int(11) NOT NULL default '0',
  spells1 int(11) unsigned NOT NULL default '0',
  spells2 int(11) unsigned NOT NULL default '0',
  PRIMARY KEY  (serial),
  UNIQUE KEY serial (serial)
) TYPE=MyISAM COMMENT='class cSpellBook';
# --------------------------------------------------------

#
# Table structure for table `tags`
#

CREATE TABLE tags (
  serial int(11) NOT NULL default '0',
  name varchar(64) NOT NULL default '',
  type varchar(6) NOT NULL default '',
  value longtext NOT NULL
) TYPE=MyISAM COMMENT='class cCustomTags';
# --------------------------------------------------------

#
# Table structure for table `uobject`
#

CREATE TABLE uobject (
  name varchar(255) default NULL,
  serial int(11) NOT NULL default '0',
  multis int(11) NOT NULL default '-1',
  pos_x smallint(6) unsigned NOT NULL default '0',
  pos_y smallint(6) unsigned NOT NULL default '0',
  pos_z smallint(6) NOT NULL default '0',
  pos_map tinyint(4) NOT NULL default '0',
  events varchar(255) default NULL,
  bindmenu varchar(255) default NULL,
  havetags tinyint(1) NOT NULL default '0',
  PRIMARY KEY  (serial),
  UNIQUE KEY serial (serial)
) TYPE=MyISAM COMMENT='class cUObject';
# --------------------------------------------------------

#
# Table structure for table `uobjectmap`
#

CREATE TABLE uobjectmap (
  serial int(11) NOT NULL default '0',
  type varchar(80) binary NOT NULL default '',
  PRIMARY KEY  (serial),
  UNIQUE KEY serial (serial)
) TYPE=MyISAM;

    