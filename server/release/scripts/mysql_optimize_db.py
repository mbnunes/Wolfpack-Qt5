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

#optimizequery = "OPTIMIZE TABLE `accounts` , `boats` , `boats_itemids` , `boats_itemoffsets` , `bookpages` , `books` , `characters` , `corpses` , `corpses_equipment` , `effects` , `effects_properties` , `guilds` , `guilds_canidates` , `guilds_members` , `houses` , `items` , `multis` , `multis_bans` , `multis_friends` , `npcs` , `players` , `settings` , `skills` , `tags` , `uobject` , `uobjectmap` "

def onLoad():
	if accountsdriver == 'mysql' or worlddriver == 'mysql':
		wolfpack.addtimer( time, "mysql_optimize_db.timer", [] )
		wolfpack.registercommand( "optimizedb", cmdoptimizedb )

def cmdoptimizedb( socket, command, arguments ):
	if accountsdriver == 'mysql' or worlddriver == 'mysql':
		socket.sysmessage( "Optimizing the database.", GRAY )
		optimize_db()
		socket.sysmessage( "Done!", GRAY )
		return

def timer( timer, args ):
	if accountsdriver == 'mysql' or worlddriver == 'mysql':
		# Optimize and restart timer
		optimize_db()
		wolfpack.addtimer( time, "custom.shard_status.timer", [] )
		return

def optimize_db():
	if accountsdriver == 'mysql' or worlddriver == 'mysql':
		try:
			# What are we optimizing
			if accountsdriver == 'mysql' and worlddriver == 'mysql':
				log( LOG_MESSAGE, "Optimizing accounts and world databases..." )
			elif accountsdriver == 'mysql' or worlddriver == 'mysql':
				if accountsdriver == 'mysql':
					log( LOG_MESSAGE, "Optimizing accounts database..." )
				elif worlddriver == 'mysql':
					log( LOG_MESSAGE, "Optimizing world database..." )
			# Time to optimize
			if accountsdriver == 'mysql':
				database.open( wolfpack.database.ACCOUNTS )
				database.execute( "OPTIMIZE TABLE `accounts`" )
				database.close()
				log( LOG_MESSAGE, "Optimized accounts database!" )
			if worlddriver == 'mysql':
				database.open( wolfpack.database.WORLD )
				database.execute( "OPTIMIZE TABLE `boats`, `boats_itemids`, `boats_itemoffsets`,`bookpages`,`books`, `characters`, `corpses`, `corpses_equipment`, `effects`, `effects_properties`, `guilds`, `guilds_canidates`, `guilds_members`, `houses`, `items`, `multis`, `multis_bans`, `multis_friends`, `npcs`, `players`, `settings`, `skills`, `tags`, `uobject`, `uobjectmap` " )
				database.close()
				log( LOG_MESSAGE, "Optimized world database!" )
		except:
			log( LOG_MESSAGE, " Performing world optimize failed." )
	return

