#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts
#  ((    _/{  "-;  # Created by: Dreoth
#   )).-' {{ ;'`   # Revised by:
#  ( (  ;._ \\ ctr # Last Modification: Created
#################################################################
#
# !!!!!!!!!!!! ONLY USE WITH MYSQL !!!!!!!!!!!!
#
# This script should ONLY be used with mysql, if you use this with sqlite it will NOT work!
# This will perform a periodic OPTIMIZE on the mysql world tables. This will remove overhead and
# improve performance with saves/loads.
#
# !!!!!!!!!!!! ONLY USE WITH MYSQL !!!!!!!!!!!!
#
#################################################################

import wolfpack
import wolfpack.database
from wolfpack.consts import *
from wolfpack.time import *
from wolfpack.database import *

database = wolfpack.database
log = wolfpack.console.log

accountsdriver = database.driver( database.ACCOUNTS )
worlddriver = database.driver( database.WORLD )

time = 24 * 3600000 # Value * Hours

mysql = 'mysql'
sqlite = 'sqlite'
supporteddrivers = ['mysql','sqlite']

"""
	\command optimizedb
	\description Optimize the world and account database.
	\notes This command can only be used if one of your
	databases is using the MySQL driver.
"""

def onLoad():
	if accountsdriver in supporteddrivers or worlddriver in supporteddrivers:
		wolfpack.addtimer( time, "system.mysql_optimize_db.timer", [] )
		wolfpack.registercommand( "optimizedb", cmdoptimizedb )
	return

def onUnload():
	return

def cmdoptimizedb( socket, command, arguments ):
	if accountsdriver in supporteddrivers or worlddriver in supporteddrivers:
		socket.sysmessage( "Optimizing the database.", GRAY )
		optimize_db()
		socket.sysmessage( "Done!", GRAY )
	return

def timer( timer, args ):
	if accountsdriver in supporteddrivers or worlddriver in supporteddrivers:
		# Optimize and restart timer
		optimize_db()
		wolfpack.addtimer( time, "system.mysql_optimize_db.timer", [] )
	return

def optimize_db():
	# MySQL
	if accountsdriver == mysql or worlddriver == mysql:
		try:
			if accountsdriver == mysql and worlddriver == mysql:
				log( LOG_MESSAGE, "MySQL: Optimizing accounts and world databases..." )
			elif accountsdriver == mysql or worlddriver == mysql:
				if accountsdriver == mysql:
					log( LOG_MESSAGE, "MySQL: Optimizing accounts database..." )
				elif worlddriver == mysql:
					log( LOG_MESSAGE, "MySQL: Optimizing world database..." )
			# Time to optimize
			if accountsdriver == mysql:
				database.open( database.ACCOUNTS )
				database.execute( "OPTIMIZE TABLE %s" % database.MYSQL_ACCOUNTS )
				database.close()
				log( LOG_MESSAGE, "MySQL: Optimized accounts database!" )
			if worlddriver == mysql:
				database.open( database.WORLD )
				database.execute( "OPTIMIZE TABLE %s" % database.MYSQL_WORLD )
				database.close()
				log( LOG_MESSAGE, "MySQL: Optimized world database!" )
		except:
			log( LOG_MESSAGE, " MySQL: Performing world/accounts optimize failed." )
	# SQLite
	if accountsdriver == sqlite or worlddriver == sqlite:
		try:
			if accountsdriver == sqlite and worlddriver == sqlite:
				log( LOG_MESSAGE, "SQLite: Optimizing accounts and world databases..." )
			elif accountsdriver == sqlite or worlddriver == sqlite:
				if accountsdriver == sqlite:
					log( LOG_MESSAGE, "SQLite: Optimizing accounts database..." )
				elif worlddriver == sqlite:
					log( LOG_MESSAGE, "SQLite: Optimizing world database..." )
			# Time to optimize
			if accountsdriver == sqlite:
				database.open( database.ACCOUNTS )
				database.execute( "VACUUM %s" % database.SQLITE_ACCOUNTS )
				database.close()
				log( LOG_MESSAGE, "SQLite: Optimized accounts database!" )
			if worlddriver == mysql:
				database.open( database.WORLD )
				database.execute( "VACUUM %s" % database.SQLITE_WORLD )
				database.close()
				log( LOG_MESSAGE, "SQLite: Optimized world database!" )
		except:
			log( LOG_MESSAGE, "SQLite: Performing world/accounts optimize failed." )
	return
