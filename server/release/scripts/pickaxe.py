#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: codex                          #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Sep, 27 '03	        #
#################################################################

import wolfpack
import wolfpack.time
import skills
import whrandom
import wolfpack.utilities
from wolfpack.consts import GRAY, LAYER_MOUNT, MINING
from skills.mining import canminegranite

def onUse( char, tool ):
	#Already digging ?
	if char.socket.hastag( 'is_mining' ) and ( char.socket.gettag( 'is_mining' ) > wolfpack.time.currenttime() ):
		# You are already digging.
		char.socket.clilocmessage( 503029, "", GRAY )
		return True

	# Can't mine on horses
	if char.itemonlayer( LAYER_MOUNT ):
		# You can't mine while riding.
		char.socket.clilocmessage( 501864, "", GRAY )
		return True

	# Who is tool owner ?
	if tool.getoutmostchar() != char:
		# You can't use that, it belongs to someone else
		char.socket.clilocmessage( 500364, "", GRAY )
		return True

	# Is that mining tool ?
	if wolfpack.utilities.isminingtool( tool ):
		# Where do you wish to dig?
		char.socket.clilocmessage( 503033, "", GRAY)
		char.socket.attachtarget( "skills.mining.response", [ tool.serial ] )
	else:
		char.socket.clilocmessage( 500735, "", GRAY) # Don't play with things you don't know about. :)
		return True

	return True

def onContextCheckVisible( char, target, tag ):
	# WE must wear the item
	if canminegranite(char) and target.layer and target.getoutmostchar() == char:
		return True # visible

	return False # not visible

def onContextCheckEnabled( char, target, tag ):
	if tag == 300:
		if char.hastag( "mining" ) and char.gettag( "mining" ) == "ore":
			return False # disabled
	if tag == 301:
		if char.gettag( "mining" ) == "ore,stone":
			return False # disabled
	return True # enabled

def onContextEntry( char, target, tag  ):
	if tag == 300:
		if char.hastag( "mining" ) and char.gettag( "mining" ) == "ore":
			char.socket.clilocmessage( 1054021 ) # You are already set to mine only ore!
			return True
		char.settag( "mining", "ore" )
		char.socket.clilocmessage( 1054020 ) # You are now set to mine only ore.

	if tag == 301:
		if char.hastag( "mining" ) and char.gettag( "mining" ) == "ore,stone":
			char.socket.clilocmessage( 1054023 ) # You are already set to mine both ore and stone!
			return True
		char.settag( "mining", "ore,stone" )
		char.socket.clilocmessage( 1054022 ) # You are now set to mine both ore and stone
	return True
