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
from wolfpack.gumps import cGump
from wolfpack.utilities import *
from wolfpack import *

def str2bool( str ):
	if str == "1" or str.upper() == "TRUE":
		return True
	return False

def info( socket, command, argstring ):
	#args = argstring.split(" ")
	#if len(argstring) > 0:
		# TODO: implement spawnregion info gump and more here
		#return True

	socket.attachtarget( "commands.info.infotargetresponse" )
	return True

def onLoad():
	wolfpack.registercommand( "info", info )

def infotargetresponse( char, args, target ):
	if not char.socket:
		return False

	# map target
	if target.char:
		charinfo( char.socket, target.char )
		return True
	elif target.item:
		iteminfo( char.socket, target.item )
		return True
	elif target.model == 0:
		map = wolfpack.map( target.pos.x, target.pos.y, target.pos.map )
		tile = wolfpack.landdata( map['id'] )
		maptileinfo( char.socket, map, tile )
		return True
	elif target.model != 0:
		tile = wolfpack.tiledata( target.model )
		statictileinfo( char.socket, target.model, target.pos, tile )
		return True
	return True

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
	# Find the given static tile
	statics = wolfpack.statics(pos.x, pos.y, pos.map, True)
	color = 0
	
	for stile in statics:	
		if stile['id'] == model and stile['z'] == pos.z:
			color = stile['color']
	
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
	
	gump.addText( 50, 240, "Color : 0x%x" % color, 0x834 )

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
	gump.setArgs( [char.serial] )

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
	gump.addInputField( 284, 360, 200, 16, 0x834, 11, unicode( char.pos.x )+","+unicode( char.pos.y )+","+unicode( char.pos.z )+","+unicode( char.pos.map ) )
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
		gump.addText( 284, 380, unicode( char.account.name ), 0x834 )
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
		#gump.addText( 113, 400, "Spawn Region:", 0x834 )
		#gump.addResizeGump( 280, 400, 0xBB8, 215, 20 )
		#gump.addInputField( 284, 400, 200, 16, 0x834, 39, unicode( char.spawnregion ) )

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
		#gump.addText( 113, 220, "Notoriety:", 0x834 )
		#gump.addResizeGump( 280, 220, 0xBB8, 215, 20 )
		#if char.hastag('notoriety'):
		#	gump.addInputField( 284, 200, 200, 16, 0x834, 43, unicode( char.gettag('notoriety') ) )
		#else:
		#	gump.addInputField( 284, 200, 200, 16, 0x834, 43, '' )
		# 44
		gump.addText( 113, 240, "Poison:", 0x834 )
		gump.addResizeGump( 280, 240, 0xBB8, 215, 20 )
		gump.addInputField( 284, 240, 200, 16, 0x834, 44, unicode( char.poison ) )
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
		#gump.addText( 113, 160, "Carve:", 0x834 )
		#gump.addResizeGump( 280, 160, 0xBB8, 215, 20 )
		#gump.addInputField( 284, 160, 200, 16, 0x834, 40, char.carve)
		# 41
		#gump.addText( 113, 180, "Loot List:", 0x834 )
		#gump.addResizeGump( 280, 180, 0xBB8, 215, 20 )
		#gump.addInputField( 284, 180, 200, 16, 0x834, 41, char.lootpacks)
		# 43
		gump.addText( 113, 180, "Mindamage:", 0x834 )
		#gump.addResizeGump( 280, 180, 0xBB8, 215, 20 )
		gump.addText( 284, 180, unicode( char.mindamage ), 0x834 )
		# 44
		gump.addText( 113, 200, "Maxdamage:", 0x834 )
		#gump.addResizeGump( 280, 200, 0xBB8, 215, 20 )
		gump.addText( 284, 200, unicode( char.maxdamage ), 0x834 )
		# 45 Passed
		gump.addText( 113, 220, "Physical Damage:", 0x834 )
		#gump.addResizeGump( 280, 220, 0xBB8, 215, 20 )
		physicaltotal = 100
		if char.hastag('dmg_fire'):
			physicaltotal -= int(char.gettag('dmg_fire'))
		if char.hastag('dmg_cold'):
			physicaltotal -= int(char.gettag('dmg_cold'))
		if char.hastag('dmg_poison'):
			physicaltotal -= int(char.gettag('dmg_poison'))
		if char.hastag('dmg_energy'):
			physicaltotal -= int(char.gettag('dmg_energy'))
		gump.addText( 284, 220, unicode( physicaltotal ), 0x834 )
		# 46
		gump.addText( 113, 240, "Fire Damage:", 0x834 )
		gump.addResizeGump( 280, 240, 0xBB8, 215, 20 )
		if char.hastag('dmg_fire'):
			gump.addInputField( 284, 240, 200, 16, 0x834, 46, unicode( char.gettag('dmg_fire') ) )
		else:
			gump.addInputField( 284, 240, 200, 16, 0x834, 46, '0' )
		# 47
		gump.addText( 113, 260, "Cold Damage:", 0x834 )
		gump.addResizeGump( 280, 260, 0xBB8, 215, 20 )
		if char.hastag('dmg_cold'):
			gump.addInputField( 284, 260, 200, 16, 0x834, 47, unicode( char.gettag('dmg_cold') ) )
		else:
			gump.addInputField( 284, 260, 200, 16, 0x834, 47, '0' )
		# 48
		gump.addText( 113, 280, "Poison Damage:", 0x834 )
		gump.addResizeGump( 280, 280, 0xBB8, 215, 20 )
		if char.hastag('dmg_poison'):
			gump.addInputField( 284, 280, 200, 16, 0x834, 48, unicode( char.gettag('dmg_poison') ) )
		else:
			gump.addInputField( 284, 280, 200, 16, 0x834, 48, '0' )
		# 49
		gump.addText( 113, 300, "Energy Damage:", 0x834 )
		gump.addResizeGump( 280, 300, 0xBB8, 215, 20 )
		if char.hastag('dmg_energy'):
			gump.addInputField( 284, 300, 200, 16, 0x834, 49, unicode( char.gettag('dmg_energy') ) )
		else:
			gump.addInputField( 284, 300, 200, 16, 0x834, 49, '0' )
		# 50
		gump.addText( 113, 320, "Physical Resist:", 0x834 )
		gump.addResizeGump( 280, 320, 0xBB8, 215, 20 )
		if char.hastag('res_physical'):
			gump.addInputField( 284, 320, 200, 16, 0x834, 50, unicode( char.gettag('res_physical') ) )
		else:
			gump.addInputField( 284, 320, 200, 16, 0x834, 50, '0' )
		# 51
		gump.addText( 113, 340, "Fire Resist:", 0x834 )
		gump.addResizeGump( 280, 340, 0xBB8, 215, 20 )
		if char.hastag('res_fire'):
			gump.addInputField( 284, 340, 200, 16, 0x834, 51, unicode( char.gettag('res_fire') ) )
		else:
			gump.addInputField( 284, 340, 200, 16, 0x834, 51, '0' )
		#52
		gump.addText( 113, 360, "Cold Resist:", 0x834 )
		gump.addResizeGump( 280, 360, 0xBB8, 215, 20 )
		if char.hastag('res_cold'):
			gump.addInputField( 284, 360, 200, 16, 0x834, 52, unicode( char.gettag('res_cold') ) )
		else:
			gump.addInputField( 284, 360, 200, 16, 0x834, 52, '0' )
		#53
		gump.addText( 113, 380, "Poison Resist:", 0x834 )
		gump.addResizeGump( 280, 380, 0xBB8, 215, 20 )
		if char.hastag('res_poison'):
			gump.addInputField( 284, 380, 200, 16, 0x834, 53, unicode( char.gettag('res_poison') ) )
		else:
			gump.addInputField( 284, 380, 200, 16, 0x834, 53, '0' )
		# 54
		gump.addText( 113, 400, "Energy Resist:", 0x834 )
		gump.addResizeGump( 280, 400, 0xBB8, 215, 20 )
		if char.hastag('res_energy'):
			gump.addInputField( 284, 400, 200, 16, 0x834, 54, unicode( char.gettag('res_energy') ) )
		else:
			gump.addInputField( 284, 400, 200, 16, 0x834, 54, '0' )

		gump.addText( 415, 450, "Page 4 of 4", 0x834 )
		# next page
		#gump.addPageButton( 390, 450, 0x0FA, 0x0FA, 5 )
		# prev page
		gump.addPageButton( 370, 450, 0x0FC, 0x0FC, 3 )

		#gump.startPage( 5 )
		#gump.addText( 415, 450, "Page 5 of 5", 0x834 )
		# next page
		#gump.addPageButton( 390, 450, 0x0FA, 0x0FA, 6 )
		# prev page
		#gump.addPageButton( 370, 450, 0x0FC, 0x0FC, 4 )

	gump.send( socket )
	return

def charinfo_response( player, args, choice ):
	socket = player.socket
	char = wolfpack.findchar(args[0])
	if choice.button == 0 or not char or not socket:
		return True

	# check for rank
	#if char.rank >= player.rank and not player == char:
	#	socket.sysmessage( "You've burnt your fingers!" )
	#	return True
	
	textentries = choice.text
	keys = textentries.keys()
		
	for key in keys:
		if key == 1:
			if char.name != textentries[1]:
				player.log(LOG_MESSAGE, "Changing 'name' for character 0x%x from '%s' to '%s'.\n" % ( char.serial, char.name, textentries[key] ) )
				char.name = textentries[ key ]
		elif key == 2:
			value = textentries[2]
			if char.title != value:
				player.log(LOG_MESSAGE, "Changing 'title' for character 0x%x from '%s' to '%s'.\n" % ( char.serial, char.title, textentries[key] ) )
				char.title = textentries[ key ]
		elif key == 3:
			value = int(hex2dec(textentries[key]))
			if char.id != value:
				player.log(LOG_MESSAGE, "Changing 'id' for character 0x%x from 0x%x to 0x%x.\n" % ( char.serial, char.id, value ) )
				char.id = int( hex2dec( textentries[ key ] ) )
		elif key == 4:
			value = int(hex2dec(textentries[key]))
			if char.orgid != value:
				player.log(LOG_MESSAGE, "Changing 'orgid' for character 0x%x from 0x%x to 0x%x.\n" % ( char.serial, char.orgid, value ) )
				char.orgid = int( hex2dec( textentries[ key ] ) )
		elif key == 5:
			value = int(hex2dec(textentries[key]))
			if char.skin != value:
				player.log(LOG_MESSAGE, "Changing 'skin' for character 0x%x from 0x%x to 0x%x.\n" % ( char.serial,  char.skin, value ) )
				char.skin = int( hex2dec( textentries[ key ] ) )
		elif key == 6:
			value = int(hex2dec(textentries[key]))
			if char.orgskin != value:
				player.log(LOG_MESSAGE, "Changing 'orgskin' for character 0x%x from 0x%x to 0x%x.\n" % ( char.serial,  char.orgskin, value ) )
				char.orgskin = int( hex2dec( textentries[ key ] ) )
		elif key == 7:
			value = int(hex2dec(textentries[key]))
			if char.fame != value:
				player.log(LOG_MESSAGE, "Changing 'fame' for character 0x%x from %i to %i.\n" % ( char.serial,  char.fame, value ) )
				char.fame = int( textentries[ key ] )
		elif key == 8:
			value = int(hex2dec(textentries[key]))
			if char.karma != value:
				player.log(LOG_MESSAGE, "Changing 'karma' for character 0x%x from %i to %i.\n" % ( char.serial,  char.karma, value ) )
				char.karma = int( textentries[ key ] )
		elif key == 9:
			value = int(hex2dec(textentries[key]))
			if char.kills != value:
				player.log(LOG_MESSAGE, "Changing 'kills' for character 0x%x from %u to %u.\n" % ( char.serial,  char.kills, value ) )
				char.kills = int( textentries[ key ] )
		elif key == 10:
			value = int(hex2dec(textentries[key]))
			if char.deaths != value:
				player.log(LOG_MESSAGE, "Changing 'deaths' for character 0x%x from %u to %u.\n" % ( char.serial,  char.deaths, value ) )
				char.deaths = int(  textentries[ key ] )
		elif key == 11:
			value = textentries[key]
			if str(char.pos) != value:
				player.log(LOG_MESSAGE, "Changing 'position' for character 0x%x from %s to %s.\n" % ( char.serial,  str(char.pos), value ) )
				char.pos = value
		elif key == 12:
			value = int(hex2dec(textentries[key]))
			if char.direction != value:
				player.log(LOG_MESSAGE, "Changing 'direction' for character 0x%x from %u to %u.\n" % ( char.serial,  char.direction, value ) )
				char.direction = int( textentries[ key ] )
		elif key == 13:
			value = str2bool(textentries[key])
			if char.invulnerable != value:
				player.log(LOG_MESSAGE, "Changing 'invulnerability' for character 0x%x from %s to %s.\n" % ( char.serial,  str(char.invulnerable), str(value) ) )
				char.invulnerable = value
		elif key == 14:
			value = int(hex2dec(textentries[key]))
			if char.strength != value:
				player.log(LOG_MESSAGE, "Changing 'strength' for character 0x%x from %u to %u.\n" % ( char.serial,  char.strength, value ) )
				char.strength = int( textentries[ key ] )
		elif key == 15:
			value = int(hex2dec(textentries[key]))
			if char.dexterity != value:
				player.log(LOG_MESSAGE, "Changing 'dexterity' for character 0x%x from %u to %u.\n" % ( char.serial,  char.dexterity, value ) )
				char.dexterity = value
		elif key == 16:
			value = int(hex2dec(textentries[key]))
			if char.intelligence != value:
				player.log(LOG_MESSAGE, "Changing 'intelligence' for character 0x%x from %u to %u.\n" % ( char.serial,  char.intelligence, value ) )
				char.intelligence = value
		elif key == 17:
			value = int(hex2dec(textentries[key]))
			if char.maxhitpoints != value:
				player.log(LOG_MESSAGE, "Changing 'maxhitpoints' for character 0x%x from %u to %u.\n" % ( char.serial,  char.maxhitpoints, value ) )
				char.maxhitpoints = value
		elif key == 18:
			value = int(hex2dec(textentries[key]))
			if char.hitpoints != value:
				player.log(LOG_MESSAGE, "Changing 'hitpoints' for character 0x%x from %u to %u.\n" % ( char.serial,  char.hitpoints, value ) )
				char.hitpoints = value
		elif key == 19:
			value = int(hex2dec(textentries[key]))
			if char.maxstamina != value:
				player.log(LOG_MESSAGE, "Changing 'maxstamina' for character 0x%x from %u to %u.\n" % ( char.serial,  char.maxstamina, value ) )
				char.maxstamina = value
		elif key == 20:
			value = int(hex2dec(textentries[key]))
			if char.stamina != value:
				player.log(LOG_MESSAGE, "Changing 'stamina' for character 0x%x from %u to %u.\n" % ( char.serial,  char.stamina, value ) )
				char.stamina = value
		elif key == 21:
			value = int(hex2dec(textentries[key]))
			if char.maxmana != value:
				player.log(LOG_MESSAGE, "Changing 'maxmana' for character 0x%x from %u to %u.\n" % ( char.serial,  char.maxmana, value ) )
				char.maxmana = value
		elif key == 22:
			value = int(hex2dec(textentries[key]))
			if char.mana != value:
				player.log(LOG_MESSAGE, "Changing 'mana' for character 0x%x from %u to %u.\n" % ( char.serial,  char.mana, value ) )
				char.mana = value
		elif key == 23:
			value = str2bool( textentries[ key ] )
			if char.hidden != value:
				player.log(LOG_MESSAGE, "Changing 'hidden' for character 0x%x from %s to %s.\n" % ( char.serial,  str(char.hidden), str(value) ) )
				char.hidden = value
		elif key == 24:
			value = str2bool( textentries[ key ] )
			if char.dead != value:
				player.log(LOG_MESSAGE, "Changing 'dead' for character 0x%x from %s to %s.\n" % ( char.serial,  str(char.dead), str(value) ) )
				char.dead = value
		elif key == 25:
			value = str2bool( textentries[ key ] )
			if char.polymorph != value:
				player.log(LOG_MESSAGE, "Changing 'polymorph' for character 0x%x from %s to %s.\n" % ( char.serial,  str(char.polymorph), str(value) ) )
				char.polymorph = str2bool( textentries[ key ] )
		elif key == 26:
			value = str2bool( textentries[ key ] )
			if char.incognito != value:
				player.log(LOG_MESSAGE, "Changing 'incognito' for character 0x%x from %s to %s.\n" % ( char.serial,  str(char.incognito), str(value) ) )
				char.incognito = str2bool( textentries[ key ] )
		elif key == 27:
			value = int(hex2dec(textentries[key]))
			if char.hunger != value:
				player.log(LOG_MESSAGE, "Changing 'hunger' for character 0x%x from %i to %i.\n" % ( char.serial, char.hunger, value ) )
				char.hunger = int( hex2dec( textentries[ key ] ) )
		elif key == 28:
			value = str2bool( textentries[ key ] )
			if char.war != value:
				player.log(LOG_MESSAGE, "Changing 'war' for character 0x%x from %s to %s.\n" % ( char.serial,  str(char.war), str(value) ) )
				char.war = str2bool( textentries[ key ] )
		elif key == 29:
			value = str2bool( textentries[ key ] )
			if char.invisible != value:
				player.log(LOG_MESSAGE, "Changing 'invisible' for character 0x%x from %s to %s.\n" % ( char.serial,  str(char.invisible), str(value) ) )
				char.invisible = str2bool( textentries[ key ] )
		elif key == 30:
			value = str2bool( textentries[ key ] )
			if char.frozen != value:
				player.log(LOG_MESSAGE, "Changing 'frozen' for character 0x%x from %s to %s.\n" % ( char.serial,  str(char.frozen), str(value) ) )
				char.frozen = str2bool( textentries[ key ] )
		elif key == 31:
			value = int(hex2dec(textentries[key]))
			if char.stealthedsteps != value:
				player.log(LOG_MESSAGE, "Changing 'stealthedsteps' for character 0x%x from %i to %i.\n" % ( char.serial, char.stealthedsteps, value ) )
				char.stealthedsteps = int( hex2dec( textentries[ key ] ) )
		elif key == 32:
			value = int(hex2dec(textentries[key]))
			if char.strength2 != value:
				player.log(LOG_MESSAGE, "Changing 'strength2' for character 0x%x from %i to %i.\n" % ( char.serial, char.strength2, value ) )
				char.strength2 = int( hex2dec( textentries[ key ] ) )
		elif key == 33:
			value = int(hex2dec(textentries[key]))
			if char.dexterity2 != value:
				player.log(LOG_MESSAGE, "Changing 'dexterity2' for character 0x%x from %i to %i.\n" % ( char.serial, char.dexterity2, value ) )
				char.dexterity2 = int( hex2dec( textentries[ key ] ) )
		elif key == 34:
			value = int(hex2dec(textentries[key]))
			if char.intelligence2 != value:
				player.log(LOG_MESSAGE, "Changing 'intelligence2' for character 0x%x from %i to %i.\n" % ( char.serial, char.intelligence2, value ) )
				char.intelligence2 = int( hex2dec( textentries[ key ] ) )
		elif key == 35:
			value = int(hex2dec(textentries[key]))
			if char.saycolor != value:
				player.log(LOG_MESSAGE, "Changing 'saycolor' for character 0x%x from 0x%x to 0x%x.\n" % ( char.serial, char.saycolor, value ) )
				char.saycolor = int( hex2dec( textentries[ key ] ) )
		elif key == 36:
			value = int(hex2dec(textentries[key]))
			if char.emotecolor != value:
				player.log(LOG_MESSAGE, "Changing 'emotecolor' for character 0x%x from 0x%x to 0x%x.\n" % ( char.serial, char.emotecolor, value ) )
				char.emotecolor = int( hex2dec( textentries[ key ] ) )
		elif key == 37:
			if ( textentries[key] ) == "female":				
				value = 1
			elif ( textentries[key] ) == "male":
				value = 0
			elif ( textentries[key] ) == "none":
				value = 0
			elif len(textentries[key]) == 1:
				if ( int( textentries[ key ] ) < 0 ) or ( int( textentries[ key ] ) > 1 ):
					value = 0
				else:
					value = int( textentries[ key ] )
					
			if char.gender != value:
				player.log(LOG_MESSAGE, "Changing 'gender' for character 0x%x from %i to %i.\n" % ( char.serial, char.gender, value ) )
				char.gender = value
		elif key == 38:
			if char.npc:
				if ( textentries[ key ] ) == 'Null' or ( textentries[ key ] ) == 'None':
					value = None
				else:
					value = wolfpack.findchar( int( hex2dec( textentries[ key ] ) ) )
					
				if value != char.owner:
					player.log(LOG_MESSAGE, "Changing 'owner' for character 0x%x from %s to %s.\n" % ( char.serial, str(char.owner), str(value) ) )
					char.owner = value
					
		elif key == 39:
			if not char.npc:
				value = int(hex2dec(textentries[key]))
				if char.visrange != value:
					player.log(LOG_MESSAGE, "Changing 'visrange' for character 0x%x from %i to %i.\n" % ( char.serial, char.visrange, value ) )
					char.visrange = int( hex2dec( textentries[ key ] ) )
			#else:
			#	char.spawnregion = ( textentries[ key ] )
		elif key == 40:
			if not char.npc:
				value = int(hex2dec(textentries[key]))
				if char.lightbonus != value:
					player.log(LOG_MESSAGE, "Changing 'lightbonus' for character 0x%x from %i to %i.\n" % ( char.serial, char.lightbonus, value ) )
					char.lightbonus = int( hex2dec( textentries[ key ] ) )
			#else:
			#	char.carve = ( textentries[ key ] )
		elif key == 41:
			if not char.npc:
				value = textentries[ key ]
				if char.profile != value:
					char.profile = textentries[ key ]
				
		elif key == 42:
			pass

		elif key == 43:
			pass

		elif key == 44:
			value = int(hex2dec(textentries[key]))
			if char.poison != value:
				player.log(LOG_MESSAGE, "Changing 'poison' for character 0x%x from %i to %i.\n" % ( char.serial, char.poison, value ) )
				char.poison = value

		elif key == 46:
			if char.npc:
				if textentries[ key ] == '':
					value = 0
				else:
					value = min( 100, int( hex2dec( textentries[key] ) ) )
				
				# Get current value
				current = 0
				if char.hastag('dmg_fire'):
					current = int(char.gettag('dmg_fire'))
				
				if current != value:
					player.log(LOG_MESSAGE, "Changing tag 'dmg_fire' for character 0x%x from %i to %i.\n" % ( char.serial, current, value ) )
					if value <= 0:
						char.deltag('dmg_fire')
					else:
						char.settag('dmg_fire', value)
		elif key == 47:
			if char.npc:
				if textentries[ key ] == '':
					value = 0
				else:
					value = min( 100, int( hex2dec( textentries[key] ) ) )
				
				# Get current value
				current = 0
				if char.hastag('dmg_cold'):
					current = int(char.gettag('dmg_cold'))
				
				if current != value:
					player.log(LOG_MESSAGE, "Changing tag 'dmg_cold' for character 0x%x from %i to %i.\n" % ( char.serial, current, value ) )
					if value <= 0:
						char.deltag('dmg_cold')
					else:
						char.settag('dmg_cold', value)
		elif key == 48:
			if char.npc:
				if textentries[ key ] == '':
					value = 0
				else:
					value = min( 100, int( hex2dec( textentries[key] ) ) )
				
				# Get current value
				current = 0
				if char.hastag('dmg_poison'):
					current = int(char.gettag('dmg_poison'))
				
				if current != value:
					player.log(LOG_MESSAGE, "Changing tag 'dmg_poison' for character 0x%x from %i to %i.\n" % ( char.serial, current, value ) )
					if value <= 0:
						char.deltag('dmg_poison')
					else:
						char.settag('dmg_poison', value)
		elif key == 49:
			if char.npc:
				if textentries[ key ] == '':
					value = 0
				else:
					value = min( 100, int( hex2dec( textentries[key] ) ) )
				
				# Get current value
				current = 0
				if char.hastag('dmg_energy'):
					current = int(char.gettag('dmg_energy'))
				
				if current != value:
					player.log(LOG_MESSAGE, "Changing tag 'dmg_energy' for character 0x%x from %i to %i.\n" % ( char.serial, current, value ) )
					if value <= 0:
						char.deltag('dmg_energy')
					else:
						char.settag('dmg_energy', value)
		elif key == 50:
			if char.npc:
				if textentries[ key ] == '':
					value = 0
				else:
					value = min( 100, int( hex2dec( textentries[key] ) ) )
				
				# Get current value
				current = 0
				if char.hastag('res_physical'):
					current = int(char.gettag('res_physical'))
				
				if current != value:
					player.log(LOG_MESSAGE, "Changing tag 'res_physical' for character 0x%x from %i to %i.\n" % ( char.serial, current, value ) )
					if value <= 0:
						char.deltag('res_physical')
					else:
						char.settag('res_physical', value)
		elif key == 51:
			if char.npc:
				if textentries[ key ] == '':
					value = 0
				else:
					value = min( 100, int( hex2dec( textentries[key] ) ) )
				
				# Get current value
				current = 0
				if char.hastag('res_fire'):
					current = int(char.gettag('res_fire'))
				
				if current != value:
					player.log(LOG_MESSAGE, "Changing tag 'res_fire' for character 0x%x from %i to %i.\n" % ( char.serial, current, value ) )
					if value <= 0:
						char.deltag('res_fire')
					else:
						char.settag('res_fire', value)
		elif key == 52:
			if char.npc:
				if textentries[ key ] == '':
					value = 0
				else:
					value = min( 100, int( hex2dec( textentries[key] ) ) )
				
				# Get current value
				current = 0
				if char.hastag('res_cold'):
					current = int(char.gettag('res_cold'))
				
				if current != value:
					player.log(LOG_MESSAGE, "Changing tag 'res_cold' for character 0x%x from %i to %i.\n" % ( char.serial, current, value ) )
					if value <= 0:
						char.deltag('res_cold')
					else:
						char.settag('res_cold', value)
					
		elif key == 53:
			if char.npc:
				if textentries[ key ] == '':
					value = 0
				else:
					value = min( 100, int( hex2dec( textentries[key] ) ) )
				
				# Get current value
				current = 0
				if char.hastag('res_poison'):
					current = int(char.gettag('res_poison'))
				
				if current != value:
					player.log(LOG_MESSAGE, "Changing tag 'res_poison' for character 0x%x from %i to %i.\n" % ( char.serial, current, value ) )
					if value <= 0:
						char.deltag('res_poison')
					else:
						char.settag('res_poison', value)
		elif key == 54:
			if char.npc:
				if textentries[ key ] == '':
					value = 0
				else:
					value = min( 100, int( hex2dec( textentries[key] ) ) )
				
				# Get current value
				current = 0
				if char.hastag('res_energy'):
					current = int(char.gettag('res_energy'))
				
				if current != value:
					player.log(LOG_MESSAGE, "Changing tag 'res_energy' for character 0x%x from %i to %i.\n" % ( char.serial, current, value ) )
					if value <= 0:
						char.deltag('res_energy')
					else:
						char.settag('res_energy', value)

	if choice.button == 1:
		charinfo( socket, char )
		
	char.update()
	return True

def iteminfo( socket, item ):
	if not socket or not item:
		return False

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
	#gump.addInputField( 284, 260, 200, 16, 0x834,  6, unicode( item.weight ) )
	gump.addText( 284, 260, unicode( item.weight ), 0x834 )
	# 7
	gump.addText( 113, 280, "Layer:", 0x834 )
	gump.addResizeGump( 280, 280, 0xBB8, 215, 20 )
	gump.addInputField( 284, 280, 200, 16, 0x834, 7, unicode( item.layer ) )
	# 8
	#gump.addText( 113, 300, "Type:", 0x834 )
	#gump.addResizeGump( 280, 300, 0xBB8, 215, 20 )
	#gump.addInputField( 284, 300, 200, 16, 0x834, 8, unicode( item.type ) )
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
	gump.addInputField( 284, 400, 200, 16, 0x834, 13, unicode( item.movable ) )

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
	#gump.addText( 113, 200, "Restock:", 0x834 )
	#gump.addResizeGump( 280, 200, 0xBB8, 215, 20 )
	#gump.addInputField( 284, 200, 200, 16, 0x834, 16, unicode( item.restock ) )
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
		return True

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
		#elif key == 6:
		#	item.weight = float( textentries[ key ] )
		elif key == 7:
			item.layer = int( hex2dec( textentries[ key ] ) )
		#elif key == 8:
		#	item.type = int( hex2dec( textentries[ key ] ) )
		elif key == 9:
			item.maxhealth = int( hex2dec( textentries[ key ] ) )
		elif key == 10:
			item.health = int( hex2dec( textentries[ key ] ) )
		elif key == 11:
			item.decay = str2bool( textentries[ key ] )
		elif key == 12:
			item.newbie = int( hex2dec( textentries[ key ] ) )
		elif key == 13:
			item.movable = int( hex2dec( textentries[ key ] ) )
		elif key == 14:
			item.visible = int( hex2dec( textentries[ key ] ) )
		elif key == 15:
			if ( textentries[ key ] == '' ):
				if ( item.hastag('price') ):
					item.deltag('price')
			else:
				item.settag( 'price', int( hex2dec( textentries[ key ] ) ) )
		#elif key == 16:
		#	item.restock = int( hex2dec( textentries[ key ] ) )
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
	return True
