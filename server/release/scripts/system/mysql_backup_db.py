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
# This will perform a periodic BACKUP of the mysql world tables. This will remove overhead and
# improve performance with saves/loads.
#
# You must also make sure you're mysql user has access to the FILE privs!
# !!!!!!!!!!!! ONLY USE WITH MYSQL !!!!!!!!!!!!
#
#################################################################

import wolfpack
import wolfpack.database
from wolfpack.consts import *
from wolfpack.time import *
from wolfpack.database import *
import datetime
import os

# ONLY SET TO TRUE ONCE YOU ARE SETUP FOR THIS!
enabled = FALSE
# MAKE SURE YOU SET THIS!
backup_path = '/Path/To/Wolfpack/backups'
#backup_path = 'C:\Wolfpack\backups'

database = wolfpack.database
log = wolfpack.console.log

accountsdriver = database.driver( database.ACCOUNTS )
worlddriver = database.driver( database.WORLD )

time = 24 * 3600000 * 7 # Every Week

worldtables = "`boats`, `boats_itemids`, `boats_itemoffsets`,`bookpages`,`books`, `characters`, `corpses`, `corpses_equipment`, `effects`, `effects_properties`, `guilds`, `guilds_canidates`, `guilds_members`, `houses`, `items`, `multis`, `multis_bans`, `multis_friends`, `npcs`, `players`, `settings`, `skills`, `tags`, `uobject`, `uobjectmap`"

"""
	\command mysql_backup_db
	\description Performs a backup on the world and account database.
	\notes This command can only be used if one of your
	databases is using the MySQL driver.
"""

def onLoad():
	if enabled == TRUE:
		if accountsdriver == 'mysql' or worlddriver == 'mysql':
			wolfpack.addtimer( time, "system.mysql_backup_db.timer", [] )
			wolfpack.registercommand( "mysql_backup", cmdbackupdb )

def cmdbackupdb( socket, command, arguments ):
	if enabled == TRUE:
		if accountsdriver == 'mysql' or worlddriver == 'mysql':
			socket.sysmessage( "Performing a database backup.", GRAY )
			backup_db()
			socket.sysmessage( "Done!", GRAY )
			return

def timer( timer, args ):
	if enabled == TRUE:
		if accountsdriver == 'mysql' or worlddriver == 'mysql':
			# Optimize and restart timer
			optimize_db()
			wolfpack.addtimer( time, "system.mysql_backup_db.timer", [] )
			return

def backup_db():
	if enabled == TRUE:
		accountsdir = "%s/%s_accounts" % ( backup_path, datetime.date.today() )
		worlddir = "%s/%s_world" % ( backup_path, datetime.date.today() )
		if accountsdriver == 'mysql' or worlddriver == 'mysql':
			try:
				# What are we optimizing
				if accountsdriver == 'mysql' and worlddriver == 'mysql':
					log( LOG_MESSAGE, "Doing a backup for the accounts and world databases..." )
					if not os.path.isdir( accountsdir ):
						os.mkdirs( accountsdir )
						os.chmod( accountsdir, 0777)
					if not os.path.isdir( worlddir ):
						os.mkdirs( worlddir )
						os.chmod( worlddir, 0777)
				elif accountsdriver == 'mysql' or worlddriver == 'mysql':
					if accountsdriver == 'mysql':
						if not os.path.isdir( accountsdir ):
							os.mkdirs( accountsdir )
							os.chmod( accountsdir, 0777)
						log( LOG_MESSAGE, "Doing a backup for the accounts database..." )
					elif worlddriver == 'mysql':
						if not os.path.isdir( worlddir ):
							os.mkdirs( worlddir )
							os.chmod( worlddir, 0777)
						log( LOG_MESSAGE, "Doing a backup for the world database..." )
				# Time to backup
				if accountsdriver == 'mysql':
					database.open( database.ACCOUNTS )
					database.execute( "BACKUP TABLE `accounts` TO '%s'" % ( accountsdir ) )
					database.close()
					log( LOG_MESSAGE, "Accounts database saved to: %s" % ( accountsdir ) )
				if worlddriver == 'mysql':
					database.open( database.WORLD )
					database.execute( "BACKUP TABLE %s TO '%s'" % ( worldtables, worlddir ) )
					database.close()
					#else:
					#	log( LOG_MESSAGE, "Warning: Failed to open the world database!" )
					log( LOG_MESSAGE, "World database saved to: %s" % (worlddir ) )
			except:
				log( LOG_MESSAGE, " Backup of the accounts/world database failed." )
		return
