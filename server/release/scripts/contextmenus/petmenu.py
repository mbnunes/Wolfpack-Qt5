#===============================================================#
#	 )			(\_		 | WOLFPACK 13.0.0 Scripts										#
#	((		_/{	"-;	| Created by: Correa												 #
#	 )).-' {{ ;'`	 | Revised by:																#
#	( (	;._ \\ ctr | Last Modification: Created								 #
#===============================================================#

from speech.pets import *

def onContextEntry(char, target, tag):
	if not "speech.pets" in target.events:
		return 0
		
	if target.owner != char or not target.tamed:
		return 0

	if tag == 1: # Command: Kill
			attack(char, target, 0)
	elif tag == 2: # Command: Follow
			follow(char, target, 0)
	elif tag == 5: # Command: Stay
			stop(char, target, 0)
	elif tag == 7: # Transfer
			transfer(char, target)
	elif tag == 8: # Release
			release(char, target)
			
	# Disabled for now
	#elif ( tag == 3 ): # Command: Guard
	#		ai.onSpeechInput( char, target.name + " GUARD" )
	#elif ( tag == 4 ): # Command: Stop
	#		ai.onSpeechInput( char, target.name + " STOP" )
	#elif ( tag == 6 ): # Add Friend
	#		ai.onSpeechInput( char, target.name + " ADD FRIEND" )				
			
	return 1


			
