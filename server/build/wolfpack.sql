# phpMyAdmin MySQL-Dump
# version 2.3.2
# http://www.phpmyadmin.net/ (download page)
#
# Servidor: localhost
# Tempo de Generação: Out 22, 2002 at 03:42 PM
# Versão do Servidor: 4.00.01
# Versão do PHP: 4.2.2
# Banco de Dados : `wolfpack`
# --------------------------------------------------------


#
# Estrutura da tabela `books`
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
# Estrutura da tabela `characters`
#

CREATE TABLE characters (
  serial int(11) NOT NULL default '0',
  name varchar(255) default NULL,
  title varchar(255) default NULL,
  account varchar(255) default NULL,
  creationday int(11) NOT NULL default '0',
  gmmoveeff int(11) NOT NULL default '0',
  guildtype tinyint(3) unsigned NOT NULL default '0',
  guildtraitor tinyint(1) unsigned NOT NULL default '0',
  cell smallint(6) NOT NULL default '0',
  dir char(1) NOT NULL default '',
  body smallint(5) unsigned NOT NULL default '0',
  xbody smallint(5) unsigned NOT NULL default '0',
  skin smallint(5) unsigned NOT NULL default '0',
  xskin smallint(5) unsigned NOT NULL default '0',
  priv tinyint(3) unsigned NOT NULL default '0',
  stablemaster int(11) NOT NULL default '0',
  npctype tinyint(3) unsigned NOT NULL default '0',
  time_unused int(10) unsigned NOT NULL default '0',
  allmove tinyint(3) unsigned NOT NULL default '0',
  font tinyint(3) unsigned NOT NULL default '0',
  emote smallint(5) unsigned NOT NULL default '0',
  strength smallint(6) NOT NULL default '0',
  strength2 smallint(6) NOT NULL default '0',
  dexterity smallint(6) NOT NULL default '0',
  dexterity2 smallint(6) NOT NULL default '0',
  intelligence smallint(6) NOT NULL default '0',
  intelligence2 smallint(6) NOT NULL default '0',
  hitpoints smallint(6) NOT NULL default '0',
  spawnregion varchar(255) default NULL,
  stamina smallint(6) NOT NULL default '0',
  npc tinyint(1) NOT NULL default '0',
  holdgold int(10) unsigned NOT NULL default '0',
  shop tinyint(1) NOT NULL default '0',
  own int(11) NOT NULL default '-1',
  robe int(11) NOT NULL default '0',
  karma int(11) NOT NULL default '0',
  fame int(11) NOT NULL default '0',
  kills int(10) unsigned NOT NULL default '0',
  deaths int(10) unsigned NOT NULL default '0',
  dead tinyint(1) NOT NULL default '0',
  packitem int(11) NOT NULL default '0',
  fixedlight tinyint(3) unsigned NOT NULL default '0',
  speech tinyint(3) unsigned NOT NULL default '0',
  disablemsg varchar(255) default NULL,
  cantrain tinyint(1) NOT NULL default '0',
  def int(10) unsigned NOT NULL default '0',
  lodamage int(11) NOT NULL default '0',
  hidamage int(11) NOT NULL default '0',
  war tinyint(1) NOT NULL default '0',
  npcwander tinyint(3) unsigned NOT NULL default '0',
  oldnpcwander tinyint(3) unsigned NOT NULL default '0',
  carve varchar(255) default NULL,
  fx1 int(11) NOT NULL default '0',
  fy1 int(11) NOT NULL default '0',
  fz1 int(11) NOT NULL default '0',
  fx2 int(11) NOT NULL default '0',
  fy2 int(11) NOT NULL default '0',
  spawn int(11) NOT NULL default '-1',
  hidden tinyint(3) unsigned NOT NULL default '0',
  hunger int(11) NOT NULL default '0',
  npcaitype int(11) NOT NULL default '0',
  spattack int(11) NOT NULL default '0',
  spadelay int(11) NOT NULL default '0',
  taming int(11) NOT NULL default '0',
  summontimer int(10) unsigned NOT NULL default '0',
  summonremainingseconds smallint(5) unsigned NOT NULL default '0',
  advobj int(11) NOT NULL default '0',
  poison int(11) NOT NULL default '0',
  poisoned int(10) unsigned NOT NULL default '0',
  fleeat smallint(6) NOT NULL default '0',
  reattackat smallint(6) NOT NULL default '0',
  split tinyint(3) unsigned NOT NULL default '0',
  splitchance tinyint(3) unsigned NOT NULL default '0',
  guildtoggle tinyint(1) unsigned NOT NULL default '0',
  guildstone int(11) NOT NULL default '0',
  guildtitle varchar(255) default NULL,
  guildfealty int(11) NOT NULL default '0',
  murderrate int(10) unsigned NOT NULL default '0',
  menupriv int(11) NOT NULL default '0',
  questtype int(11) NOT NULL default '0',
  questdestregion int(11) NOT NULL default '0',
  questorigregion int(11) NOT NULL default '0',
  questbountypostserial int(11) NOT NULL default '0',
  questbountyreward int(11) NOT NULL default '0',
  jailtimer int(10) unsigned NOT NULL default '0',
  jailsecs int(11) NOT NULL default '0',
  lootlist varchar(255) default NULL,
  food int(10) unsigned NOT NULL default '0',
  say smallint(5) unsigned default NULL,
  mana smallint(6) default NULL,
  PRIMARY KEY  (serial),
  UNIQUE KEY serial (serial)
) TYPE=MyISAM COMMENT='cChar';
# --------------------------------------------------------

#
# Estrutura da tabela `items`
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
  layer tinyint(3) NOT NULL default '0',
  type int(11) NOT NULL default '0',
  type2 int(11) NOT NULL default '0',
  offspell int(11) NOT NULL default '0',
  more1 tinyint(3) unsigned NOT NULL default '0',
  more2 tinyint(3) unsigned NOT NULL default '0',
  more3 tinyint(3) unsigned NOT NULL default '0',
  more4 tinyint(3) unsigned NOT NULL default '0',
  moreb1 tinyint(3) unsigned NOT NULL default '0',
  moreb2 tinyint(3) unsigned NOT NULL default '0',
  moreb3 tinyint(3) unsigned NOT NULL default '0',
  moreb4 tinyint(3) unsigned NOT NULL default '0',
  morex int(11) NOT NULL default '0',
  morey int(11) NOT NULL default '0',
  morez int(11) NOT NULL default '0',
  amount int(11) NOT NULL default '0',
  doordir tinyint(3) unsigned NOT NULL default '0',
  dye int(11) NOT NULL default '0',
  decaytime int(11) NOT NULL default '0',
  att int(11) NOT NULL default '0',
  def int(11) NOT NULL default '0',
  hidamage smallint(6) NOT NULL default '0',
  lodamage smallint(6) NOT NULL default '0',
  st smallint(6) NOT NULL default '0',
  time_unused int(11) NOT NULL default '0',
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
  poisoned int(11) unsigned NOT NULL default '0',
  magic int(11) NOT NULL default '0',
  owner int(11) NOT NULL default '-1',
  visible int(11) NOT NULL default '0',
  spawn int(11) NOT NULL default '-1',
  dir int(11) NOT NULL default '0',
  priv int(11) NOT NULL default '0',
  value int(11) NOT NULL default '0',
  restock int(11) NOT NULL default '0',
  disabled int(11) unsigned NOT NULL default '0',
  spawnregion int(11) NOT NULL default '0',
  good int(11) NOT NULL default '0',
  glow int(11) NOT NULL default '0',
  glow_color smallint(5) unsigned NOT NULL default '0',
  glowtype int(11) NOT NULL default '0',
  `desc` varchar(255) default NULL,
  carve varchar(255) default NULL,
  accuracy int(11) NOT NULL default '0',
  PRIMARY KEY  (serial),
  UNIQUE KEY serial (serial)
) TYPE=MyISAM COMMENT='cItem';
# --------------------------------------------------------

#
# Estrutura da tabela `skills`
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
# Estrutura da tabela `uobject`
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
  PRIMARY KEY  (serial),
  UNIQUE KEY serial (serial)
) TYPE=MyISAM COMMENT='class cUObject';
# --------------------------------------------------------

#
# Estrutura da tabela `uobjectmap`
#

CREATE TABLE uobjectmap (
  serial int(11) NOT NULL default '0',
  type varchar(80) binary NOT NULL default '',
  PRIMARY KEY  (serial),
  UNIQUE KEY serial (serial)
) TYPE=MyISAM;

CREATE TABLE spellbooks (
  serial int(11) NOT NULL default '0',
  spells1 int(11) UNSIGNED NOT NULL default '0',
  spells2 int(11) UNSIGNED NOT NULL default '0',
  PRIMARY KEY  (serial),
  UNIQUE KEY serial (serial)
) TYPE=MyISAM COMMENT='class cSpellBook';

CREATE TABLE multis_bans (
  serial int(11) NOT NULL default '0',
  ban int(11) NOT NULL default '-1',
  KEY  (serial)
) TYPE=MyISAM COMMENT='Bans of cMulti';

CREATE TABLE multis_friends (
  serial int(11) NOT NULL default '0',
  friend int(11) NOT NULL default '-1',
  KEY  (serial)
) TYPE=MyISAM COMMENT='Friends of cMulti';

CREATE TABLE multis (
  serial int(11) NOT NULL default '0',
  coowner int(11) NOT NULL default '-1',
  deedsection varchar(255) NOT NULL default '',
  PRIMARY KEY  (serial),
  UNIQUE KEY serial (serial)
) TYPE=MyISAM COMMENT='class cMulti';

CREATE TABLE boats_itemoffsets (
  serial int(11) NOT NULL default '0',
  a tinyint(1) UNSIGNED NOT NULL default '0',
  b tinyint(1) UNSIGNED NOT NULL default '0',
  c tinyint(1) UNSIGNED NOT NULL default '0',
  offset smallint(6) default '0',
  KEY (serial)
) TYPE=MyISAM COMMENT='Itemoffsets of cBoat Components';

CREATE TABLE boats_itemids (
  serial int(11) NOT NULL default '0',
  a tinyint(1) UNSIGNED NOT NULL default '0',
  b tinyint(1) UNSIGNED NOT NULL default '0',
  id smallint(6) unsigned default '0',
  KEY (serial)
) TYPE=MyISAM COMMENT='ItemIDs of cBoat Components';

CREATE TABLE boats (
  serial int(11) NOT NULL default '0',
  autosail tinyint(1) UNSIGNED NOT NULL default '0',
  boatdir tinyint(1) UNSIGNED NOT NULL default '0',
  itemserial1 int(11) NOT NULL default '-1',
  itemserial2 int(11) NOT NULL default '-1',
  itemserial3 int(11) NOT NULL default '-1',
  itemserial4 int(11) NOT NULL default '-1',
  multi1 smallint(6) unsigned default '0',
  multi2 smallint(6) unsigned default '0',
  multi3 smallint(6) unsigned default '0',
  multi4 smallint(6) unsigned default '0',
  multi5 smallint(6) unsigned default '0',
  multi6 smallint(6) unsigned default '0',
  multi7 smallint(6) unsigned default '0',
  multi8 smallint(6) unsigned default '0',
  PRIMARY KEY  (serial),
  UNIQUE KEY serial (serial)
) TYPE=MyISAM COMMENT='class cBoat';

CREATE TABLE houses (
  serial int(11) NOT NULL default '0',
  nokey tinyint(1) NOT NULL default '0',
  charpos_x smallint(6) NOT NULL default '0',
  charpos_y smallint(6) NOT NULL default '0',
  charpos_z smallint(6) NOT NULL default '0',
  PRIMARY KEY  (serial),
  UNIQUE KEY serial (serial)
) TYPE=MyISAM COMMENT='class cHouse';

CREATE TABLE guildstones_war (
  serial int(11) NOT NULL default '0',
  enemy int(11) NOT NULL default '-1'
) TYPE=MyISAM COMMENT='Other guildstones this guild is at war with.';

CREATE TABLE guildstones_recruits (
  serial int(11) NOT NULL default '0',
  recruit int(11) NOT NULL default '-1'
) TYPE=MyISAM COMMENT='Recruitees of this guild.';

CREATE TABLE guildstones_members (
  serial int(11) NOT NULL default '0',
  member int(11) NOT NULL default '-1'
) TYPE=MyISAM COMMENT='Members in this guild.';

CREATE TABLE guildstones (
  serial int(11) NOT NULL default '0',
  guildname varchar(255) NOT NULL default '',
  abbreviation varchar(255) NOT NULL default '',
  guildtype tinyint(1) NOT NULL default '0',
  charter longtext NOT NULL default '',
  webpage varchar(255) NOT NULL default 'http://www.wpdev.org',
  master int(11) NOT NULL default '-1',
  priv int(11) NOT NULL default '-1',
  PRIMARY KEY  (serial),
  UNIQUE KEY serial (serial)
) TYPE=MyISAM COMMENT='class cGuildStone';

CREATE TABLE corpses_equipment (
  serial int(11) NOT NULL default '0',
  item int(11) NOT NULL default '-1',
  layer tinyint(3) UNSIGNED NOT NULL default '0'
) TYPE=MyISAM COMMENT='Equipment of corpses.';

CREATE TABLE corpses (
  serial int(11) NOT NULL default '0',
  bodyid smallint(6) UNSIGNED NOT NULL default '0',
  hairstyle smallint(6) UNSIGNED NOT NULL default '0',  
  haircolor smallint(6) UNSIGNED NOT NULL default '0',
  beardstyle smallint(6) UNSIGNED NOT NULL default '0',
  beardcolor smallint(6) UNSIGNED NOT NULL default '0',
  PRIMARY KEY  (serial),
  UNIQUE KEY serial (serial)
) TYPE=MyISAM COMMENT='class cCorpse';

