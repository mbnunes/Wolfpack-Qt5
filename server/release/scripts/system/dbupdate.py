
import wolfpack
from wolfpack.consts import *
from wolfpack import console, database
from wolfpack.database import WORLD

#
# MySQL: Update Database Version 7 to 8
#
def mysql_update_7():
	sql = "ALTER TABLE players ADD `maxcontrolslots` tinyint(4) NOT NULL default '5' AFTER intlock;"
	database.execute(sql)
	return True

#
# MySQL Database Updates
#
MYSQL_UPDATES = {
	7: mysql_update_7,
}

SQLITE_UPDATES = {
	# 7: sqlite_update_8,
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
			database.execute("REPLACE INTO settings VALUES('db_version', '%u');" % (i + 1))
		except Exception, e:
			console.log(LOG_WARNING, "Unable to update database version to %u:\n%s\n" % (i + 1, str(e)))

	return True

def onLoad():
	wolfpack.registerglobal(EVENT_UPDATEDATABASE, 'system.dbupdate')
