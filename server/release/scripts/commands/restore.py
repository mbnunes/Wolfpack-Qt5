#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts
#  ((    _/{  "-;  | Created by: Dreoth
#   )).-' {{ ;'`   | Revised by:
#  ( (  ;._ \\ ctr | Last Modification: Created
#===============================================================#
# .restore Command
#===============================================================#

"""
	\command restore
	\description Restore health, stamina and mana of the targetted character.
	\note The targetted character won't be resurrected.
"""

"""
	\command heal
	\description Restore health of the targetted character.
	\note The targetted character won't be resurrected.
"""

import wolfpack
from wolfpack.consts import LOG_MESSAGE, GRAY

def onLoad():
	wolfpack.registercommand( "restore", restore )
	wolfpack.registercommand( "heal", heal )

def restore( socket, command, arguments ):
	try:
		socket.attachtarget( "commands.restore.dorestore" )
		socket.sysmessage( 'Target the character you wish to restore.' )
	except:
		socket.sysmessage( 'There was an error with restore.' )

def dorestore( char, args, target ):
	if target.item:
		char.socket.sysmessage( 'That was not a valid character!', GRAY )
	elif target.char:
		target.char.hitpoints = int( target.char.maxhitpoints )
		target.char.stamina = int( target.char.maxstamina )
		target.char.mana = int( target.char.maxmana )
		target.char.updatestats()
		target.char.effect(0x376A, 9, 32)
		target.char.soundeffect(0x202)
		char.log( LOG_MESSAGE, "Restored 0x%x.\n" % target.char.serial )

def heal( socket, command, arguments ):
	try:
		socket.attachtarget( "commands.restore.doheal" )
		socket.sysmessage( 'Target the character you wish to heal.' )
	except:
		socket.sysmessage( 'There was an error with heal.' )

def doheal( char, args, target ):
	if target.item:
		char.socket.sysmessage( 'That was not a valid character!', GRAY )
	elif target.char:
		target.char.hitpoints = int( target.char.maxhitpoints )
		target.char.updatehealth()
		target.char.effect(0x376A, 9, 32)
		target.char.soundeffect(0x202)
		char.log( LOG_MESSAGE, "Healed 0x%x.\n" % target.char.serial )
