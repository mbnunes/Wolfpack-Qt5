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
from wolfpack.consts import TRACKING
from wolfpack.gumps import cGump
from wolfpack.consts import GRAY
from wolfpack import tr

elves_tracking_chance = int( wolfpack.settings.getnumber("Racial Features", "Elves evade tracking chance", 50, True) )

#cliloc
#502989 tracking failed
#502990 This area is too crowded to track any individual animal.
##502991 You see no evidence of animals in the area.
#502992 This area is too crowded to track any individual creature.Il y a trop de monde pour pouvoir pister une créature en particulier.
##502993 You see no evidence of creatures in the area.#Vous ne décelez nulle trace d'une créature dans cette zone.
#502994 This area is too crowded to track any individual.#Il y a trop de monde pour pouvoir pister quelqu'un.
##502995 You see no evidence of people in the area. #Vous ne décelez nulle trace de qui que ce soit dans cette zone.
#1011350 ) # What do you wish to track?
##1018086What do you wish to track?
#1018093 Select the one you would like to track.
#1002165 tracking
#1002166 The tracking skill will allow you to track animals, monsters, and people. Use the skill by clicking the jewel on your skill list. You will be presented with a window, which will allow you to choose what you wish to track. Double-click the category you wish to track (animal, human, or monster), and you will be presented with a second window listing the specific PCs or NPC's that you can track. If you are successful, and an appropriate creature can be tracked, you will be told in what direction that creature can be found. The higher your skill level
#1005645 I did not find a nearby track piece

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
				if charcible.player :
					if charcible.elf:
						if random.randint(1,100) > elves_tracking_chance:
							liste.append(charcible)
					else:
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
		for charcible in liste:
			char.socket.sysmessage("%s %s"%(charcible.name,dircard(char.directionto(charcible))))
	#check skill
	success = char.checkskill( TRACKING, 0, 1000 )
	return True

def onLoad():
	skills.register( TRACKING, tracking ) 
