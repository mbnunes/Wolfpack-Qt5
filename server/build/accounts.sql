CREATE TABLE `accounts` (
	`login` varchar(16) NOT NULL default '',
	`password` varchar(32) NOT NULL default '',
	`flags` int NOT NULL default '0',
	`acl` varchar(255) NOT NULL default 'player',
	`lastlogin` int NOT NULL default '0',
	`blockuntil` int NOT NULL default '0',
	PRIMARY KEY  (`login`)
);
