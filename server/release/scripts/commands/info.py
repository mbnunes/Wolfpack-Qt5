#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: Correa                         #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#
# .info Commands						#
#===============================================================#

"""
	\command info
	\description Show a dialog for modifying the properties of items or characters.
"""

import wolfpack
import string
import math
import wolfpack.gumps
from wolfpack.consts import *
from wolfpack.gumps import *
from wolfpack.utilities import *
from wolfpack import *

def info( socket, command, argstring ):
	#args = argstring.split(" ")
	#if len(argstring) > 0:
		# TODO: implement spawnregion info gump and more here
		#return 1

	socket.attachtarget( "commands.info.infotargetresponse" )
	return 1

def onLoad():
	wolfpack.registercommand( "info", info )

def infotargetresponse( char, args, target ):
	if not char.socket:
		return 0

	# map target
	if target.char:
		charinfo( char.socket, target.char )
		return 1
	elif target.item:
		iteminfo( char.socket, target.item )
		return 1
	elif target.model == 0:
		map = wolfpack.map( target.pos.x, target.pos.y, target.pos.map )
		tile = wolfpack.landdata( map['id'] )
		maptileinfo( char.socket, map, tile )
		return 1
	elif target.model != 0:
		tile = wolfpack.tiledata( target.model )
		statictileinfo( char.socket, target.model, target.pos, tile )
		return 1
	return 1

def maptileinfo( socket, map, tile ):
	gump = cGump( 0, 0, 0, 0, 40 )
	gump.addResizeGump( 0, 40, 0xA28, 450, 250 ) # Background
	gump.addGump( 105, 18, 0x58B ) # Fancy top-bar
	gump.addGump( 182, 0, 0x589 ) # "Button" like gump
	gump.addTilePic( 202, 23, 0x14eb ) # Type of info menu

	gump.addText( 175, 90, "Landscape Info", 0x530 )

	# Give information about the tile
	gump.addText( 50, 120, "Name: "+tile['name'], 0x834 )
	gump.addText( 50, 145, "ID: "+hex(map['id']), 0x834 )
	gump.addText( 50, 170, "Z Height: "+unicode(map['z']), 0x834 )

	# Wet ? Impassable ? At least these are the most interesting
	flags = ""

	if tile['flag1']&0x80:
		flags = flags + "wet, "

	if tile['flag1']&0x40:
		flags = flags + "impassable, "

	if tile['flag2']&0x02:
		flags = flags + "surface, "

	if tile['flag2']&0x04:
		flags = flags + "stairs, "

	flags = flags[0:len(flags)-2] # cut the last ", "
	gump.addText( 50, 195, "Properties: "+flags, 0x834 )

	# OK button
	gump.addButton( 50, 240, 0xF9, 0xF8, 0 ) # Only Exit possible

	gump.send( socket )
	return

def statictileinfo( socket, model, pos, tile ):
	gump = cGump( 0, 0, 0, 0, 40 )

	gump.addResizeGump( 0, 40, 0xA28, 450, 300 ) # Background
	gump.addGump( 105, 18, 0x58B ) # Fancy top-bar
	gump.addGump( 182, 0, 0x589 ) # "Button" like gump
	gump.addTilePic( 202, 23, 0x14EF ) # Display our tile

	gump.addText( 175, 90, "Static Info", 0x530 )

	# Give information about the tile
	gump.addText( 50, 120, "Name: "+tile['name'], 0x834 )
	gump.addText( 50, 140, "ID: "+hex( model ), 0x834 )
	gump.addText( 50, 160, "Position: "+unicode( pos.x )+","+unicode( pos.y )+","+unicode( pos.z ), 0x834 )
	gump.addText( 50, 180, "Weight: "+unicode( tile['weight'] ), 0x834 )
	gump.addText( 50, 200, "Height: "+unicode( tile['height'] ), 0x834 )

	# Wet ? Impassable ? At least these are the most interesting
	gump.addCroppedText( 50, 220, 230, 40, "Properties: "+ tile['flagnames'], 0x834 )

	# OK button
	gump.addButton( 50, 275, 0xF9, 0xF8, 0 ) # Only Exit possible

	# Item Preview
	gump.addResizeGump( 300, 120, 0xBB8, 110, 150 )
	gump.addTilePic( 340, 160 - ( tile['height'] / 2 ), model )

	gump.send( socket )
	return

def charinfo( socket, char ):
	if not char or not socket:
		return

	gump = wolfpack.gumps.cGump()
	gump.setCallback( "commands.info.charinfo_response" )
	gump.setArgs( [char] )

	gump.startPage(0)
	gump.addResizeGump( 64, 34, 9260, 464, 462 ) #Background
	gump.addGump(30, 33, 10421, 0)
	gump.addGump(59, 22, 10420, 0)
	gump.addGump(43, 173, 10422, 0)
	gump.addResizeGump(100, 79, 9200, 405, 65 )
	if not char.npc:
		gump.addText(160, 90, "Character Properties", 380)
		gump.addText( 160, 110, "Serial: " + hex( char.serial ), 0x834 )
		gump.addResizeGump(100, 151, 9200, 404, 283)
		gump.addGump( 43, 173, 10422, 0 )
	elif char.npc:
		gump.addText(160, 90, "NPC Properties", 380)
		gump.addText( 160, 110, "Serial: " + hex( char.serial ), 0x834 )
		gump.addResizeGump(100, 151, 9200, 404, 283)
		gump.addGump( 43, 173, 10422, 0 )
	# Apply button
	gump.addButton( 102, 450, 0xEF, 0xF0, 1 )
	# OK button
	gump.addButton( 177, 450, 0xF9, 0xF8, -1 )
	# Cancel button
	gump.addButton( 252, 450, 0xF3, 0xF1, 0 )

	gump.startPage( 1 )

	# lets begin with basechar information
	#gump.addResizeGump( 195, 120, 0xBB8, 215, 20 )
	# 1
	gump.addText( 113, 160, "Name:", 0x834 )
	gump.addResizeGump( 280, 160, 0xBB8, 215, 20 )
	gump.addInputField( 284, 160, 200, 16, 0x834,  1, char.name )
	# 2
	gump.addText( 113, 180, "Title:", 0x834 )
	gump.addResizeGump( 280, 180, 0xBB8, 215, 20 )
	gump.addInputField( 284, 180, 200, 16, 0x834,  2, char.title )
	# 3
	gump.addText( 113, 200, "Body:", 0x834 )
	gump.addResizeGump( 280, 200, 0xBB8, 215, 20 )
	gump.addInputField( 284, 200, 200, 16, 0x834,  3, hex( char.id ) )
	# 4
	gump.addText( 113, 220, "Original Body:", 0x834 )
	gump.addResizeGump( 280, 220, 0xBB8, 215, 20 )
	gump.addInputField( 284, 220, 200, 16, 0x834,  4, hex( char.orgid ) )
	# 5
	gump.addText( 113, 240, "Skin:", 0x834 )
	gump.addResizeGump( 280, 240, 0xBB8, 215, 20 )
	gump.addInputField( 284, 240, 200, 16, 0x834,  5, hex( char.skin ) )
	# 6
	gump.addText( 113, 260, "Original Skin:", 0x834 )
	gump.addResizeGump( 280, 260, 0xBB8, 215, 20 )
	gump.addInputField( 284, 260, 200, 16, 0x834,  6, hex( char.orgskin ) )
	# 7
	gump.addText( 113, 280, "Fame:", 0x834 )
	gump.addResizeGump( 280, 280, 0xBB8, 215, 20 )
	gump.addInputField( 284, 280, 200, 16, 0x834, 7, unicode( char.fame ) )
	# 8
	gump.addText( 113, 300, "Karma:", 0x834 )
	gump.addResizeGump( 280, 300, 0xBB8, 215, 20 )
	gump.addInputField( 284, 300, 200, 16, 0x834, 8, unicode( char.karma ) )
	# 9
	gump.addText( 113, 320, "Kills:", 0x834 )
	gump.addResizeGump( 280, 320, 0xBB8, 215, 20 )
	gump.addInputField( 284, 320, 200, 16, 0x834, 9, unicode( char.kills ) )
	# 10
	gump.addText( 113, 340, "Deaths:", 0x834 )
	gump.addResizeGump( 280, 340, 0xBB8, 215, 20 )
	gump.addInputField( 284, 340, 200, 16, 0x834, 10, unicode( char.deaths ) )
	# 11
	gump.addText( 113, 360, "Position (x,y,z,map):", 0x834 )
	gump.addResizeGump( 280, 360, 0xBB8, 215, 20 )
	gump.addInputField( 284, 360, 200, 16, 0x834, 10, unicode( char.pos.x )+","+unicode( char.pos.y )+","+unicode( char.pos.z )+","+unicode( char.pos.map ) )
	# 12
	gump.addText( 113, 380, "Direction:", 0x834 )
	gump.addResizeGump( 280, 380, 0xBB8, 215, 20 )
	gump.addInputField( 284, 380, 200, 16, 0x834, 12, unicode( char.direction ) )
	# 13
	gump.addText( 113, 400, "Invulnerable:", 0x834 )
	gump.addResizeGump( 280, 400, 0xBB8, 215, 20 )
	gump.addInputField( 284, 400, 200, 16, 0x834, 13, unicode( char.invulnerable ) )


	# next page
	if not char.npc:
		gump.addText( 415, 450, "Page 1 of 4", 0x834 )
	else:
		gump.addText( 415, 450, "Page 1 of 5", 0x834 )
	gump.addPageButton( 390, 450, 0x0FA, 0x0FA, 2 )

	gump.startPage( 2 )

	# 14
	gump.addText( 113, 160, "Strength:", 0x834 )
	gump.addResizeGump( 280, 160, 0xBB8, 215, 20 )
	gump.addInputField( 284, 160, 200, 16, 0x834, 14, unicode( char.strength ) )
	# 15
	gump.addText( 113, 180, "Dexterity:", 0x834 )
	gump.addResizeGump( 280, 180, 0xBB8, 215, 20 )
	gump.addInputField( 284, 180, 200, 16, 0x834, 15, unicode( char.dexterity ) )
	# 16
	gump.addText( 113, 200, "Intelligence:", 0x834 )
	gump.addResizeGump( 280, 200, 0xBB8, 215, 20 )
	gump.addInputField( 284, 200, 200, 16, 0x834, 16, unicode( char.intelligence ) )
	# 17
	gump.addText( 113, 220, "Max Health:", 0x834 )
	gump.addResizeGump( 280, 220, 0xBB8, 215, 20 )
	gump.addInputField( 284, 220, 200, 16, 0x834, 17, unicode( char.maxhitpoints ) )
	# 18
	gump.addText( 113, 240, "Current Health:", 0x834 )
	gump.addResizeGump( 280, 240, 0xBB8, 215, 20 )
	gump.addInputField( 284, 240, 200, 16, 0x834, 18, unicode( char.hitpoints ) )
	# 19
	gump.addText( 113, 260, "Max Stamina:", 0x834 )
	gump.addResizeGump( 280, 260, 0xBB8, 215, 20 )
	gump.addInputField( 284, 260, 200, 16, 0x834, 19, unicode( char.maxstamina ) )
	# 20
	gump.addText( 113, 280, "Current Stamina:", 0x834 )
	gump.addResizeGump( 280, 280, 0xBB8, 215, 20 )
	gump.addInputField( 284, 280, 200, 16, 0x834, 20, unicode( char.stamina) )
	# 21
	gump.addText( 113, 300, "Max Mana:", 0x834 )
	gump.addResizeGump( 280, 300, 0xBB8, 215, 20 )
	gump.addInputField( 284, 300, 200, 16, 0x834, 21, unicode( char.maxmana ) )
	# 22
	gump.addText( 113, 320, "Current Mana:", 0x834 )
	gump.addResizeGump( 280, 320, 0xBB8, 215, 20 )
	gump.addInputField( 284, 320, 200, 16, 0x834, 22, unicode( char.mana ) )
	# 23
	gump.addText( 113, 340, "Hidden:", 0x834 )
	gump.addResizeGump( 280, 340, 0xBB8, 215, 20 )
	gump.addInputField( 284, 340, 200, 16, 0x834, 23, unicode( char.hidden ) )
	# 24
	gump.addText( 113, 360, "Dead:", 0x834 )
	gump.addResizeGump( 280, 360, 0xBB8, 215, 20 )
	gump.addInputField( 284, 360, 200, 16, 0x834, 24, unicode( char.dead ) )
	# 25
	gump.addText( 113, 380, "Polymorphed:", 0x834 )
	gump.addResizeGump( 280, 380, 0xBB8, 215, 20 )
	gump.addInputField( 284, 380, 200, 16, 0x834, 25, unicode( char.polymorph ) )
	# 26
	gump.addText( 113, 400, "Incognito:", 0x834 )
	gump.addResizeGump( 280, 400, 0xBB8, 215, 20 )
	gump.addInputField( 284, 400, 200, 16, 0x834, 26, unicode( char.incognito ) )


	if not char.npc:
		gump.addText( 415, 450, "Page 2 of 4", 0x834 )
	else:
		gump.addText( 415, 450, "Page 2 of 5", 0x834 )
	# prev page
	gump.addPageButton( 370, 450, 0x0FC, 0x0FC, 1 )
	# next page
	gump.addPageButton( 390, 450, 0x0FA, 0x0FA, 3 )

	gump.startPage( 3 )

	# 27
	gump.addText( 113, 160, "Hunger:", 0x834 )
	gump.addResizeGump( 280, 160, 0xBB8, 215, 20 )
	gump.addInputField( 284, 160, 200, 16, 0x834, 27, unicode( char.hunger ) )
	# 28
	gump.addText( 113, 180, "War:", 0x834 )
	gump.addResizeGump( 280, 180, 0xBB8, 215, 20 )
	gump.addInputField( 284, 180, 200, 16, 0x834, 28, unicode( char.war ) )
	# 29
	gump.addText( 113, 200, "Invisible:", 0x834 )
	gump.addResizeGump( 280, 200, 0xBB8, 215, 20 )
	gump.addInputField( 284, 200, 200, 16, 0x834, 29, unicode( char.invisible ) )
	# 30
	gump.addText( 113, 220, "Frozen:", 0x834 )
	gump.addResizeGump( 280, 220, 0xBB8, 215, 20 )
	gump.addInputField( 284, 220, 200, 16, 0x834, 30, unicode( char.frozen ) )
	# 31
	gump.addText( 113, 240, "Stealthed Steps:", 0x834 )
	gump.addResizeGump( 280, 240, 0xBB8, 215, 20 )
	gump.addInputField( 284, 240, 200, 16, 0x834, 31, unicode( char.stealthedsteps) )
	# 32
	gump.addText( 113, 260, "Strength Modifier:", 0x834 )
	gump.addResizeGump( 280, 260, 0xBB8, 215, 20 )
	gump.addInputField( 284, 260, 200, 16, 0x834, 32, unicode( char.strength2 ) )
	# 33
	gump.addText( 113, 280, "Dexterity Modifier:", 0x834 )
	gump.addResizeGump( 280, 280, 0xBB8, 215, 20 )
	gump.addInputField( 284, 280, 200, 16, 0x834, 33, unicode( char.dexterity2 ) )
	# 34
	gump.addText( 113, 300, "Intelligence Modifier:", 0x834 )
	gump.addResizeGump( 280, 300, 0xBB8, 215, 20 )
	gump.addInputField( 284, 300, 200, 16, 0x834, 34, unicode( char.intelligence2 ) )
	# 35
	gump.addText( 113, 320, "Speech Color:", 0x834 )
	gump.addResizeGump( 280, 320, 0xBB8, 215, 20 )
	gump.addInputField( 284, 320, 200, 16, 0x834, 35, unicode( char.saycolor ) )
	# 36
	gump.addText( 113, 340, "Emote Color:", 0x834 )
	gump.addResizeGump( 280, 340, 0xBB8, 215, 20 )
	gump.addInputField( 284, 340, 200, 16, 0x834, 36, unicode( char.emotecolor ) )
	# 37
	gump.addText( 113, 360, "Gender (male,female):", 0x834 )
	gump.addResizeGump( 280, 360, 0xBB8, 215, 20 )
	if char.gender:
		if char.gender == 0:
			gender = "male"
		elif char.gender == 1:
			gender = "female"
		else:
			gender = "male"
	else:
		gender = "male"
	gump.addInputField( 284, 360, 200, 16, 0x834, 37, unicode( gender ) )
	if not char.npc:
		# 38
		gump.addText( 113, 380, "Account:", 0x834 )
		gump.addResizeGump( 280, 380, 0xBB8, 215, 20 )
		gump.addInputField( 284, 380, 200, 16, 0x834, 38, unicode( char.account.name ) )
		# 39
		gump.addText( 113, 400, "Visual Range:", 0x834 )
		gump.addResizeGump( 280, 400, 0xBB8, 215, 20 )
		gump.addInputField( 284, 400, 200, 16, 0x834, 39, unicode( char.visrange ) )
	elif char.npc:
		# 38
		gump.addText( 113, 380, "Owner (Hex Serial):", 0x834 )
		gump.addResizeGump( 280, 380, 0xBB8, 215, 20 )
		gump.addInputField( 284, 380, 200, 16, 0x834, 38, unicode( char.owner ) )
		# 39
		gump.addText( 113, 400, "Spawn Region:", 0x834 )
		gump.addResizeGump( 280, 400, 0xBB8, 215, 20 )
		gump.addInputField( 284, 400, 200, 16, 0x834, 39, unicode( char.spawnregion ) )

	if not char.npc:
		gump.addText( 415, 450, "Page 3 of 4", 0x834 )
	else:
		gump.addText( 415, 450, "Page 3 of 5", 0x834 )
	# next page
	gump.addPageButton( 390, 450, 0x0FA, 0x0FA, 4 )
	# prev page
	gump.addPageButton( 370, 450, 0x0FC, 0x0FC, 2 )

	gump.startPage( 4 )

	if not char.npc:
		# 40
		gump.addText( 113, 160, "Light Bonus:", 0x834 )
		gump.addResizeGump( 280, 160, 0xBB8, 215, 20 )
		gump.addInputField( 284, 160, 200, 16, 0x834, 40, unicode( char.lightbonus ) )
		# 41
		gump.addText( 113, 180, "Profile:", 0x834 )
		gump.addResizeGump( 280, 180, 0xBB8, 215, 20 )
		gump.addInputField( 284, 180, 200, 16, 0x834, 41, char.profile)
		# 43
		gump.addText( 113, 220, "Notoriety:", 0x834 )
		gump.addResizeGump( 280, 220, 0xBB8, 215, 20 )
		if char.hastag('notoriety'):
			gump.addInputField( 284, 200, 200, 16, 0x834, 42, unicode( char.gettag('notoriety') ) )
		else:
			gump.addInputField( 284, 200, 200, 16, 0x834, 42, '' )
		# 44
		gump.addText( 113, 240, "Poisoned:", 0x834 )
		gump.addResizeGump( 280, 240, 0xBB8, 215, 20 )
		if char.hastag('poisoned'):
			gump.addInputField( 284, 200, 200, 16, 0x834, 42, unicode( char.gettag('poisoned') ) )
		else:
			gump.addInputField( 284, 200, 200, 16, 0x834, 42, '' )
		# 45
		#gump.addText( 113, 260, ":", 0x834 )
		#gump.addResizeGump( 280, 260, 0xBB8, 215, 20 )
		#gump.addInputField( 284, 260, 200, 16, 0x834, 45, '' )
		# 46
		#gump.addText( 113, 280, ":", 0x834 )
		#gump.addResizeGump( 280, 280, 0xBB8, 215, 20 )
		#gump.addInputField( 284, 280, 200, 16, 0x834, 46, '' )
		# 47
		#gump.addText( 113, 300, ":", 0x834 )
		#gump.addResizeGump( 280, 300, 0xBB8, 215, 20 )
		#gump.addInputField( 284, 300, 200, 16, 0x834, 47, '' )
		# 48
		#gump.addText( 113, 320, ":", 0x834 )
		#gump.addResizeGump( 280, 320, 0xBB8, 215, 20 )
		#gump.addInputField( 284, 320, 200, 16, 0x834, 48, '' )
		# 49
		#gump.addText( 113, 340, ":", 0x834 )
		#gump.addResizeGump( 280, 340, 0xBB8, 215, 20 )
		#gump.addInputField( 284, 340, 200, 16, 0x834, 49, '' )

		gump.addText( 415, 450, "Page 4 of 4", 0x834 )
		# next page
		#gump.addPageButton( 390, 450, 0x0FA, 0x0FA, 5 )
		# prev page
		gump.addPageButton( 370, 450, 0x0FC, 0x0FC, 3 )

	elif char.npc:
		# 40
		gump.addText( 113, 160, "Carve:", 0x834 )
		gump.addResizeGump( 280, 160, 0xBB8, 215, 20 )
		gump.addInputField( 284, 160, 200, 16, 0x834, 40, char.carve)
		# 41
		gump.addText( 113, 180, "Loot List:", 0x834 )
		gump.addResizeGump( 280, 180, 0xBB8, 215, 20 )
		gump.addInputField( 284, 180, 200, 16, 0x834, 41, char.lootlist)
		#42
		gump.addText( 113, 200, "Required Taming:", 0x834 )
		gump.addResizeGump( 280, 200, 0xBB8, 215, 20 )
		gump.addInputField( 284, 200, 200, 16, 0x834, 42, unicode( char.totame ) )
		#43
		gump.addText( 113, 220, "Min Damage:", 0x834 )
		gump.addResizeGump( 280, 220, 0xBB8, 215, 20 )
		if char.hastag('mindamage'):
			gump.addInputField( 284, 220, 200, 16, 0x834, 43, unicode( char.gettag('mindamage') ) )
		else:
			gump.addInputField( 284, 220, 200, 16, 0x834, 43, '' )
		# 44
		gump.addText( 113, 240, "Max Damage:", 0x834 )
		gump.addResizeGump( 280, 240, 0xBB8, 215, 20 )
		if char.hastag('maxdamage'):
			gump.addInputField( 284, 240, 200, 16, 0x834, 44, unicode( char.gettag('maxdamage') ) )
		else:
			gump.addInputField( 284, 240, 200, 16, 0x834, 44, '' )
		# 45
		gump.addText( 113, 260, "Physical Damage:", 0x834 )
		gump.addResizeGump( 280, 260, 0xBB8, 215, 20 )
		if char.hastag('dmg_physical'):
			gump.addInputField( 284, 260, 200, 16, 0x834, 45, unicode( char.gettag('dmg_physical') ) )
		else:
			gump.addInputField( 284, 260, 200, 16, 0x834, 45, '' )
		# 46
		gump.addText( 113, 280, "Fire Damage:", 0x834 )
		gump.addResizeGump( 280, 280, 0xBB8, 215, 20 )
		if char.hastag('dmg_fire'):
			gump.addInputField( 284, 280, 200, 16, 0x834, 46, unicode( char.gettag('dmg_fire') ) )
		else:
			gump.addInputField( 284, 280, 200, 16, 0x834, 46, '' )
		# 47
		gump.addText( 113, 300, "Cold Damage:", 0x834 )
		gump.addResizeGump( 280, 300, 0xBB8, 215, 20 )
		if char.hastag('dmg_cold'):
			gump.addInputField( 284, 300, 200, 16, 0x834, 47, unicode( char.gettag('dmg_cold') ) )
		else:
			gump.addInputField( 284, 300, 200, 16, 0x834, 47, '' )
		# 48
		gump.addText( 113, 320, "Poison Damage:", 0x834 )
		gump.addResizeGump( 280, 320, 0xBB8, 215, 20 )
		if char.hastag('dmg_poison'):
			gump.addInputField( 284, 320, 200, 16, 0x834, 48, unicode( char.gettag('dmg_poison') ) )
		else:
			gump.addInputField( 284, 320, 200, 16, 0x834, 48, '' )
		# 49
		gump.addText( 113, 340, "Energy Damage:", 0x834 )
		gump.addResizeGump( 280, 340, 0xBB8, 215, 20 )
		if char.hastag('dmg_energy'):
			gump.addInputField( 284, 340, 200, 16, 0x834, 49, unicode( char.gettag('dmg_energy') ) )
		else:
			gump.addInputField( 284, 340, 200, 16, 0x834, 48, '' )
		# 50
		gump.addText( 113, 360, "Physical Resist:", 0x834 )
		gump.addResizeGump( 280, 360, 0xBB8, 215, 20 )
		if char.hastag('res_physical'):
			gump.addInputField( 284, 360, 200, 16, 0x834, 50, unicode( char.gettag('res_physical') ) )
		else:
			gump.addInputField( 284, 360, 200, 16, 0x834, 50, '' )
		# 51
		gump.addText( 113, 380, "Fire Resist:", 0x834 )
		gump.addResizeGump( 280, 380, 0xBB8, 215, 20 )
		if char.hastag('res_fire'):
			gump.addInputField( 284, 380, 200, 16, 0x834, 51, unicode( char.gettag('res_fire') ) )
		else:
			gump.addInputField( 284, 380, 200, 16, 0x834, 51, '' )
		#52
		gump.addText( 113, 400, "Cold Resist:", 0x834 )
		gump.addResizeGump( 280, 400, 0xBB8, 215, 20 )
		if char.hastag('res_cold'):
			gump.addInputField( 284, 400, 200, 16, 0x834, 52, unicode( char.gettag('res_cold') ) )
		else:
			gump.addInputField( 284, 400, 200, 16, 0x834, 52, '' )

		gump.addText( 415, 450, "Page 4 of 5", 0x834 )
		# next page
		gump.addPageButton( 390, 450, 0x0FA, 0x0FA, 5 )
		# prev page
		gump.addPageButton( 370, 450, 0x0FC, 0x0FC, 3 )

		gump.startPage( 5 )

		#53
		gump.addText( 113, 160, "Poison Resist:", 0x834 )
		gump.addResizeGump( 280, 160, 0xBB8, 215, 20 )
		if char.hastag('res_poison'):
			gump.addInputField( 284, 160, 200, 16, 0x834, 53, unicode( char.gettag('res_poison') ) )
		else:
			gump.addInputField( 284, 160, 200, 16, 0x834, 53, '' )
		# 54
		gump.addText( 113, 180, "Energy Resist:", 0x834 )
		gump.addResizeGump( 280, 180, 0xBB8, 215, 20 )
		if char.hastag('res_energy'):
			gump.addInputField( 284, 180, 200, 16, 0x834, 54, unicode( char.gettag('res_energy') ) )
		else:
			gump.addInputField( 284, 180, 200, 16, 0x834, 54, '' )

		gump.addText( 415, 450, "Page 5 of 5", 0x834 )
		# next page
		#gump.addPageButton( 390, 450, 0x0FA, 0x0FA, 6 )
		# prev page
		gump.addPageButton( 370, 450, 0x0FC, 0x0FC, 4 )

	gump.send( socket )
	return

def charinfo_response( player, args, choice ):
	socket = player.socket
	char = args[0]
	if choice.button == 0 or not char or not socket:
		return 1

	# check for rank
	#if char.rank >= player.rank and not player == char:
	#	socket.sysmessage( "You've burnt your fingers!" )
	#	return 1

	textentries = choice.text
	keys = textentries.keys()

	for key in keys:
		if key == 1:
			char.name = textentries[ key ]
		elif key == 2:
			char.title = textentries[ key ]
		elif key == 3:
			char.id = int( hex2dec( textentries[ key ] ) )
		elif key == 4:
			char.orgid = int( hex2dec( textentries[ key ] ) )
		elif key == 5:
			char.skin = int( hex2dec( textentries[ key ] ) )
		elif key == 6:
			char.orgskin = int( hex2dec( textentries[ key ] ) )
		elif key == 7:
			char.fame = int( textentries[ key ] )
		elif key == 8:
			char.karma = int( textentries[ key ] )
		elif key == 9:
			char.kills = int( textentries[ key ] )
		elif key == 10:
			char.deaths = int(  textentries[ key ] )
		elif key == 11:
			char.pos = ( textentries[ key ] )
		elif key == 12:
			char.direction = int( textentries[ key ] )
		elif key == 13:
			char.invulnerable = int(  textentries[ key ]  )
		elif key == 14:
			char.strength = int( textentries[ key ] )
		elif key == 15:
			char.dexterity = int( textentries[ key ] )
		elif key == 16:
			char.intelligence = int( textentries[ key ] )
		elif key == 17:
			char.maxhitpoints = int( textentries[ key ] )
		elif key == 18:
			char.hitpoints = int( textentries[ key ] )
		elif key == 19:
			char.maxstamina = int( textentries[ key ] )
		elif key == 20:
			char.stamina = int( textentries[ key ] )
		elif key == 21:
			char.maxmana = int( textentries[ key ] )
		elif key == 22:
			char.mana = int( textentries[ key ] )
		elif key == 23:
			char.hidden = int( textentries[ key ] )
		elif key == 24:
			char.dead = int( textentries[ key ] )
		elif key == 25:
			char.polymorph = int( textentries[ key ] )
		elif key == 26:
			char.incognito = int( textentries[ key ] )
		elif key == 27:
			char.hunger = int( textentries[ key ] )
		elif key == 28:
			char.war = int( textentries[ key ] )
		elif key == 29:
			char.invisible = int( textentries[ key ] )
		elif key == 30:
			char.frozen = int( textentries[ key ] )
		elif key == 31:
			char.stealthedsteps = int( textentries[ key ] )
		elif key == 32:
			char.strength2 = int( hex2dec( textentries[ key ] ) )
		elif key == 33:
			char.dexterity2 = int( hex2dec( textentries[ key ] ) )
		elif key == 34:
			char.intelligence2 = int( hex2dec( textentries[ key ] ) )
		elif key == 35:
			char.saycolor = int( hex2dec( textentries[ key ] ) )
		elif key == 36:
			char.emotecolor = int( hex2dec( textentries[ key ] ) )
		elif key == 37:
			if ( textentries[key] ) == "female":
				char.gender = 1
			elif ( textentries[key] ) == "male":
				char.gender = 0
			elif ( textentries[key] ) == "none":
				char.gender = 0
			elif len(textentries[key]) == 1:
				if ( int( textentries[ key ] ) < 0 ) or ( int( textentries[ key ] ) > 1 ):
					char.gender = 0
				else:
					char.gender = int( textentries[ key ] )
		elif key == 38:
			if not char.npc:
				char.account.name = textentries[ key ]
			else:
				if ( textentries[ key ] ) == 'Null' or ( textentries[ key ] ) == 'None':
					char.owner = 'None'
				else:
					char.owner = int( hex2dec( textentries[ key ] ) )
		elif key == 39:
			if not char.npc:
				char.visrange = int( textentries[ key ] )
			else:
				char.spawnregion = ( textentries[ key ] )
		elif key == 40:
			if not char.npc:
				char.lightbonus = int( textentries[ key ] )
			else:
				char.carve = ( textentries[ key ] )
		elif key == 41:
			if not char.npc:
				char.profile = textentries[ key ]
			else:
				char.lootlist = textentries[ key ]
		elif key == 42:
			if not char.npc:
				if (textentries[ key ]) == '':
					if char.hastag( 'guild' ):
						char.deltag( 'guild' )
				else:
					char.settag( 'guild', int( hex2dec( textentries[ key ] ) ) )
			else:
				char.totame = int( textentries[ key ] )
		elif key == 43:
			if not char.npc:
				if (textentries[ key ]) == '':
					if char.hastag( 'notoriety' ):
						char.deltag( 'notoriety' )
				else:
					char.settag( 'notoriety', int( textentries[ key ] ) )
			elif char.npc:
				if (textentries[ key ]) == '':
					if char.hastag( 'mindamage' ):
						char.deltag( 'mindamage' )
				else:
					char.settag( 'mindamage', int( textentries[ key ] ) )
		elif key == 44:
			if not char.npc:
				if (textentries[ key ]) == '':
					if char.hastag( 'poisoned' ):
						char.deltag( 'poisoned' )
				else:
					char.settag( 'poisoned', int( textentries[ key ] ) )
			elif char.npc:
				if (textentries[ key ]) == '':
					if char.hastag( 'maxdamage' ):
						char.deltag( 'maxdamage' )
				else:
					char.settag( 'maxdamage', int( textentries[ key ] ) )
		elif key == 45:
			if char.npc:
				if (textentries[ key ]) == '':
					if char.hastag( 'dmg_physical' ):
						char.deltag( 'dmg_physical' )
				else:
					char.settag( 'dmg_physical', int( textentries[ key ] ) )
		elif key == 46:
			if char.npc:
				if (textentries[ key ]) == '':
					if char.hastag( 'dmg_fire' ):
						char.deltag( 'dmg_fire' )
				else:
					char.settag( 'dmg_fire', int( textentries[ key ] ) )
		elif key == 47:
			if char.npc:
				if (textentries[ key ]) == '':
					if char.hastag( 'dmg_cold' ):
						char.deltag( 'dmg_cold' )
				else:
					char.settag( 'dmg_cold', int( textentries[ key ] ) )
		elif key == 48:
			if char.npc:
				if (textentries[ key ]) == '':
					if char.hastag( 'dmg_poison' ):
						char.deltag( 'dmg_poison' )
				else:
					char.settag( 'dmg_poison', int( textentries[ key ] ) )
		elif key == 49:
			if char.npc:
				if (textentries[ key ]) == '':
					if char.hastag( 'dmg_energy' ):
						char.deltag( 'dmg_energy' )
				else:
					char.settag( 'dmg_energy', int( textentries[ key ] ) )
		elif key == 50:
			if char.npc:
				if (textentries[ key ]) == '':
					if char.hastag( 'res_physical' ):
						char.deltag( 'res_physical' )
				else:
					char.settag( 'res_physical', int( textentries[ key ] ) )
		elif key == 51:
			if char.npc:
				if (textentries[ key ]) == '':
					if char.hastag( 'res_fire' ):
						char.deltag( 'res_fire' )
				else:
					char.settag( 'res_fire', int( textentries[ key ] ) )
		elif key == 52:
			if char.npc:
				if (textentries[ key ]) == '':
					if char.hastag( 'res_cold' ):
						char.deltag( 'res_cold' )
				else:
					char.settag( 'res_cold', int( textentries[ key ] ) )
		elif key == 53:
			if char.npc:
				if (textentries[ key ]) == '':
					if char.hastag( 'res_poison' ):
						char.deltag( 'res_poison' )
				else:
					char.settag( 'res_poison', int( textentries[ key ] ) )
		elif key == 54:
			if char.npc:
				if (textentries[ key ]) == '':
					if char.hastag( 'res_energy' ):
						char.deltag( 'res_energy' )
				else:
					char.settag( 'res_energy', int( textentries[ key ] ) )

	if choice.button == 1:
		charinfo( socket, char )
		char.update()

	return 1

def iteminfo( socket, item ):
	if not socket or not item:
		return 0

	gump = wolfpack.gumps.cGump()
	gump.setCallback( "commands.info.iteminfo_response" )
	gump.setArgs( [item] )

	gump.startPage(0)
	gump.addResizeGump( 64, 34, 9260, 464, 462 ) #Background
	gump.addGump(30, 33, 10421, 0)
	gump.addGump(59, 22, 10420, 0)
	gump.addGump(43, 173, 10422, 0)
	gump.addResizeGump(100, 79, 9200, 405, 65 )
	gump.addText(160, 90, "Item Properties", 380)
	gump.addText( 160, 110, "Serial: " + hex( item.serial ), 0x834 )
	gump.addTilePic( 106, 75, item.id ) # Type of info menu
	gump.addResizeGump(100, 151, 9200, 404, 283)
	gump.addGump(43, 173, 10422, 0)

	# Apply button
	gump.addButton( 102, 450, 0xEF, 0xF0, 1 )
	# OK button
	gump.addButton( 177, 450, 0xF9, 0xF8, -1 )
	# Cancel button
	gump.addButton( 252, 450, 0xF3, 0xF1, 0 )

	gump.startPage( 1 )

	# 1
	gump.addText( 113, 160, "Name:", 0x834 )
	gump.addResizeGump( 280, 160, 0xBB8, 215, 20 )
	gump.addInputField( 284, 160, 200, 16, 0x834,  1, unicode( item.name ) )
	# 2
	gump.addText( 113, 180, "ID:", 0x834 )
	gump.addResizeGump( 280, 180, 0xBB8, 215, 20 )
	gump.addInputField( 284, 180, 200, 16, 0x834,  2, hex( item.id ) )
	# 3
	gump.addText( 113, 200, "Position (x,y,z,map):", 0x834 )
	gump.addResizeGump( 280, 200, 0xBB8, 215, 20 )
	gump.addInputField( 284, 200, 200, 16, 0x834,  3, unicode( item.pos.x )+","+unicode( item.pos.y )+","+unicode( item.pos.z )+","+unicode( item.pos.map ) )
	# 4
	gump.addText( 113, 220, "Color:", 0x834 )
	gump.addResizeGump( 280, 220, 0xBB8, 215, 20 )
	gump.addInputField( 284, 220, 200, 16, 0x834,  4, hex( item.color ) )
	# 5
	gump.addText( 113, 240, "Amount:", 0x834 )
	gump.addResizeGump( 280, 240, 0xBB8, 215, 20 )
	gump.addInputField( 284, 240, 200, 16, 0x834,  5, unicode( item.amount ) )
	# 6
	gump.addText( 113, 260, "Weight:", 0x834 )
	gump.addResizeGump( 280, 260, 0xBB8, 215, 20 )
	gump.addInputField( 284, 260, 200, 16, 0x834,  6, unicode( item.weight ) )
	# 7
	gump.addText( 113, 280, "Layer:", 0x834 )
	gump.addResizeGump( 280, 280, 0xBB8, 215, 20 )
	gump.addInputField( 284, 280, 200, 16, 0x834, 7, unicode( item.layer ) )
	# 8
	gump.addText( 113, 300, "Type:", 0x834 )
	gump.addResizeGump( 280, 300, 0xBB8, 215, 20 )
	gump.addInputField( 284, 300, 200, 16, 0x834, 8, unicode( item.type ) )
	# 9
	gump.addText( 113, 320, "Max. hitpoints:", 0x834 )
	gump.addResizeGump( 280, 320, 0xBB8, 215, 20 )
	gump.addInputField( 284, 320, 200, 16, 0x834, 9, unicode( item.maxhealth ) )
	# 10
	gump.addText( 113, 340, "Hitpoints:", 0x834 )
	gump.addResizeGump( 280, 340, 0xBB8, 215, 20 )
	gump.addInputField( 284, 340, 200, 16, 0x834, 10, unicode( item.health ) )
	# 11
	gump.addText( 113, 360, "Decay:", 0x834 )
	gump.addResizeGump( 280, 360, 0xBB8, 215, 20 )
	gump.addInputField( 284, 360, 200, 16, 0x834, 11, unicode( item.decay ) )
	# 12
	gump.addText( 113, 380, "Newbie:", 0x834 )
	gump.addResizeGump( 280, 380, 0xBB8, 215, 20 )
	gump.addInputField( 284, 380, 200, 16, 0x834, 12, unicode( item.newbie ) )
	# 13
	# This should really be .movable! :P
	gump.addText( 113, 400, "Movable:", 0x834 )
	gump.addResizeGump( 280, 400, 0xBB8, 215, 20 )
	gump.addInputField( 284, 400, 200, 16, 0x834, 13, unicode( item.magic ) )

	# next page
	gump.addText( 415, 450, "Page 1 of 4", 0x834 )
	gump.addPageButton( 390, 450, 0x0FA, 0x0FA, 2 )

	# PAGE 2
	gump.startPage( 2 )

	# 14
	gump.addText( 113, 160, "Visible:", 0x834 )
	gump.addResizeGump( 280, 160, 0xBB8, 215, 20 )
	gump.addInputField( 284, 160, 200, 16, 0x834, 14, unicode( item.visible ) )
	# 15
	gump.addText( 113, 180, "Price:", 0x834 )
	gump.addResizeGump( 280, 180, 0xBB8, 215, 20 )
	if ( item.hastag( 'price' ) ):
		gump.addInputField( 284, 180, 200, 16, 0x834, 15, unicode( item.gettag( 'price' ) ) )
	else:
		gump.addInputField( 284, 180, 200, 16, 0x834, 15, '')
	# 16
	gump.addText( 113, 200, "Restock:", 0x834 )
	gump.addResizeGump( 280, 200, 0xBB8, 215, 20 )
	gump.addInputField( 284, 200, 200, 16, 0x834, 16, unicode( item.restock ) )
	# 17
	gump.addText( 113, 220, "Time unused:", 0x834 )
	gump.addResizeGump( 280, 220, 0xBB8, 215, 20 )
	if ( item.hastag( 'timeunused' ) ):
		gump.addInputField( 284, 220, 200, 16, 0x834, 17, unicode( item.gettag( 'timeunused' ) ) )
	else:
		gump.addInputField( 284, 220, 200, 16, 0x834, 17, '' )
	# 18
	gump.addText( 113, 240, "Charges:", 0x834 )
	gump.addResizeGump( 280, 240, 0xBB8, 215, 20 )
	if ( item.hastag( 'charges' ) ):
		gump.addInputField( 284, 240, 200, 16, 0x834, 18, unicode( item.gettag( 'charges' ) ) )
	else:
		gump.addInputField( 284, 240, 200, 16, 0x834, 18, '' )
	# 19
	gump.addText( 113, 260, "Max Charges:", 0x834 )
	gump.addResizeGump( 280, 260, 0xBB8, 215, 20 )
	if ( item.hastag( 'maxcharges' ) ):
		gump.addInputField( 284, 260, 200, 16, 0x834, 19, unicode( item.gettag( 'maxcharges' ) ) )
	else:
		gump.addInputField( 284, 260, 200, 16, 0x834, 19, '' )
	# 20
	gump.addText( 113, 280, "Required Strength:", 0x834 )
	gump.addResizeGump( 280, 280, 0xBB8, 215, 20 )
	if ( item.hastag( 'req_str' ) ):
		gump.addInputField( 284, 280, 200, 16, 0x834, 20, unicode( item.gettag( 'req_str' ) ) )
	else:
		gump.addInputField( 284, 280, 200, 16, 0x834, 20, '' )
	# 21
	gump.addText( 113, 300, "Required Dexterity:", 0x834 )
	gump.addResizeGump( 280, 300, 0xBB8, 215, 20 )
	if ( item.hastag( 'req_dex' ) ):
		gump.addInputField( 284, 300, 200, 16, 0x834, 21, unicode( item.gettag( 'req_dex' ) ) )
	else:
		gump.addInputField( 284, 300, 200, 16, 0x834, 21, '' )
	# 22
	gump.addText( 113, 320, "Required Intelligence:", 0x834 )
	gump.addResizeGump( 280, 320, 0xBB8, 215, 20 )
	if ( item.hastag( 'req_int' ) ):
		gump.addInputField( 284, 320, 200, 16, 0x834, 22, unicode( item.gettag( 'req_int' ) ) )
	else:
		gump.addInputField( 284, 320, 200, 16, 0x834, 22, '' )
	# 23
	gump.addText( 113, 340, "Bonus Strength:", 0x834 )
	gump.addResizeGump( 280, 340, 0xBB8, 215, 20 )
	if ( item.hastag( 'boni_str' ) ):
		gump.addInputField( 284, 340, 200, 16, 0x834, 23, unicode( item.gettag( 'boni_str' ) ) )
	else:
		gump.addInputField( 284, 340, 200, 16, 0x834, 23, '' )
	# 24
	gump.addText( 113, 360, "Bonus Dexterity:", 0x834 )
	gump.addResizeGump( 280, 360, 0xBB8, 215, 20 )
	if ( item.hastag( 'boni_dex' ) ):
		gump.addInputField( 284, 360, 200, 16, 0x834, 24, unicode( item.gettag( 'boni_dex' ) ) )
	else:
		gump.addInputField( 284, 360, 200, 16, 0x834, 24, '' )
	# 25
	gump.addText( 113, 380, "Bonus Intelligence:", 0x834 )
	gump.addResizeGump( 280, 380, 0xBB8, 215, 20 )
	if ( item.hastag( 'boni_int' ) ):
		gump.addInputField( 284, 380, 200, 16, 0x834, 25, unicode( item.gettag( 'boni_int' ) ) )
	else:
		gump.addInputField( 284, 380, 200, 16, 0x834, 25, '' )
	# 26
	gump.addText( 113, 400, "Weapon Speed:", 0x834 )
	gump.addResizeGump( 280, 400, 0xBB8, 215, 20 )
	if ( item.hastag( 'speed' ) ):
		gump.addInputField( 284, 400, 200, 16, 0x834, 26, unicode( item.gettag( 'speed' ) ) )
	else:
		gump.addInputField( 284, 400, 200, 16, 0x834, 26, '' )

	gump.addText( 415, 450, "Page 2 of 4", 0x834 )
	# prev page
	gump.addPageButton( 370, 450, 0x0FC, 0x0FC, 1 )
	# next page
	gump.addPageButton( 390, 450, 0x0FA, 0x0FA, 3 )

	# PAGE 3
	gump.startPage( 3 )

	# 27
	gump.addText( 113, 160, "Resource Name:", 0x834 )
	gump.addResizeGump( 280, 160, 0xBB8, 215, 20 )
	if ( item.hastag( 'resname' ) ):
		gump.addInputField( 284, 160, 200, 16, 0x834, 27, unicode( item.gettag( 'resname' ) ) )
	else:
		gump.addInputField( 284, 160, 200, 16, 0x834, 27, '')
	# 28
	gump.addText( 113, 180, "Resource Name 2:", 0x834 )
	gump.addResizeGump( 280, 180, 0xBB8, 215, 20 )
	if ( item.hastag( 'resname2' ) ):
		gump.addInputField( 284, 180, 200, 16, 0x834, 28, unicode( item.gettag( 'resname2' ) ) )
	else:
		gump.addInputField( 284, 180, 200, 16, 0x834, 28, '')
	# 29
	gump.addText( 113, 200, "Exceptional:", 0x834 )
	gump.addResizeGump( 280, 200, 0xBB8, 215, 20 )
	if ( item.hastag( 'exceptional' ) ):
		gump.addInputField( 284, 200, 200, 16, 0x834, 29, unicode( item.gettag( 'exceptional' ) ) )
	else:
		gump.addInputField( 284, 200, 200, 16, 0x834, 29, '')
	# 17
	gump.addText( 113, 220, "Resist Physical:", 0x834 )
	gump.addResizeGump( 280, 220, 0xBB8, 215, 20 )
	if ( item.hastag( 'res_physical' ) ):
		gump.addInputField( 284, 220, 200, 16, 0x834, 30, unicode( item.gettag( 'res_physical' ) ) )
	else:
		gump.addInputField( 284, 220, 200, 16, 0x834, 30, '' )
	#30
	gump.addText( 113, 240, "Resist Fire:", 0x834 )
	gump.addResizeGump( 280, 240, 0xBB8, 215, 20 )
	if ( item.hastag( 'res_fire' ) ):
		gump.addInputField( 284, 240, 200, 16, 0x834, 31, unicode( item.gettag( 'res_fire' ) ) )
	else:
		gump.addInputField( 284, 240, 200, 16, 0x834, 31, '' )
	# 31
	gump.addText( 113, 260, "Resist Cold:", 0x834 )
	gump.addResizeGump( 280, 260, 0xBB8, 215, 20 )
	if ( item.hastag( 'res_cold' ) ):
		gump.addInputField( 284, 260, 200, 16, 0x834, 32, unicode( item.gettag( 'res_cold' ) ) )
	else:
		gump.addInputField( 284, 260, 200, 16, 0x834, 32, '' )
	# 32
	gump.addText( 113, 280, "Resist Poison:", 0x834 )
	gump.addResizeGump( 280, 280, 0xBB8, 215, 20 )
	if ( item.hastag( 'res_poison' ) ):
		gump.addInputField( 284, 280, 200, 16, 0x834, 33, unicode( item.gettag( 'res_poison' ) ) )
	else:
		gump.addInputField( 284, 280, 200, 16, 0x834, 33, '' )
	# 33
	gump.addText( 113, 300, "Resist Energy:", 0x834 )
	gump.addResizeGump( 280, 300, 0xBB8, 215, 20 )
	if ( item.hastag( 'res_energy' ) ):
		gump.addInputField( 284, 300, 200, 16, 0x834, 34, unicode( item.gettag( 'res_energy' ) ) )
	else:
		gump.addInputField( 284, 300, 200, 16, 0x834, 34, '' )
	# 34
	gump.addText( 113, 320, "Damage Physical:", 0x834 )
	gump.addResizeGump( 280, 320, 0xBB8, 215, 20 )
	if ( item.hastag( 'dmg_physical' ) ):
		gump.addInputField( 284, 320, 200, 16, 0x834, 35, unicode( item.gettag( 'dmg_physical' ) ) )
	else:
		gump.addInputField( 284, 320, 200, 16, 0x834, 35, '' )
	# 35
	gump.addText( 113, 340, "Damage Fire:", 0x834 )
	gump.addResizeGump( 280, 340, 0xBB8, 215, 20 )
	if ( item.hastag( 'dmg_fire' ) ):
		gump.addInputField( 284, 340, 200, 16, 0x834, 36, unicode( item.gettag( 'dmg_fire' ) ) )
	else:
		gump.addInputField( 284, 340, 200, 16, 0x834, 36, '' )
	# 36
	gump.addText( 113, 360, "Damage Cold:", 0x834 )
	gump.addResizeGump( 280, 360, 0xBB8, 215, 20 )
	if ( item.hastag( 'dmg_cold' ) ):
		gump.addInputField( 284, 360, 200, 16, 0x834, 37, unicode( item.gettag( 'dmg_cold' ) ) )
	else:
		gump.addInputField( 284, 360, 200, 16, 0x834, 37, '' )
	# 37
	gump.addText( 113, 380, "Damage Poison:", 0x834 )
	gump.addResizeGump( 280, 380, 0xBB8, 215, 20 )
	if ( item.hastag( 'dmg_poison' ) ):
		gump.addInputField( 284, 380, 200, 16, 0x834, 38, unicode( item.gettag( 'dmg_poison' ) ) )
	else:
		gump.addInputField( 284, 380, 200, 16, 0x834, 38, '' )
	# 38
	gump.addText( 113, 400, "Damage Energy:", 0x834 )
	gump.addResizeGump( 280, 400, 0xBB8, 215, 20 )
	if ( item.hastag( 'dmg_energy' ) ):
		gump.addInputField( 284, 400, 200, 16, 0x834, 39, unicode( item.gettag( 'dmg_energy' ) ) )
	else:
		gump.addInputField( 284, 400, 200, 16, 0x834, 39, '' )

	gump.addText( 415, 450, "Page 3 of 4", 0x834 )
	# next page
	gump.addPageButton( 390, 450, 0x0FA, 0x0FA, 4 )
	# prev page
	gump.addPageButton( 370, 450, 0x0FC, 0x0FC, 2 )

	# PAGE 4
	gump.startPage( 4 )

	# 39
	gump.addText( 113, 160, "Min Damage:", 0x834 )
	gump.addResizeGump( 280, 160, 0xBB8, 215, 20 )
	if ( item.hastag( 'mindamage' ) ):
		gump.addInputField( 284, 160, 200, 16, 0x834, 40, unicode( item.gettag( 'mindamage' ) ) )
	else:
		gump.addInputField( 284, 160, 200, 16, 0x834, 40, '')
	# 40
	gump.addText( 113, 180, "Max Damage:", 0x834 )
	gump.addResizeGump( 280, 180, 0xBB8, 215, 20 )
	if ( item.hastag( 'maxdamage' ) ):
		gump.addInputField( 284, 180, 200, 16, 0x834, 41, unicode( item.gettag( 'maxdamage' ) ) )
	else:
		gump.addInputField( 284, 180, 200, 16, 0x834, 41, '')

	gump.addText( 415, 450, "Page 4 of 4", 0x834 )
	# prev page
	gump.addPageButton( 370, 450, 0x0FC, 0x0FC, 3 )

	gump.send( socket )

	return

def iteminfo_response( player, args, choice ):
	socket = player.socket
	item = args[0]
	if choice.button == 0 or not item or not socket:
		return 1

	textentries = choice.text
	keys = textentries.keys()

	for key in keys:
		if key == 1:
			item.name = textentries[ key ]
		elif key == 2:
			item.id = int( hex2dec( textentries[ key ] ) )
		elif key == 3:
			item.pos = unicode( textentries[key] )
		elif key == 4:
			item.color = int( hex2dec( textentries[ key ] ) )
		elif key == 5:
			item.amount = int( hex2dec( textentries[ key ] ) )
		elif key == 6:
			item.weight = float( textentries[ key ] )
		elif key == 7:
			item.layer = int( hex2dec( textentries[ key ] ) )
		elif key == 8:
			item.type = int( hex2dec( textentries[ key ] ) )
		elif key == 9:
			item.maxhealth = int( hex2dec( textentries[ key ] ) )
		elif key == 10:
			item.health = int( hex2dec( textentries[ key ] ) )
		elif key == 11:
			item.decay = int( hex2dec( textentries[ key ] ) )
		elif key == 12:
			item.newbie = int( hex2dec( textentries[ key ] ) )
		elif key == 13:
			# this should be .movable :P
			item.magic = int( hex2dec( textentries[ key ] ) )
		elif key == 14:
			item.visible = int( hex2dec( textentries[ key ] ) )
		elif key == 15:
			if ( textentries[ key ] == '' ):
				if ( item.hastag('price') ):
					item.deltag('price')
			else:
				item.settag( 'price', int( hex2dec( textentries[ key ] ) ) )
		elif key == 16:
			item.restock = int( hex2dec( textentries[ key ] ) )
		elif key == 17:
			if ( textentries[ key ] == '' ):
				if ( item.hastag('timeunused') ):
					item.deltag('timeunused')
			else:
				item.settag( 'timeunused', int( hex2dec( textentries[ key ] ) ) )
		elif key == 18:
			if ( textentries[ key ] == '' ):
				if ( item.hastag('charges') ):
					item.deltag('charges')
			else:
				item.settag( 'charges', int( hex2dec( textentries[ key ] ) ) )
		elif key == 19:
			if ( textentries[ key ] == '' ):
				if ( item.hastag('maxcharges') ):
					item.deltag('maxcharges')
			else:
				item.settag( 'maxcharges', int( hex2dec( textentries[ key ] ) ) )
		elif key == 20:
			if ( textentries[ key ] == '' ):
				if ( item.hastag('req_str') ):
					item.deltag('req_str')
			else:
				item.settag( 'req_str', int( hex2dec( textentries[ key ] ) ) )
		elif key == 21:
			if ( textentries[ key ] == '' ):
				if ( item.hastag('req_dex') ):
					item.deltag('req_dex')
			else:
				item.settag( 'req_dex', int( hex2dec( textentries[ key ] ) ) )
		elif key == 22:
			if ( textentries[ key ] == '' ):
				if ( item.hastag('req_int') ):
					item.deltag('req_int')
			else:
				item.settag( 'req_int', int( hex2dec( textentries[ key ] ) ) )
		elif key == 23:
			if ( textentries[ key ] == '' ):
				if ( item.hastag('boni_str') ):
					item.deltag('boni_str')
			else:
				item.settag( 'boni_str', int( hex2dec( textentries[ key ] ) ) )
		elif key == 24:
			if ( textentries[ key ] == '' ):
				if ( item.hastag('boni_dex') ):
					item.deltag('boni_dex')
			else:
				item.settag( 'boni_dex', int( hex2dec( textentries[ key ] ) ) )
		elif key == 25:
			if ( textentries[ key ] == '' ):
				if ( item.hastag('boni_int') ):
					item.deltag('boni_int')
			else:
				item.settag( 'boni_int', int( hex2dec( textentries[ key ] ) ) )
		elif key == 26:
			if ( textentries[ key ] == '' ):
				if ( item.hastag('speed') ):
					item.deltag('speed')
			else:
				item.settag( 'speed', int( hex2dec( textentries[ key ] ) ) )
		elif key == 27:
			if ( textentries[ key ] == '' ):
				if ( item.hastag('resname') ):
					item.deltag('resname')
			else:
				item.settag( 'resname', unicode( textentries[ key ] ) )
		elif key == 28:
			if ( textentries[ key ] == '' ):
				if ( item.hastag('resname2') ):
					item.deltag('resname2')
			else:
				item.settag( 'resname2', unicode( textentries[ key ] ) )
		elif key == 29:
			if ( textentries[ key ] == '' ):
				if ( item.hastag('exceptional') ):
					item.deltag('exceptional')
			else:
				item.settag( 'exceptional', int( hex2dec( textentries[ key ] ) ) )
		elif key == 30:
			if ( textentries[ key ] == '' ):
				if ( item.hastag('res_physical') ):
					item.deltag('res_physical')
			else:
				item.settag( 'res_physical', int( hex2dec( textentries[ key ] ) ) )
		elif key == 31:
			if ( textentries[ key ] == '' ):
				if ( item.hastag('res_fire') ):
					item.deltag('res_fire')
			else:
				item.settag( 'res_fire', int( hex2dec( textentries[ key ] ) ) )
		elif key == 32:
			if ( textentries[ key ] == '' ):
				if ( item.hastag('res_cold') ):
					item.deltag('res_cold')
			else:
				item.settag( 'res_cold', int( hex2dec( textentries[ key ] ) ) )
		elif key == 33:
			if ( textentries[ key ] == '' ):
				if ( item.hastag('res_poison') ):
					item.deltag('res_poison')
			else:
				item.settag( 'res_poison', int( hex2dec( textentries[ key ] ) ) )
		elif key == 34:
			if ( textentries[ key ] == '' ):
				if ( item.hastag('res_energy') ):
					item.deltag('res_energy')
			else:
				item.settag( 'res_energy', int( hex2dec( textentries[ key ] ) ) )
		elif key == 35:
			if ( textentries[ key ] == '' ):
				if ( item.hastag('dmg_physical') ):
					item.deltag('dmg_physical')
			else:
				item.settag( 'dmg_physical', int( hex2dec( textentries[ key ] ) ) )
		elif key == 36:
			if ( textentries[ key ] == '' ):
				if ( item.hastag('dmg_fire') ):
					item.deltag('dmg_fire')
			else:
				item.settag( 'dmg_fire', int( hex2dec( textentries[ key ] ) ) )
		elif key == 37:
			if ( textentries[ key ] == '' ):
				if ( item.hastag('dmg_cold') ):
					item.deltag('dmg_cold')
			else:
				item.settag( 'dmg_cold', int( hex2dec( textentries[ key ] ) ) )
		elif key == 38:
			if ( textentries[ key ] == '' ):
				if ( item.hastag('dmg_poison') ):
					item.deltag('dmg_poison')
			else:
				item.settag( 'dmg_poison', int( hex2dec( textentries[ key ] ) ) )
		elif key == 39:
			if ( textentries[ key ] == '' ):
				if ( item.hastag('dmg_energy') ):
					item.deltag('dmg_energy')
			else:
				item.settag( 'dmg_energy', int( hex2dec( textentries[ key ] ) ) )
		elif key == 40:
			if ( textentries[ key ] == '' ):
				if ( item.hastag('mindamage') ):
					item.deltag('mindamage')
			else:
				item.settag( 'mindamage', int( hex2dec( textentries[ key ] ) ) )
		elif key == 41:
			if ( textentries[ key ] == '' ):
				if ( item.hastag('maxdamage') ):
					item.deltag('maxdamage')
			else:
				item.settag( 'maxdamage', int( hex2dec( textentries[ key ] ) ) )

	if choice.button == 1:
		iteminfo( socket, item )
		item.update()

	return 1

