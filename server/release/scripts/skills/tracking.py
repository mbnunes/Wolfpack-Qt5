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

#cliloc
#502989 tracking failed
#502990 This area is too crowded to track any individual animal.
#502991 You see no evidence of animals in the area.
#502992 This area is too crowded to track any individual creature.
#502993 You see no evidence of creatures in the area.
#502994 This area is too crowded to track any individual.
#502995 You see no evidence of people in the area.
#1011350 What do you wish to track?
#1018086 What do you wish to track?
#1018093 Select the one you would like to track.
#1002165 tracking
#1002166 The tracking skill will allow you to track animals, monsters, and people. Use the skill by clicking the jewel on your skill list. You will be presented with a window, which will allow you to choose what you wish to track. Double-click the category you wish to track (animal, human, or monster), and you will be presented with a second window listing the specific PCs or NPC's that you can track. If you are successful, and an appropriate creature can be tracked, you will be told in what direction that creature can be found. The higher your skill level
#1005645 I did not find a nearby track piece

# Some comments from stratics to implement some time:
# Necromancers can be a bit harder or easier depending on which form they aren in.
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

	if char.socket.hastag('usingtracking'):
		socket.sysmessage('You stop tracking the target')
		char.socket.questarrow(False, 0, 0)
		char.socket.deltag('usingtracking')
		return True
   
	socket.clilocmessage( 1011350 ) # What do you wish to track?
	socket.closegump( 0x87651592 ) # What to track
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

	socket.settag( 'skill_delay', int( wolfpack.time.currenttime() + TRACKING_DELAY ) )

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

def trackWhatResponse( char, args, target ):
	# what the range, hypothesis : skill 100%==radius 50==the small map ingame
	rayon= char.skill[ TRACKING ]*50/1000
	button=target.button
	liste=[]
	#look around characters (not really a circle but nervermind)
	iterator = wolfpack.charregion(char.pos.x -rayon,char.pos.y-rayon,char.pos.x +rayon,char.pos.y+rayon,char.pos.map)
	# filter the list with the player's choice
	charcible = iterator.first
	while charcible:
		# NEVER track invisible staff members
		if not charcible.invisible and not charcible.dead:
			if target.button == 4 : #have to exit non-players and exit self
				if charcible.player and not charcible == char:
					# Tracking Chances
					if ( charcible.skill[HIDING] + charcible.skill[STEALTH] ) > 0:
						chance = int( ( char.skill[TRACKING] + char.skill[DETECTINGHIDDEN] + random.randint(10,200) ) / ( charcible.skill[HIDING] + charcible.skill[STEALTH] ) * 50 )
					else:
						chance = 100
	
					if charcible.elf:
						chance -= ( chance * elves_tracking_chance ) / 100
					
					if random.randint(1,100) < chance:
						liste.append(charcible)

			if target.button == 3 : #have to exit non-PNJ humans
				if charcible.bodytype==4 and charcible.npc:
					liste.append(charcible)
			if target.button == 2 : #have to exit non-Monsters
				if charcible.bodytype==1 and charcible.npc:
					liste.append(charcible)
			if target.button == 1 : #have to exit non-animals
				if charcible.bodytype==3 and charcible.npc:
					liste.append(charcible)
				
		charcible = iterator.next
	#Answer to the player
	if liste==[]: #nothing to find
		if target.button == 4 or target.button == 3:
			char.socket.clilocmessage( 502995, '', GRAY ) # You see no evidence of people in the area.
		elif target.button == 1:
			char.socket.clilocmessage( 502991, '', GRAY ) # You see no evidence of animals in the area.
		else:
			char.socket.clilocmessage( 502993, '', GRAY ) # You see no evidence of creatures in the area.
	else: #list is not empty

		createsecondmenu(char, liste, 1)

		#for charcible in liste:
		#	char.socket.sysmessage("%s %s"%(charcible.name,dircard(char.directionto(charcible))))
	
	#check skill
	success = char.checkskill( TRACKING, 0, 1000 )
	return True

def createsecondmenu(char, liste, startpoint):

	socket = char.socket

	socket.closegump( 0x87651592 ) # What to track
	gump = cGump( x=20, y=30, callback=trackWhatResponse2, type=0x87651592 )

	gump.startPage( 0 )
	gump.addBackground( 5054, 440, 135 )
	gump.addResizeGump( 10, 10, 2620, 420, 75 )
	gump.addResizeGump( 10, 85, 3000, 420, 25 )

	# Last and next buttons
	if ( len(liste) - startpoint + 1 ) > 4:
		gump.addButton(415, 115, 5224, 5224, 200)

	if startpoint > 4:
		gump.addButton(5, 115, 5223, 5223, 201)
	
	# First Char
	if not len(liste) < startpoint:
		npc1 = liste[startpoint - 1]
		bodyinfo = wolfpack.bodyinfo(npc1.id)
		showid1 = bodyinfo['figurine']	

		gump.addTilePic( 20, 20, showid1 )
		gump.addButton( 25, 110, 4005, 4007, startpoint )
		gump.addText(24, 90, npc1.name, 0)

	# Second Char
	if not len(liste) < ( startpoint + 1 ):
		npc2 = liste[startpoint]
		bodyinfo = wolfpack.bodyinfo(npc2.id)
		showid2 = bodyinfo['figurine']

		gump.addTilePic( 120, 20, showid2 )
		gump.addButton( 125, 110, 4005, 4007, startpoint + 1 )
		gump.addText(120, 90, npc2.name, 0)

	# Third Char
	if not len(liste) < ( startpoint + 2 ):
		npc3 = liste[startpoint + 1]
		bodyinfo = wolfpack.bodyinfo(npc3.id)
		showid3 = bodyinfo['figurine']

		gump.addTilePic( 220, 20, showid3 )
		gump.addButton( 225, 110, 4005, 4007, startpoint + 2 )
		gump.addText(220, 90, npc3.name, 0)

	# Last Char
	if not len(liste) < ( startpoint + 3 ):
		npc4 = liste[startpoint + 2]
		bodyinfo = wolfpack.bodyinfo(npc4.id)
		showid4 = bodyinfo['figurine']

		gump.addTilePic( 320, 20, showid4 )
		gump.addButton( 325, 110, 4005, 4007, startpoint + 3 )
		gump.addText(320, 90, npc4.name, 0)

	gump.setArgs( [liste, startpoint] )

	gump.send( char )

	if skills.skilltable[ TRACKING ][ skills.UNHIDE ] and char.hidden:
		char.reveal()
	return True

def trackWhatResponse2( char, args, target ):

	# Retrieve List and Startpoint
	list = args[0]
	startpoint = args[1]
	
	# Next and last buttons
	if target.button == 200:
		createsecondmenu(char, list, startpoint + 4)
		return True

	if target.button == 201:
		createsecondmenu(char, list, startpoint - 4)
		return True

	# Button is 0?
	if target.button == 0:
		return True

	# Lets show the NPC position
	npc = list[target.button - 1]

	# Send the Arrow
	char.socket.questarrow(True, npc.pos.x, npc.pos.y)

	# Assign that we are using Tracking now. On RunUO I think a reverse click make arrow disappear... but its not supported on core yet (Ill work on it later). Here, to turn off the arrow we have to use Tracking again
	char.socket.settag('usingtracking', 1)

	# Message
	char.socket.sysmessage('Tracking the target. Use tracking Skill again to turn off the arrow...')

	return True

def onLoad():
	skills.register( TRACKING, tracking ) 
