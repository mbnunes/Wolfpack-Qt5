
import wolfpack
from wolfpack.consts import *
from wolfpack import console, database
from wolfpack.database import WORLD

#
# MySQL: Update Database Version 7 to 8
#
def mysql_update_11():
	return True

def mysql_update_7():
	sql = "ALTER TABLE players ADD `maxcontrolslots` tinyint(4) NOT NULL default '5' AFTER intlock;"
	database.execute(sql)

def mysql_update_10():
	# Create new guild tables

	sql = """CREATE TABLE guilds_enemies (
		guild unsigned int(10) NOT NULL default '0',
		enemy unsigned int(10) NOT NULL default '0',
		PRIMARY KEY(guild,enemy)
		);"""

	database.execute(sql)

	sql = """CREATE TABLE guilds_allies (
		guild unsigned int(10) NOT NULL default '0',
		ally unsigned int(10) NOT NULL default '0',
		PRIMARY KEY(guild,ally)
		);"""

	database.execute(sql)

	return True
#
# SQLLite Database Updates
#

def sqlite_update_11():
	return True # Do Nothing

def sqlite_update_10():
	return True # Do Nothing

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

	#sql = "replace into settings (option, value) values ('db_version',8)"
	#database.execute(sql)

	sql = "VACUUM"
	database.execute(sql)

	return True

MYSQL_UPDATES = {
	7: mysql_update_7,
	9: mysql_update_10,
	10: mysql_update_11,
}

SQLITE_UPDATES = {
	7: sqlite_update_8,
	# ???? What the hell happened to 9 ????
	9: sqlite_update_10,
	10: sqlite_update_11,
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
			elif driver == 'sqlite':
				database.execute("REPLACE INTO settings VALUES('db_version', '%u');" % (i + 1))
		except Exception, e:
			console.log(LOG_WARNING, "Unable to update database version to %u:\n%s\n" % (i + 1, str(e)))

	return True

def onLoad():
	wolfpack.registerglobal(EVENT_UPDATEDATABASE, 'system.dbupdate')
