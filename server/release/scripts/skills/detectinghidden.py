#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: khpae                          #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

from wolfpack.consts import *
from wolfpack.utilities import *
import wolfpack
import wolfpack.time
import skills

# UOSS : All dungeons can spawn the following traps at random locations: Floor saw trap, floor spike trap, poison gas trap and exploding mushroom trap.
# Boxes, Chests, Crates, Barrels, Kegs and Fruit Bowls(!) can be trapped. (Rule of thumb: If it is a container, it might be trapped)

# dungeon trap reveal duration 5 min
DUNGEON_TRAP_REVEAL_DURATION = 5*60*1000
dungeon_traps = range( 0x119a, 0x11b5 )

# faction trap reveal duration 1 min
FACTION_TRAP_REVEAL_DURATION = 1*60*1000

DETECTHIDDEN_DELAY = 5000

def detectinghidden( char, skill ):
	# only handle detect hidden
	if skill != DETECTINGHIDDEN:
		return False

	if char.socket.hastag( 'skill_delay' ):
		if wolfpack.time.currenttime() < char.socket.gettag( 'skill_delay' ):
			char.socket.clilocmessage( 500118, "", 0x3b2, 3 )
			return True
		else:
			char.socket.deltag( 'skill_delay' )

	char.socket.clilocmessage( 500819, "", 0x3b2, 3 )
	char.socket.attachtarget( "skills.detectinghidden.response" )

	return True

def response( char, args, target ):
	if not char:
		return

	if skills.skilltable[ DETECTINGHIDDEN ][ skills.UNHIDE ] and char.hidden:
		char.removefromview()
		char.hidden = False
		char.update()

	# if we are in our house : reveal all hidden chars in this house
	# w/o checking skill
	house = wolfpack.findmulti( char.pos )
	# it's our house
	if house and house.owner == char.serial:
		# get all chars in this house
		chars = house.chars()
		# set visible to the chars
		for echar in chars:
			if echar.hidden and echar.serial != char.serial:
				echar.hidden = 0
				echar.update()
		# do we need to checkskill ?
		return

	char.socket.settag( 'skill_delay', int( wolfpack.time.currenttime() + DETECTHIDDEN_DELAY ) )

	success = char.checkskill( DETECTINGHIDDEN, 0, 1000 )
	# You can see nothing hidden there
	if not success:
		char.socket.clilocmessage( 500817, "", 0x3b2, 3 )
		return

	pos = None
	item = None
	if target.pos:
		pos = target.pos
	elif target.item:
		item = target.item
	# when we target at a position : hidden people, dungeon trap, faction trap
	if pos:
		x = pos.x
		y = pos.y
		map = pos.map
		skill = char.skill[ DETECTINGHIDDEN ]
		reveal_range = 1 + skill / 100
		# first, we test hidden chars
		chars = wolfpack.chars( x, y, map, reveal_range )
		for tchar in chars:
			if not tchar.priv2 & 0x08:
				# hidden using hiding skill
				if tchar.hidden == 1:
					# FIXME : only hidden with lower skill will be revealed
					if tchar.skill[ HIDING ] <= skill:
						reveal_char( tchar )
				# hidden using invisibility spell
				elif tchar.hidden == 2:
					# FIXME : only hidden with lower skill will be revealed
					if tchar.skill[ MAGERY ] <= skill:
						reveal_char( tchar )
		# next, dungeon / factoin traps
		items = wolfpack.items( x, y, map, reveal_range )
		for titem in items:
			if titem.id in dungeon_traps:
				if titem.visible:
					if titem.hastag( 'level' ):
						level = item.gettag( 'level' )
					else:
						level = 1
					# level from 1 to 5
					if skill / 2 >= level:
						titem.addtimer( DUNGEON_TRAP_REVEAL_DURATION, "skills.detectinghidden.hide_trap", [ titem.visible ] )
						titem.visible = 0
						titem.update()
			# faction trap : no idea yet
	# now, we deal with the trap boxes - show trap items as color
	elif item:
		char.socket.sysmessage( "detecting trap box is not implemented yet." )

def reveal_char( char ):
	if not char or not char.socket:
		return
	char.socket.clilocmessage( 500814, "", 0x3b2, 3 )
	char.hidden = 0
	char.update()

def hide_trap( self, args ):
	if not self.isitem():
		return
	self.visible = args[ 0 ]
	self.update()

def onLoad():
	skills.register( DETECTINGHIDDEN, detectinghidden )
