#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: DarkStorm                      #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

# Script for bladed weapons

import wolfpack
import whrandom
import skills.lumberjacking
from wolfpack import utilities
from wolfpack import settings
from wolfpack.consts import *

#import weapons.blades

# Lists of IDs
blood = [ "122a", "122b", "122d", "122f" ]
fish = [ 0x9cc, 0x9cd, 0x9ce, 0x9cf ]

def onUse( char, item ):
	char.socket.clilocmessage( 0xF69A6 ) # What do you want to use this on?
	char.socket.attachtarget( "blades.response", [ item.serial ] )
	return 1

def regrow_wool(char, arguments):
	if char.npc:
		char.baseid = 'sheep_unsheered'
		char.id = 207
		char.update()

def response( char, args, target ):
	item = wolfpack.finditem( args[0] )
	if not item:
		return

	# Check reach
	if target.item and not char.canreach(target.item, 5):
		char.socket.clilocmessage(500312)
		return
	elif target.char and not char.canreach(target.char, 5):
		char.socket.clilocmessage(500312)
		return
	elif target.pos and not char.canreach(target.pos, 5):
		char.socket.clilocmessage(500312)
		return

	# Corpse => Carve
	# Wood => Kindling/Logs
	model = 0

	if target.item:
		if target.item.id == 0x2006 and target.item.corpse:
			carve_corpse( char, target.item )
			return
		else:
			model = target.item.id

		# For cutting fish
		if model in fish:
			cut_fish(char, target.item)
			return

	# This is for sheering only
	elif target.char and target.char.npc:
		if target.char.baseid == 'sheep_unsheered':
			target.char.id = 223
			target.char.baseid = 'sheep_sheered'
			target.char.update()

			# Create Wool
			wool = wolfpack.additem("df8")
			wool.amount = 2

			if not utilities.tobackpack(wool, char):
				wool.update()

			char.socket.clilocmessage( 0x7A2E4 ) # You place the gathered wool into your backpack.

			# Let the wool regrow (minutes)
			delay = settings.getnumber('Game Speed', 'Regrow Wool Minutes', 180, 1)
			delay *= 60000 # Miliseconds per Minute
			target.char.dispel(None, 1, "regrow_wool", [])
			target.char.addtimer(delay, "blades.regrow_wool", [], 1, 0, "regrow_wool")
			return
		elif target.char.id == 'sheep_sheered':
			char.socket.clilocmessage( 0x7A2E1 ) # This sheep is not yet ready to be shorn.
			return
		else:
			char.socket.clilocmessage( 0x7A2E2 ) # You can only skin dead creatures.
			return
	else:
		model = target.model

	if target.model == 0:
		map = wolfpack.map( target.pos.x, target.pos.y, target.pos.map )
		treeid = map['id']
	elif target.model != 0:
		treeid = target.model

	if utilities.istree(treeid):
		# Axes/Polearms get Logs, Swords get kindling.
		# Also allows a mace's war axe to be use. 0x13af and 0x13b0
		if item.type == 1002 or item.id == 0x13af or item.id == 0x13b0:
			if not item or not item.container == char:
				char.message( "You must equip this item to use it on this target!" )
				return
			else:
				skills.lumberjacking.response( [ target, item, char ] )
		# Swords and Fencing Weapons: Get kindling
		elif item.type == 1001 or item.type == 1005:
			skills.lumberjacking.hack_kindling( char, target.pos )
	else:
		# You can't use a bladed item on that.
		char.socket.clilocmessage( 500494, "", GRAY )
		return False

# CARVE CORPSE
def carve_corpse( char, corpse ):
	if corpse.container:
		char.socket.sysmessage( "You can't carve corpses in a container" )
		return

	if not char.canreach(corpse, 3):
		char.socket.clilocmessage( 0x7A258, "", 0x3b2, 3, corpse ) # You cannot reach that
		return

	# Human Bodies can always be carved
	if corpse.bodyid == 0x190 or corpse.bodyid == 0x191:
		char.message( "You can't carve a human body right now" )
		return

	# Not carvable or already carved
	try:
		charbase = wolfpack.charbase(corpse.charbaseid)
		carve = charbase['carve']
	except:
		char.socket.clilocmessage( 0x7A305, "", 0x3b2, 3, corpse ) # You see nothing useful to carve..
		return

	if corpse.hastag('carved') or carve == '':
		char.socket.clilocmessage( 0x7A305, "", 0x3b2, 3, corpse ) # You see nothing useful to carve..
		return

	# Create all items in the carve list
	carve = wolfpack.list(str(carve))

	for id in carve:
		amount = 1
		# Is amount contained in it ?
		if id.find( "," ) != -1:
			parts = id.split( "," )
			id = parts[0]
			amount = int( parts[1] )

		item = wolfpack.additem( id )
		item.amount = amount
		if not utilities.tocontainer( item, corpse ):
			item.update()

	# Create Random Blood
	bloodid = whrandom.choice( blood )
	blooditem = wolfpack.additem( bloodid )
	blooditem.moveto( corpse.pos )
	blooditem.decay = 1
	blooditem.update()

	char.socket.clilocmessage( 0x7A2F3, "", 0x3b2, 3, corpse ) # You carve away some meat which remains on the corpse
	corpse.settag('carved', 1)

# CUT FISH
def cut_fish( char, item ):
		item_new = wolfpack.additem( "97a" )
		item_new.amount = item.amount * 2
		if not utilities.tocontainer( item_new, char.getbackpack() ):
			item_new.update()
		item.delete()
