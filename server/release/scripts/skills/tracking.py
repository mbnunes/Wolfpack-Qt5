#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: Correa                         #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

import wolfpack
import wolfpack.time
import skills
from wolfpack.consts import TRACKING
from wolfpack.gumps import cGump


STEALTH_DELAY = 5000
# the hiding skill before you can use the stealth skill
MIN_HIDING = 800

def tracking( char, skill ):
	socket = char.socket

	if skill != TRACKING or not char.socket:
		return False

	if char.socket.hastag( 'skill_delay' ):
		if wolfpack.time.currenttime() < socket.gettag( 'skill_delay' ):
			socket.clilocmessage( 500118, "", 0x3b2, 3 )
			return True
		else:
			socket.deltag( 'skill_delay' )

	socket.clilocmessage( 1011350 ) # What do you wish to track?
	socket.closegump( 0x87651592 ) # What to track
	gump = cGump( x=20, y=30, callback="skills.tracking.trackWhatResponse", type=0x87651592 )

	gump.startPage( 0 )
	gump.addBackground( 5054, 440, 135 )
	gump.addResizeGump( 10, 10, 2620, 420, 75 )
	gump.addResizeGump( 10, 85, 3000, 420, 25 )

	gump.addTilePic( 20, 20, 9682 )
	gump.addButton( 20, 110, 4005, 4007, 1 )
	gump.addXmfHtmlGump( 20, 90, 100, 20, 1018087 ) # Animals

	gump.addTilePic( 120, 20, 9607 )
	gump.addButton( 120, 110, 4005, 4007, 2 )
	gump.addXmfHtmlGump( 120, 90, 100, 20, 1018088 ) # Monsters

	gump.addTilePic( 220, 20, 8454 )
	gump.addButton( 220, 110, 4005, 4007, 3 )
	gump.addXmfHtmlGump( 220, 90, 100, 20, 1018089 ) # Human NPCs

	gump.addTilePic( 320, 20, 8455 )
	gump.addButton( 320, 110, 4005, 4007, 4 )
	gump.addXmfHtmlGump( 320, 90, 100, 20, 1018090 ) # Players

	gump.send( char )

	socket.settag( 'skill_delay', int( wolfpack.time.currenttime() + STEALTH_DELAY ) )

	return True

def trackWhatResponse( char, args, target ):
	return True

def onLoad():
	skills.register( TRACKING, tracking )

