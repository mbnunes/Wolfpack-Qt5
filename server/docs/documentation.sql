CREATE TABLE `documentation_commands` (
  `name` varchar(32) NOT NULL default '',
  `description` varchar(255) NOT NULL default '',
  `usage` longtext NOT NULL,
  `notes` longtext NOT NULL,
  UNIQUE KEY `command` (`name`)
) TYPE=MyISAM;

CREATE TABLE `documentation_events` (
  `name` varchar(255) NOT NULL default '',
  `prototype` longtext NOT NULL,
  `parameters` longtext NOT NULL,
  `returnvalue` longtext NOT NULL,
  `callcondition` longtext NOT NULL,
  `notes` longtext NOT NULL,
  UNIQUE KEY `event` (`name`)
) TYPE=MyISAM;

CREATE TABLE `documentation_settings` (
  `key` varchar(255) NOT NULL default '',
  `value` varchar(255) NOT NULL default '',
  UNIQUE KEY `key` (`key`)
) TYPE=MyISAM;
    