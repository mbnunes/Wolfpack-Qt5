CREATE TABLE accounts (
	login varchar(255) NOT NULL default '',
	password varchar(255) NOT NULL default '',
	flags int NOT NULL default '0',
	acl varchar(255) NOT NULL default 'player',
	lastlogin int NOT NULL default '',
	blockuntil int NOT NULL default '',
	PRIMARY KEY (login)
);
