#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: Correa                         #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#

from speech.pets import *
import gumps.confirm_release_gump

def onContextCheckVisible(char, object, tag):
	# All entries are invisible if we're not the owner of this NPC
	# In Addition Transfer is invisible if the target is untamed
	if (not char.gm) and (tag == 9) and (object.summoned or not object.tamed):
		return False

	# follow, stop and stay  are visible for friends
	if tag in [2, 6] and isPetFriend(char, object):
		return True

	if tag == 11:
		if ( (object.owner == char) or char.gm ) and object.id == 791 and not object.baseid == 'fire_beetle':
			return True
		else:
			return False

	return (object.owner == char) or char.gm

def onContextEntry(char, target, tag):
	if (not char.gm) and (not target.hasscript('speech.pets')):
		return False

	#it has to be our pet, a friend, or we have to be gm
	if (not char.gm) and ((target.owner != char) and not isPetFriend(char, target) or (not target.tamed)):
		return False

	#check if can be controlled
	if not checkPetControl(target, char, "", ""):
		return False

	if tag == 1: # Command: Guard
		guard(char, target)
	elif tag == 2: # Command: Follow
		follow(char, target, 0)
	elif tag == 3: # Command: Drop
		drop(target)
	elif tag == 4: # Command: Kill
		attack(char, target, 0)

	#elif tag == 5: # Command: Stop
	#	stop(char, target, 0)

	elif tag == 6: # Command: Stay
		stop(char, target, 0)
	elif tag == 7: # Add Friend
		addfriend(char, target)
	elif tag == 8: # Remove Friend
		removefriend(char, target)
	elif tag == 9: # Transfer
		transfer(char, target)
	elif tag == 10: # Release
		gumps.confirm_release_gump.ConfirmReleaseGump(char, target)

	elif ( tag == 11 ): # Open Backpack
		# Just to be sure... right?
		if ( (target.owner == char) or char.gm ) and target.id == 791 and not target.baseid == 'fire_beetle':
			char.socket.sendcontainer( target.getbackpack() )

	return True
