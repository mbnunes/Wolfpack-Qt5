CREATE TABLE `accounts` (
  `login` varchar(255) NOT NULL default '',
  `PASSWORD` varchar(255) NOT NULL default '',
  `flags` int(11) NOT NULL default '0',
  `acl` varchar(255) NOT NULL default 'player',
  `lastlogin` int(11) NOT NULL default '0',
  `blockuntil` int(11) NOT NULL default '0',
  PRIMARY KEY  (`login`),
  UNIQUE KEY `login` (`login`)
) TYPE=MyISAM;
