#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: Correa                         #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#

from speech.pets import *

def onContextCheckVisible(char, object, tag):
	# All entries are invisible if we're not the owner of this NPC
	# In Addition Transfer is invisible if the target is untamed
	if (not char.gm) and (tag == 7) and (object.summoned or not object.tamed):
		return False

	return (object.owner == char) or char.gm

def onContextEntry(char, target, tag):
	if (not char.gm) and (not target.hasscript('speech.pets')):
		return False

	#it has to be our pet or we have to be gm
	if (not char.gm) and ((target.owner != char) or (not target.tamed)):
		return False

	#check if can be controlled
	if not checkPetControl(target, char, "", ""):
		return True

	if tag == 1: # Command: Kill
		attack(char, target, 0)
	elif tag == 2: # Command: Follow
		follow(char, target, 0)
	elif tag == 5: # Command: Stay
		stop(char, target, 0)
	elif tag == 7: # Transfer
		transfer(char, target)
	elif tag == 8: # Release
		release(target)

	# Disabled for now
	#elif ( tag == 3 ): # Command: Guard
	#		ai.onSpeechInput( char, target.name + " GUARD" )
	#elif ( tag == 4 ): # Command: Stop
	#		ai.onSpeechInput( char, target.name + " STOP" )
	#elif ( tag == 6 ): # Add Friend
	#		ai.onSpeechInput( char, target.name + " ADD FRIEND" )

	return True
