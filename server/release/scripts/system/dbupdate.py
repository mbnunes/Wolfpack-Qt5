
import wolfpack
from wolfpack.consts import *
from wolfpack import console, database
from wolfpack.database import WORLD, ACCOUNTS

#
# onLoad
#
def onLoad():
	wolfpack.registerglobal(EVENT_UPDATEACCTDATABASE, 'system.dbupdate')
	wolfpack.registerglobal(EVENT_UPDATEDATABASE, 'system.dbupdate')

#
# Null update procedure, should be used for version changes that
# don't require any updating action
#
def null_update_procedure():
    return True


#########################################################################################
#######################   World DB Update   #############################################
#########################################################################################

#
# MySQL: Update Database Procedures
# Naming convention: mysql_update_%current version%() - will update to %current version% + 1
#
def mysql_update_7():
	sql = "ALTER TABLE players ADD `maxcontrolslots` tinyint(4) NOT NULL default '5' AFTER intlock;"
	database.execute(sql)

	return True

def mysql_update_10():
	# Create new guild tables

	sql = "CREATE TABLE `guilds_enemies` ( `guild` int(10) unsigned NOT NULL default '0', `enemy` int(10) unsigned NOT NULL default '0', PRIMARY KEY(`guild`,`enemy`)) TYPE=MYISAM CHARACTER SET utf8;"
	database.execute(sql)

	sql = "CREATE TABLE `guilds_allies` ( `guild` int(10) unsigned NOT NULL default '0', `ally` int(10) unsigned NOT NULL default '0', PRIMARY KEY(`guild`,`ally`)) TYPE=MYISAM CHARACTER SET utf8;"
	database.execute(sql)

	return True

def mysql_update_12():
	# change 'priv' from signed to unsigned
	sql = "ALTER TABLE `items` MODIFY `priv` tinyint(3) unsigned NOT NULL default '0';"
	database.execute(sql)

	return True

#
# SQLite Database Updates
# Naming convention: sqlite_update_%current version%() - will update to %current version% + 1
#

def sqlite_update_8():

	sql = "CREATE TABLE tmp_players7 ( serial bigint, account varchar(16), additionalflags bigint, visualrange tinyint(3), profile longtext, fixedlight tinyint(3), strlock smallint, dexlock smallint, intlock smallint);"
	database.execute(sql)
	sql = "insert into tmp_players7 select * from players;"
	database.execute(sql)
	sql = "drop table players;"
	database.execute(sql)

	sql = "CREATE TABLE players ( serial bigint NOT NULL default '0', account varchar(16) default NULL, additionalflags bigint NOT NULL default '0', visualrange tinyint(3) NOT NULL default '0', profile longtext, fixedlight tinyint(3) NOT NULL default '0',	strlock smallint NOT NULL default '0', dexlock smallint NOT NULL default '0', intlock smallint NOT NULL default '0', maxcontrolslots tinyint(4) NOT NULL default '5', PRIMARY KEY  (serial) );"
	database.execute(sql)

	sql = "insert into players select *, 5 from tmp_players7"
	database.execute(sql)
	sql = "drop table tmp_players7;"
	database.execute(sql)

	# optimize
	sql = "VACUUM"
	database.execute(sql)

	return True

def sqlite_update_11():
        return True

def sqlite_update_12():
	sql = "CREATE TABLE tmp_items12 (serial unsigned int(10) NOT NULL default '0',id unsigned smallint(5) NOT NULL default '0',color unsigned smallint(5) NOT NULL default '0',cont unsigned int(10) NOT NULL default '0',layer unsigned tinyint(3) NOT NULL default '0',amount smallint(5)  NOT NULL default '0',hp smallint(6) NOT NULL default '0',maxhp smallint(6) NOT NULL default '0',movable tinyint(3)  NOT NULL default '0',owner unsigned int(10) NOT NULL default '0',visible tinyint(3)  NOT NULL default '0',priv tinyint(3)  NOT NULL default '0',baseid varchar(64) NOT NULL default '',PRIMARY KEY (serial));"
	database.execute(sql)
	sql = "insert into tmp_items12 select * from items;"
	database.execute(sql)
	sql = "drop table items;"
	database.execute(sql)

	sql = "CREATE TABLE items (serial unsigned int(10) NOT NULL default '0',id unsigned smallint(5) NOT NULL default '0',color unsigned smallint(5) NOT NULL default '0',cont unsigned int(10) NOT NULL default '0',layer unsigned tinyint(3) NOT NULL default '0',amount smallint(5)  NOT NULL default '0',hp smallint(6) NOT NULL default '0',maxhp smallint(6) NOT NULL default '0',movable tinyint(3)  NOT NULL default '0',owner unsigned int(10) NOT NULL default '0',visible tinyint(3)  NOT NULL default '0',priv unsigned tinyint(3)  NOT NULL default '0',baseid varchar(64) NOT NULL default '',PRIMARY KEY (serial));"
	database.execute(sql)

	sql = "insert into items select * from tmp_items12"
	database.execute(sql)
	sql = "drop table tmp_items12;"
	database.execute(sql)

	# optimize
	sql = "VACUUM"
	database.execute(sql)

	return True

# MySQL and Sqlite version update arrays
MYSQL_UPDATES = {
	7: mysql_update_7,
        8: null_update_procedure,
	9: mysql_update_10,
	10: null_update_procedure,
        11: null_update_procedure,
        12: mysql_update_12,
}

SQLITE_UPDATES = {
	7: sqlite_update_8,
	8: null_update_procedure,# ???? What the hell happened to 9 ????
	9: null_update_procedure,
	10: null_update_procedure,
        11: sqlite_update_11,
        12: sqlite_update_12,
}

#
# Database update event
#
def onUpdateDatabase(current, version):
	# Find the update table for the database driver in use
	driver = database.driver(WORLD)
	if driver == 'mysql':
		updates = MYSQL_UPDATES
	elif driver == 'sqlite':
		updates = SQLITE_UPDATES
	else:
		console.log(LOG_ERROR, "Unknown database driver: %s.\n" % driver)
		return False

	for i in range(version, current):
		# No update for this version available
		if not updates.has_key(i):
			console.log(LOG_ERROR, "No update available for database version %u.\n" % i)
			return False

		console.log(LOG_MESSAGE, "Updating database from version %u to %u.\n" % (i, i+1))

		try:
			if not updates[i]():
				return False
		except Exception, e:
			console.log(LOG_ERROR, str(e) + "\n")
			return False

		wolfpack.setoption('db_version', str(i + 1))
		try:
			if driver == 'mysql':
				database.execute("REPLACE INTO `settings` VALUES('db_version', '%u');" % (i + 1))
			else:
				database.execute("REPLACE INTO settings VALUES('db_version', '%u');" % (i + 1))
		except Exception, e:
			console.log(LOG_WARNING, "Unable to update database version to %u:\n%s\n" % (i + 1, str(e)))

	return True

#########################################################################################
#######################   Account DB Update   ###########################################
#########################################################################################

#
# Update Version
#
def updateacctversion(version, driver):
	try:
		if driver == 'mysql':
			database.execute("REPLACE INTO `settings` VALUES('db_version', '%u');" % version)
		else:
			database.execute("REPLACE INTO settings VALUES('db_version', '%u');" % version)
	except Exception, e:
		console.log(LOG_WARNING, "Unable to update account database version to %u:\n%s\n" % (i + 1, str(e)))

#
# MySQL
#
def acct_mysql_update_1():
	sql = "ALTER TABLE accounts ADD creationdate varchar(19) default NULL AFTER email;"
	database.execute(sql)

	sql = "ALTER TABLE accounts ADD totalgametime int NOT NULL default '0' AFTER creationdate;"
	database.execute(sql)

	sql = "ALTER TABLE accounts ADD slots smallint(5) NOT NULL default '1' AFTER totalgametime;"
	database.execute(sql)

	return True

#
# SQLite
#
def acct_sqlite_update_1():
	sql = "ALTER TABLE accounts ADD creationdate varchar(19) default NULL;"
	database.execute(sql)

	sql = "ALTER TABLE accounts ADD totalgametime int NOT NULL default '0';"
	database.execute(sql)

	sql = "ALTER TABLE accounts ADD slots smallint(5) NOT NULL default '1';"
	database.execute(sql)

	return True

# MySQL and Sqlite version update arrays
ACCT_MYSQL_UPDATES = {
	0: acct_mysql_update_1,
}

ACCT_SQLITE_UPDATES = {
	0: acct_sqlite_update_1,
}

#
# Database update event
#
def onUpdateAcctDatabase(current, version):
	database.open(ACCOUNTS)

	driver = database.driver(ACCOUNTS)
	if driver == 'mysql':
		updates = ACCT_MYSQL_UPDATES
	elif driver == 'sqlite':
		updates = ACCT_SQLITE_UPDATES
	else:
		console.log(LOG_ERROR, "Unknown database driver for Accounts: %s.\n" % driver)
		database.close(ACCOUNTS)
		return False

	for i in range(version, current):
		# No update for this version available
		if not updates.has_key(i):
			console.log(LOG_ERROR, "No update available for database version %u.\n" % i)
			database.close(ACCOUNTS)
			return False

		console.log(LOG_MESSAGE, "Updating database from version %u to %u.\n" % (i, i+1))

		try:
			if not updates[i]():
				database.close(ACCOUNTS)
				return False
		except Exception, e:
			console.log(LOG_ERROR, str(e) + "\n")
			database.close(ACCOUNTS)
			return False

		# Updating Version Number
		updateacctversion(i+1, driver)


	database.close(ACCOUNTS)

	return True
