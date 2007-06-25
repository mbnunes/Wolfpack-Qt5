#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: Correa /Surcouf                #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################
#TODO:
#can fail
#use a gump instead of sysmessage to answer

import wolfpack
import wolfpack.time
import skills
import random
from wolfpack.consts import TRACKING, HIDING, DETECTINGHIDDEN, STEALTH
from wolfpack.gumps import cGump
from wolfpack.consts import GRAY
from wolfpack import tr

elves_tracking_chance = int( wolfpack.settings.getnumber("Racial Features", "Elves evade tracking chance", 50, True) )

# Some comments from stratics to implement some time:
# Vampires always have a minimum of 50, if their Hiding + Stealth is less than 50. 
# The big blue four-armed demon gets a penalty of 20. It's so big it's easier to track. 
# Wraiths and Banshees get a bonus of 20 to their difficulty, if they're below 200. 

TRACKING_DELAY = 5000

def tracking( char, skill ):
	socket = char.socket

	if skill != TRACKING or not char.socket:
		return False

	if char.socket.hastag( 'skill_delay' ):
		if wolfpack.time.currenttime() < socket.gettag( 'skill_delay' ):
			socket.clilocmessage( 500118, "", 0x3b2, 3 ) # You must wait a few moments to use another skill.
			return True
		else:
			socket.deltag( 'skill_delay' )
   
	socket.clilocmessage( 1011350 ) # What do you wish to track?
	socket.closegump( 0x87651592 ) # What to track
	TrackWhatGump( char )

def TrackWhatGump( char ):
	gump = cGump( x=20, y=30, callback=trackWhatResponse, type=0x87651592 )

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

	char.socket.settag( 'skill_delay', int( wolfpack.time.currenttime() + TRACKING_DELAY ) )

	if skills.skilltable[ TRACKING ][ skills.UNHIDE ] and char.hidden:
		char.reveal()
	return True

def dircard(val): #define cardinal
	if val ==0:
		return tr("is in the North")
	elif val==1:
		return tr("is in the North-East")
	elif val==2:
		return tr("is in the East")
	elif val==3:
		return tr("is in the South-East")
	elif val==4:
		return tr("is in the South")
	elif val==5:
		return tr("is in the South-West")
	elif val==6:
		return tr("is in the West")
	elif val==7:
		return tr("is in the North-West")
	elif val==8:
		return tr("is here")

def CheckDifficulty( char, target ):
	tracking = char.skill[TRACKING]
	detectHidden = char.skill[DETECTINGHIDDEN]

	if target.elf:
		tracking /= 2

	hiding = target.skill[HIDING]
	stealth = target.skill[STEALTH]
	divisor = hiding + stealth

	# Necromancy forms affect tracking difficulty 
	if target.hasscript("magic.horrificbeast"):
		divisor -= 200
	elif target.hasscript("magic.vampiricembrace") and divisor < 500:
		divisor = 500
	elif target.hasscript("magic.wraithform") and divisor <= 2000:
		divisor += 200

	chance = 0
	if divisor > 0:
		#if ( Core.SE )
		chance = 50 * (tracking * 2 + detectHidden) / divisor
		#else
			#chance = 50 * (tracking + detectHidden + 10 * Utility.RandomMinMax( 1, 20 )) / divisor;
	else:
		chance = 100

	return chance > random.randint( 1, 100 )


def trackWhatResponse( char, args, response ):
	success = char.checkskill( TRACKING, 0, 211 )
	if not success:
		char.socket.clilocmessage( 1018092 ) # You see no evidence of those in the area.
		return
	
	char.checkskill( TRACKING, 211, 1000 ) # Passive gain

	ran= 10 + ( char.skill[ TRACKING ] / 10 )
	
	button = response.button
	
	liste=[]
	for charcible in wolfpack.chars(char.pos.x, char.pos.y, char.pos.map, ran):
		if not charcible.invisible and not charcible.dead and CheckDifficulty(char, charcible):
			# Track players
			if button == 4:
				if charcible.player and not charcible == char:
					liste.append(charcible)

			elif charcible.npc:
				# Track humans
				if button == 3 and charcible.bodytype == 4:
					liste.append(charcible)
				# Track monsters
				elif button == 2 and charcible.bodytype == 1:
					liste.append(charcible)
				# Track animals
				elif button == 1 and charcible.bodytype == 3:
					liste.append(charcible)

		#charcible = iterator.next

	if len(liste) == 0:
		if target.button in [3, 4]:
			char.socket.clilocmessage( 502995, '', GRAY ) # You see no evidence of people in the area.
		elif target.button == 1:
			char.socket.clilocmessage( 502991, '', GRAY ) # You see no evidence of animals in the area.
		else:
			char.socket.clilocmessage( 502993, '', GRAY ) # You see no evidence of creatures in the area.
	else:
		TrackWhoGump(char, liste, 1, ran)
		char.socket.clilocmessage( 1018093 ) # Select the one you would like to track.

from math import ceil
def TrackWhoGump(char, liste, startpoint, ran):

	socket = char.socket

	socket.closegump( 0x87651592 ) # TrackWhatGump/TrackWhoGump
	gump = cGump( x=20, y=30, callback=trackWhoResponse, type=0x87651592 )

	gump.startPage( 0 )
	gump.addBackground( 5054, 440, 135 )
	gump.addResizeGump( 10, 10, 2620, 420, 75 )
	gump.addResizeGump( 10, 85, 3000, 420, 25 )


	pages = ceil(len(liste) / 4)
	
	for page in range(1, pages + 1):
		gump.startPage(page)
		
		if page > 1:
			gump.addPageButton(5, 115, 5223, 5223, page - 1)
		
		if page < pages:
			gump.addPageButton(415, 115, 5224, 5224, page + 1)
		
		xoffset = 0
		for i in range(0, 4):
			if (page - 1) * 4 + i >= len(liste):
				break
			
			npc = liste[(page - 1) * 4 + i]
			
			bodyinfo = wolfpack.bodyinfo(npc.id)
			showid = bodyinfo['figurine']	
			
			gump.addTilePic( 20 + xoffset, 20, showid )
			gump.addButton( 25 + xoffset, 110, 4005, 4007, npc.serial )
			gump.addText(24 + xoffset, 90, npc.name, 0)
			
			xoffset += 100

	gump.setArgs( [liste, ran] )

	gump.send( char )

	if skills.skilltable[ TRACKING ][ skills.UNHIDE ] and char.hidden:
		char.reveal()
	return True

def trackWhoResponse( char, args, response ):
	# Button is 0?
	if response.button == 0:
		return True
		
	# Retrieve List and npc
	list = args[0]
	ran = args[1]
	npc = wolfpack.findchar(response.button)
	# Send the Arrow
	sendQuestArrow(char, npc, ran)

def sendQuestArrow( char, npc, ran ):
	char.socket.questarrow(True, npc.pos.x, npc.pos.y)
#	char.addtimer( 1000, TrackTimer, [ npc, ran ] )

# We need something to see if the Quest Arrow is activated
# if it's not (e.g. it was closed by click on it), we need not to resend
def TrackTimer( char, args ):
	npc = args[0]
	ran = args[1]
	if not npc and char.socket or not npc or not char.pos.map == npc.pos.map or not char.canreach(npc, ran):
		char.socket.clilocmessage( 503177 ) # You have lost your quarry.
		char.socket.cancelquestarrow()
	else:
		sendQuestArrow( char, npc, ran )

def onLoad():
	skills.register( TRACKING, tracking ) 
