# phpMyAdmin SQL Dump
# version 2.5.3
# http://www.phpmyadmin.net
#
# Host: localhost
# Generation Time: Apr 06, 2004 at 10:15 PM
# Server version: 4.0.17
# PHP Version: 4.3.3
# 
# Database : `wpdev`
# 

# --------------------------------------------------------

#
# Table structure for table `documentation_commands`
#

CREATE TABLE `documentation_commands` (
  `name` varchar(32) NOT NULL default '',
  `description` varchar(255) NOT NULL default '',
  `usage` longtext NOT NULL,
  `notes` longtext NOT NULL,
  UNIQUE KEY `command` (`name`)
) TYPE=MyISAM;

# --------------------------------------------------------

#
# Table structure for table `documentation_events`
#

CREATE TABLE `documentation_events` (
  `name` varchar(255) NOT NULL default '',
  `prototype` longtext NOT NULL,
  `parameters` longtext NOT NULL,
  `returnvalue` longtext NOT NULL,
  `callcondition` longtext NOT NULL,
  `notes` longtext NOT NULL,
  UNIQUE KEY `event` (`name`)
) TYPE=MyISAM;

# --------------------------------------------------------

#
# Table structure for table `documentation_objects`
#

CREATE TABLE `documentation_objects` (
  `object` varchar(64) NOT NULL default '',
  `description` longtext NOT NULL,
  PRIMARY KEY  (`object`)
) TYPE=MyISAM;

# --------------------------------------------------------

#
# Table structure for table `documentation_objects_methods`
#

CREATE TABLE `documentation_objects_methods` (
  `object` varchar(64) NOT NULL default '',
  `method` varchar(64) NOT NULL default '',
  `prototype` varchar(255) NOT NULL default '',
  `parameters` longtext NOT NULL,
  `returnvalue` longtext NOT NULL,
  `description` longtext NOT NULL
) TYPE=MyISAM;

# --------------------------------------------------------

#
# Table structure for table `documentation_objects_properties`
#

CREATE TABLE `documentation_objects_properties` (
  `object` varchar(64) NOT NULL default '',
  `property` varchar(64) NOT NULL default '',
  `description` longtext NOT NULL,
  `readonly` tinyint(1) NOT NULL default '0'
) TYPE=MyISAM;

# --------------------------------------------------------

#
# Table structure for table `documentation_settings`
#

CREATE TABLE `documentation_settings` (
  `key` varchar(255) NOT NULL default '',
  `value` varchar(255) NOT NULL default '',
  UNIQUE KEY `key` (`key`)
) TYPE=MyISAM;
    