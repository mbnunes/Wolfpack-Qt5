import wolfpack
import string
import wolfpack.gumps
from wolfpack.gumps import *
from wolfpack.utilities import *

def onLoad():
	wolfpack.registercommand( "info", "cmdinfo" )

def onCommand( socket, command, argstring ):
	if command == "INFO":
		info( socket, command, argstring )

	return 1

def info( socket, command, argstring ):
	args = argstring.split(" ")
	if len(argstring) > 0:
		# TODO: implement spawnregion info gump and more here
		return 1
	
	socket.attachtarget( "commands.info.infotargetresponse", args )
	return 1
	
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
		tile = wolfpack.tiledata( map['id'] )
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
	gump.addText( 50, 170, "Z Height: "+str(map['z']), 0x834 )
	
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
	gump.addText( 50, 160, "Position: "+str( pos.x )+","+str( pos.y )+","+str( pos.z ), 0x834 )
	gump.addText( 50, 180, "Weight: "+str( tile['weight'] ), 0x834 )
	gump.addText( 50, 200, "Height: "+str( tile['height'] ), 0x834 )
	
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
		
	page_ = 0
	pages = 0
	if char.npc:
		pages = 5
	else:
		pages = 5

	gump = cGump( 0, 0, 0, 0, 40 )
	gump.setCallback( "commands.info.charinfo_response" )
	gump.setArgs( [char] )
	
	gump.startPage( page_ )
	gump.addResizeGump( 0, 40, 0xA28, 450, 350 ) # Background
	gump.addGump( 105, 18, 0x58B ) # Fancy top-bar
	gump.addGump( 182, 0, 0x589 ) # "Button" like gump
#	gump.addTilePic( 202, 23, creatures[ char.id ].icon ) # Type of info menu
	gump.addText( 178, 90, "Char Properties", 0x530 )

	# Apply button
	gump.addButton( 50, 340, 0xEF, 0xF0, 1 ) 
	# OK button
	gump.addButton( 120, 340, 0xF9, 0xF8, -1 ) 
	# Cancel button
	gump.addButton( 190, 340, 0xF3, 0xF1, 0 ) 

	page_ = page_ + 1
	gump.startPage( page_ )
	
	# lets begin with basechar information
	gump.addResizeGump( 195, 120, 0xBB8, 215, 20 )
	gump.addResizeGump( 195, 140, 0xBB8, 215, 20 )
	gump.addResizeGump( 195, 160, 0xBB8, 215, 20 )
	gump.addResizeGump( 195, 180, 0xBB8, 215, 20 )
	gump.addResizeGump( 195, 200, 0xBB8, 215, 20 )
	gump.addResizeGump( 195, 220, 0xBB8, 215, 20 )
#	gump.addResizeGump( 195, 240, 0xBB8, 215, 20 )
	gump.addResizeGump( 195, 260, 0xBB8, 215, 20 )
	gump.addResizeGump( 195, 280, 0xBB8, 215, 20 )
	gump.addResizeGump( 195, 300, 0xBB8, 215, 20 )

	gump.addText( 50, 120, "Name:", 0x834 )
	gump.addInputField( 200, 120, 200, 16,  1, char.name, 0x834 )
	gump.addText( 50, 140, "Title:", 0x834 )
	gump.addInputField( 200, 140, 200, 16,  2, char.title, 0x834 )
	gump.addText( 50, 160, "Body:", 0x834 )
	gump.addInputField( 200, 160, 200, 16,  3, hex( char.id ), 0x834 )
	gump.addText( 50, 180, "Org. Body:", 0x834 )
	gump.addInputField( 200, 180, 200, 16,  4, hex( char.orgid ), 0x834 )
	gump.addText( 50, 200, "Skin:", 0x834 )
	gump.addInputField( 200, 200, 200, 16,  5, hex( char.skin ), 0x834 )
	gump.addText( 50, 220, "Org. Skin:", 0x834 )
	gump.addInputField( 200, 220, 200, 16,  6, hex( char.orgskin ), 0x834 )
	gump.addText( 50, 240, "Serial:", 0x834 )
#	gump.addInputField( 200, 240, 200, 16,  7, hex( char.serial ), 0x834 )
	gump.addText( 50, 260, "Strength:", 0x834 )
	gump.addInputField( 200, 260, 200, 16,  8, str( char.strength ), 0x834 )
	gump.addText( 50, 280, "Dexterity:", 0x834 )
	gump.addInputField( 200, 280, 200, 16,  9, str( char.dexterity ), 0x834 )
	gump.addText( 50, 300, "Intelligence:", 0x834 )
	gump.addInputField( 200, 300, 200, 16,  10, str( char.intelligence ), 0x834 )

	gump.addText( 310, 340, "Page "+str( page_ )+" of "+str( pages ), 0x834 )
	# next page
	gump.addPageButton( 290, 340, 0x0FA, 0x0FA, page_+1 )

	page_ = page_ + 1
	gump.startPage( page_ )

	gump.addResizeGump( 195, 120, 0xBB8, 215, 20 )
	gump.addResizeGump( 195, 140, 0xBB8, 215, 20 )
	gump.addResizeGump( 195, 160, 0xBB8, 215, 20 )
	gump.addResizeGump( 195, 180, 0xBB8, 215, 20 )
	gump.addResizeGump( 195, 200, 0xBB8, 215, 20 )
	gump.addResizeGump( 195, 220, 0xBB8, 215, 20 )
	gump.addResizeGump( 195, 240, 0xBB8, 215, 20 )
	gump.addResizeGump( 195, 260, 0xBB8, 215, 20 )
	gump.addResizeGump( 195, 280, 0xBB8, 215, 20 )
	gump.addResizeGump( 195, 300, 0xBB8, 215, 20 )

	gump.addText( 50, 120, "Max. Hitpoints:", 0x834 )
	gump.addInputField( 200, 120, 200, 16,  11, str( char.maxhitpoints ), 0x834 )
	gump.addText( 50, 140, "Hitpoints:", 0x834 )
	gump.addInputField( 200, 140, 200, 16,  12, str( char.hitpoints ), 0x834 )
	gump.addText( 50, 160, "Max. Stamina:", 0x834 )
	gump.addInputField( 200, 160, 200, 16,  13, str( char.maxstamina ), 0x834 )
	gump.addText( 50, 180, "Stamina:", 0x834 )
	gump.addInputField( 200, 180, 200, 16,  14, str( char.stamina ), 0x834 )
	gump.addText( 50, 200, "Max. Mana:", 0x834 )
	gump.addInputField( 200, 200, 200, 16,  15, str( char.maxmana ), 0x834 )
	gump.addText( 50, 220, "Mana:", 0x834 )
	gump.addInputField( 200, 220, 200, 16,  16, str( char.mana ), 0x834 )
	gump.addText( 50, 240, "Karma:", 0x834 )
	gump.addInputField( 200, 240, 200, 16, 17, str( char.karma ), 0x834 )
	gump.addText( 50, 260, "Fame:", 0x834 )
	gump.addInputField( 200, 260, 200, 16, 18, str( char.fame ), 0x834 )
	gump.addText( 50, 280, "Kills:", 0x834 )
	gump.addInputField( 200, 280, 200, 16, 19, str( char.kills ), 0x834 )
	gump.addText( 50, 300, "Deaths:", 0x834 )
	gump.addInputField( 200, 300, 200, 16, 20, str( char.deaths ), 0x834 )

	gump.addText( 310, 340, "Page "+str( page_ )+" of "+str( pages ), 0x834 )
	# prev page
	gump.addPageButton( 270, 340, 0x0FC, 0x0FC, page_-1 )
	# next page
	gump.addPageButton( 290, 340, 0x0FA, 0x0FA, page_+1 )

	page_ = page_ + 1
	gump.startPage( page_ )

	gump.addResizeGump( 195, 120, 0xBB8, 215, 20 )
	gump.addResizeGump( 195, 140, 0xBB8, 215, 20 )
	gump.addResizeGump( 195, 160, 0xBB8, 215, 20 )
	gump.addResizeGump( 195, 180, 0xBB8, 215, 20 )
	gump.addResizeGump( 195, 200, 0xBB8, 215, 20 )
	gump.addResizeGump( 195, 220, 0xBB8, 215, 20 )
	gump.addResizeGump( 195, 240, 0xBB8, 215, 20 )
	gump.addResizeGump( 195, 260, 0xBB8, 215, 20 )
	gump.addResizeGump( 195, 280, 0xBB8, 215, 20 )
	gump.addResizeGump( 195, 300, 0xBB8, 215, 20 )

	gump.addText( 50, 120, "Defense:", 0x834 )
	gump.addInputField( 200, 120, 200, 16, 21, str( char.defense ), 0x834 )
	gump.addText( 50, 140, "Position (x,y,z,map):", 0x834 )
	gump.addInputField( 200, 140, 200, 16, 22, str( char.pos.x )+","+str( char.pos.y )+","+str( char.pos.z )+","+str( char.pos.map ), 0x834 )
	gump.addText( 50, 160, "Direction:", 0x834 )
	gump.addInputField( 200, 160, 200, 16, 23, str( char.direction ), 0x834 )
	gump.addText( 50, 180, "Hunger:", 0x834 )
	gump.addInputField( 200, 180, 200, 16, 24, str( char.hunger ), 0x834 )
	gump.addText( 50, 200, "Dead:", 0x834 )
	gump.addInputField( 200, 200, 200, 16, 25, str( char.dead ), 0x834 )
	gump.addText( 50, 220, "War:", 0x834 )
	gump.addInputField( 200, 220, 200, 16, 26, str( char.war ), 0x834 )
	gump.addText( 50, 240, "Invulnerable:", 0x834 )
	gump.addInputField( 200, 240, 200, 16, 27, str( char.invulnerable ), 0x834 )
	gump.addText( 50, 260, "Polymorphed:", 0x834 )
	gump.addInputField( 200, 260, 200, 16, 28, str( char.polymorph ), 0x834 )
	gump.addText( 50, 280, "Incognito:", 0x834 )
	gump.addInputField( 200, 280, 200, 16, 29, str( char.incognito ), 0x834 )
	gump.addText( 50, 300, "Hidden:", 0x834 )
	gump.addInputField( 200, 300, 200, 16, 30, str( char.hidden ), 0x834 )

	gump.addText( 310, 340, "Page "+str( page_ )+" of "+str( pages ), 0x834 )
	# prev page
	gump.addPageButton( 270, 340, 0x0FC, 0x0FC, page_-1 )
	# next page
	gump.addPageButton( 290, 340, 0x0FA, 0x0FA, page_+1 )

	page_ = page_ + 1
	gump.startPage( page_ )

	gump.addResizeGump( 195, 120, 0xBB8, 215, 20 )
	gump.addResizeGump( 195, 140, 0xBB8, 215, 20 )
	gump.addResizeGump( 195, 160, 0xBB8, 215, 20 )
	gump.addResizeGump( 195, 180, 0xBB8, 215, 20 )
	gump.addResizeGump( 195, 200, 0xBB8, 215, 20 )
	gump.addResizeGump( 195, 220, 0xBB8, 215, 20 )
	gump.addResizeGump( 195, 240, 0xBB8, 215, 20 )
	gump.addResizeGump( 195, 260, 0xBB8, 215, 20 )
	gump.addResizeGump( 195, 280, 0xBB8, 215, 20 )
#	gump.addResizeGump( 195, 300, 0xBB8, 215, 20 )

	gump.addText( 50, 120, "Invisible:", 0x834 )
	gump.addInputField( 200, 120, 200, 16, 31, str( char.invisible ), 0x834 )
	gump.addText( 50, 140, "Frozen:", 0x834 )
	gump.addInputField( 200, 140, 200, 16, 32, str( char.frozen ), 0x834 )
	gump.addText( 50, 160, "Stealthed Steps:", 0x834 )
	gump.addInputField( 200, 160, 200, 16, 33, str( char.stealthedsteps ), 0x834 )
	gump.addText( 50, 180, "Strength modifier:", 0x834 )
	gump.addInputField( 200, 180, 200, 16, 34, str( char.strength2 ), 0x834 )
	gump.addText( 50, 200, "Dexterity modifier:", 0x834 )
	gump.addInputField( 200, 200, 200, 16, 35, str( char.dexterity2 ), 0x834 )
	gump.addText( 50, 220, "Intelligence modifier:", 0x834 )
	gump.addInputField( 200, 220, 200, 16, 36, str( char.intelligence2 ), 0x834 )
	gump.addText( 50, 240, "Speech color:", 0x834 )
	gump.addInputField( 200, 240, 200, 16, 37, hex( char.saycolor ), 0x834 )
	gump.addText( 50, 260, "Emote color:", 0x834 )
	gump.addInputField( 200, 260, 200, 16, 38, hex( char.emotecolor ), 0x834 )
	gender = "female"
	if not char.gender:
		gender = "male"
		
	gump.addText( 50, 280, "Gender:", 0x834 )
	gump.addInputField( 200, 280, 220, 16, 39, gender, 0x834 )

	gump.addText( 310, 340, "Page "+str( page_ )+" of "+str( pages ), 0x834 )
	# prev page
	gump.addPageButton( 270, 340, 0x0FC, 0x0FC, page_-1 )
	# next page
	gump.addPageButton( 290, 340, 0x0FA, 0x0FA, page_+1 )

	page_ = page_ + 1
	gump.startPage( page_ )

	if char.npc:
		gump.addResizeGump( 195, 120, 0xBB8, 215, 20 )
		gump.addResizeGump( 195, 140, 0xBB8, 215, 20 )
		gump.addResizeGump( 195, 160, 0xBB8, 215, 20 )
		gump.addResizeGump( 195, 180, 0xBB8, 215, 20 )
		gump.addResizeGump( 195, 200, 0xBB8, 215, 20 )
		gump.addResizeGump( 195, 220, 0xBB8, 215, 20 )
		gump.addResizeGump( 195, 240, 0xBB8, 215, 20 )
		#gump.addResizeGump( 195, 260, 0xBB8, 215, 20 )
		#gump.addResizeGump( 195, 280, 0xBB8, 215, 20 )
		#gump.addResizeGump( 195, 300, 0xBB8, 215, 20 )
		
		gump.addText( 50, 120, "Carve:", 0x834 )
		gump.addInputField( 200, 120, 200, 16, 41, char.carve, 0x834 )
		gump.addText( 50, 140, "Lootlist:", 0x834 )
		gump.addInputField( 200, 140, 200, 16, 42, char.lootlist, 0x834 )
		gump.addText( 50, 160, "Spawnregion:", 0x834 )
		gump.addInputField( 200, 160, 200, 16, 43, char.spawnregion, 0x834 )
		gump.addText( 50, 180, "Min. Damage:", 0x834 )
		gump.addInputField( 200, 180, 200, 16, 44, str( char.mindamage ), 0x834 )
		gump.addText( 50, 200, "Max. Damage:", 0x834 )
		gump.addInputField( 200, 200, 200, 16, 45, str( char.maxdamage ), 0x834 )
		gump.addText( 50, 220, "Taming Min. Skill:", 0x834 )
		gump.addInputField( 200, 220, 200, 16, 46, str( char.totame ), 0x834 )
		gump.addText( 50, 240, "Owner:", 0x834 )
		ownerser = "invalid"
		if char.owner:
			ownerser = str( char.owner.serial )
		gump.addInputField( 200, 240, 200, 16, 47, ownerser, 0x834 )
	
		gump.addText( 310, 340, "Page "+str( page_ )+" of "+str( pages ), 0x834 )
		# prev page
		gump.addPageButton( 270, 340, 0x0FC, 0x0FC, page_-1 )

	else:	
		gump.addResizeGump( 195, 120, 0xBB8, 215, 20 )
		gump.addResizeGump( 195, 140, 0xBB8, 215, 20 )
		gump.addResizeGump( 195, 160, 0xBB8, 215, 20 )
		gump.addResizeGump( 195, 180, 0xBB8, 215, 20 )
		gump.addResizeGump( 195, 200, 0xBB8, 215, 20 )
		#gump.addResizeGump( 195, 220, 0xBB8, 215, 20 )
		#gump.addResizeGump( 195, 240, 0xBB8, 215, 20 )
		#gump.addResizeGump( 195, 260, 0xBB8, 215, 20 )
		#gump.addResizeGump( 195, 280, 0xBB8, 215, 20 )
		#gump.addResizeGump( 195, 300, 0xBB8, 215, 20 )
		
		gump.addText( 50, 120, "Account:", 0x834 )
		gump.addInputField( 200, 120, 200, 16, 41, char.account, 0x834 )
		gump.addText( 50, 140, "Muted:", 0x834 )
		gump.addInputField( 200, 140, 200, 16, 42, str( char.muted ), 0x834 )
		gump.addText( 50, 160, "Light Bonus:", 0x834 )
		gump.addInputField( 200, 160, 200, 16, 43, str( char.lightbonus ), 0x834 )
		gump.addText( 50, 180, "Visual Range:", 0x834 )
		gump.addInputField( 200, 180, 200, 16, 44, str( char.visrange ), 0x834 )
		gump.addText( 50, 200, "Profile:", 0x834 )
		gump.addInputField( 200, 200, 200, 16, 45, str( char.profile ), 0x834 )

		gump.addText( 310, 340, "Page "+str( page_ )+" of "+str( pages ), 0x834 )
		# prev page
		gump.addPageButton( 270, 340, 0x0FC, 0x0FC, page_-1 )
		
	gump.send( socket )
	return
	
def charinfo_response( player, args, choice ):
	socket = player.socket
	char = args[0]
	if choice.button == 0 or not char or not socket:
		return 1
	
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
		elif key == 8:
			char.strength = int( hex2dec( textentries[ key ] ) )
		elif key == 9:
			char.dexterity = int( hex2dec( textentries[ key ] ) )
		elif key == 10:
			char.intelligence = int( hex2dec( textentries[ key ] ) )
		elif key == 11:
			char.maxhitpoints = int( hex2dec( textentries[ key ] ) )
		elif key == 12:
			char.hitpoints = int( hex2dec( textentries[ key ] ) )
		elif key == 13:
			char.maxstamina = int( hex2dec( textentries[ key ] ) )
		elif key == 14:
			char.stamina = int( hex2dec( textentries[ key ] ) )
		elif key == 15:
			char.maxmana = int( hex2dec( textentries[ key ] ) )
		elif key == 16:
			char.mana = int( hex2dec( textentries[ key ] ) )
		elif key == 17:
			char.karma = int( hex2dec( textentries[ key ] ) )
		elif key == 18:
			char.fame = int( hex2dec( textentries[ key ] ) )
		elif key == 19:
			char.kills = int( hex2dec( textentries[ key ] ) )
		elif key == 20:
			char.deaths = int( hex2dec( textentries[ key ] ) )
		elif key == 21:
			char.defense = int( hex2dec( textentries[ key ] ) )
		elif key == 22:
			pos = textentries[key].split( "," )
			if len(pos) > 0:
				char.pos.x = int( pos[0] )
			if len(pos) > 1:
				char.pos.y = int( pos[1] )
			if len(pos) > 2:
				char.pos.z = int( pos[2] )
			if len(pos) > 3:
				char.pos.map = int( pos[3] )
		elif key == 23:
			char.direction = int( hex2dec( textentries[ key ] ) )
		elif key == 24:
			char.hunger = int( hex2dec( textentries[ key ] ) )
		elif key == 25:
			char.dead = int( hex2dec( textentries[ key ] ) )
		elif key == 26:
			char.war = int( hex2dec( textentries[ key ] ) )
		elif key == 27:
			char.invulnerable = int( hex2dec( textentries[ key ] ) )
		elif key == 28:
			char.polymorph = int( hex2dec( textentries[ key ] ) )
		elif key == 29:
			char.incognito = int( hex2dec( textentries[ key ] ) )
		elif key == 30:
			char.hidden = int( hex2dec( textentries[ key ] ) )
		elif key == 31:
			char.invisible = int( hex2dec( textentries[ key ] ) )
		elif key == 32:
			char.frozen = int( hex2dec( textentries[ key ] ) )
		elif key == 33:
			char.stealthedsteps = int( hex2dec( textentries[ key ] ) )
		elif key == 34:
			char.strength2 = int( hex2dec( textentries[ key ] ) )
		elif key == 35:
			char.dexterity2 = int( hex2dec( textentries[ key ] ) )
		elif key == 36:
			char.intelligence2 = int( hex2dec( textentries[ key ] ) )
		elif key == 37:
			char.saycolor = int( hex2dec( textentries[ key ] ) )
		elif key == 38:
			char.emotecolor = int( hex2dec( textentries[ key ] ) )
		elif key == 39:
			if len(textentries[key]) == 1:
				char.gender = int( hex2dec( textentries[ key ] ) )
			elif textentries[key] == "female":
				char.gender = 0
			elif textentries[key] == "male":
				char.gender = 1

		if char.npc:
			if key == 41:
				char.carve = textentries[ key ]
			elif key == 42:
				char.lootlist = textentries[ key ]
			elif key == 43:
				char.spawnregion = textentries[ key ]
			elif key == 44:
				char.mindamage = int( hex2dec( textentries[ key ] ) )
			elif key == 45:
				char.maxdamage = int( hex2dec( textentries[ key ] ) )
			elif key == 46:
				char.totame = int( hex2dec( textentries[ key ] ) )
			elif key == 47:
				char.owner = wolfpack.findChar( int( hex2dec( textentries[ key ] ) ) )
		else:
			if key == 41:
				char.account = textentries[ key ]
			elif key == 42:
				char.muted = int( hex2dec( textentries[ key ] ) )
			elif key == 43:
				char.lightbonus = int( hex2dec( textentries[ key ] ) )
			elif key == 44:
				char.visrange = int( hex2dec( textentries[ key ] ) )
			elif key == 45:
				char.profile = textentries[ key ]
				
	if choice.button == 1:
		charinfo( socket, char )
				
	return 1	

def iteminfo( socket, item ):
	if not socket or not item:
		return 0

	page_ = 0
	pages = 5

	gump = cGump( 0, 0, 0, 0, 40 )
	gump.setCallback( "commands.info.iteminfo_response" )
	gump.setArgs( [item] )

	gump.startPage( page_ )
	gump.addResizeGump( 0, 40, 0xA28, 450, 350 ) #Background
	gump.addGump( 105, 18, 0x58B ) # Fancy top-bar
	gump.addGump( 182, 0, 0x589 ) # "Button" like gump
	gump.addTilePic( 202, 23, item.id ) # Type of info menu
	gump.addText( 180, 90, "Item Properties", 0x530 )

	# Apply button
	gump.addButton( 50, 340, 0xEF, 0xF0, 1 ) 
	# OK button
	gump.addButton( 120, 340, 0xF9, 0xF8, -1 ) 
	# Cancel button
	gump.addButton( 190, 340, 0xF3, 0xF1, 0 ) 

	page_ = page_ + 1
	gump.startPage( page_ )

	gump.addResizeGump( 195, 120, 0xBB8, 215, 20 )
	gump.addResizeGump( 195, 140, 0xBB8, 215, 20 )
	gump.addResizeGump( 195, 160, 0xBB8, 215, 20 )
#	gump.addResizeGump( 195, 180, 0xBB8, 215, 20 ) serial is readonly
	gump.addResizeGump( 195, 200, 0xBB8, 215, 20 )
	gump.addResizeGump( 195, 220, 0xBB8, 215, 20 )
	gump.addResizeGump( 195, 240, 0xBB8, 215, 20 )
	gump.addResizeGump( 195, 260, 0xBB8, 215, 20 )
	gump.addResizeGump( 195, 280, 0xBB8, 215, 20 )
	gump.addResizeGump( 195, 300, 0xBB8, 215, 20 )

	gump.addText( 50, 120, "Name:", 0x834 )
	gump.addInputField( 200, 120, 200, 16,  1, item.name, 0x834 )
	gump.addText( 50, 140, "Name (identified):", 0x834 )
	gump.addInputField( 200, 140, 200, 16,  2, item.name2, 0x834 )
	gump.addText( 50, 160, "ID:", 0x834 )
	gump.addInputField( 200, 160, 200, 16,  3, hex( item.id ), 0x834 )
	gump.addText( 50, 180, "Serial:", 0x834 )
	gump.addText( 200, 180, str( item.serial ), 0x834 )
	gump.addText( 50, 200, "Position (x,y,z,map):", 0x834 )
	gump.addInputField( 200, 200, 200, 16,  5, str( item.pos.x )+","+str( item.pos.y )+","+str( item.pos.z )+","+str( item.pos.map ), 0x834 )
	gump.addText( 50, 220, "Color:", 0x834 )
	gump.addInputField( 200, 220, 200, 16,  6, hex( item.color ), 0x834 )
	gump.addText( 50, 240, "Amount:", 0x834 )
	gump.addInputField( 200, 240, 200, 16,  7, str( item.amount ), 0x834 )
	gump.addText( 50, 260, "Weight:", 0x834 )
	gump.addInputField( 200, 260, 200, 16,  8, str( item.weight ), 0x834 )
	gump.addText( 50, 280, "Layer:", 0x834 )
	gump.addInputField( 200, 280, 200, 16,  9, str( item.layer ), 0x834 )
	gump.addText( 50, 300, "Good:", 0x834 )
	gump.addInputField( 200, 300, 200, 16, 10, str( item.good ), 0x834 )

	gump.addText( 310, 340, "Page "+str( page_ )+" of "+str( pages ), 0x834 )
	# next page
	gump.addPageButton( 290, 340, 0x0FA, 0x0FA, page_+1 )

	page_ = page_ + 1
	gump.startPage( page_ )

	gump.addResizeGump( 195, 120, 0xBB8, 215, 20 )
	gump.addResizeGump( 195, 140, 0xBB8, 215, 20 )
	gump.addResizeGump( 195, 160, 0xBB8, 215, 20 )
	gump.addResizeGump( 195, 180, 0xBB8, 215, 20 )
	gump.addResizeGump( 195, 200, 0xBB8, 215, 20 )
	gump.addResizeGump( 195, 220, 0xBB8, 215, 20 )
	gump.addResizeGump( 195, 240, 0xBB8, 215, 20 )
	gump.addResizeGump( 195, 260, 0xBB8, 215, 20 )
	gump.addResizeGump( 195, 280, 0xBB8, 215, 20 )
	gump.addResizeGump( 195, 300, 0xBB8, 215, 20 )

	gump.addText( 50, 120, "Type:", 0x834 )
	gump.addInputField( 200, 120, 200, 16, 11, str( item.type ), 0x834 )
	gump.addText( 50, 140, "Type 2:", 0x834 )
	gump.addInputField( 200, 140, 200, 16, 12, str( item.type2 ), 0x834 )
	gump.addText( 50, 160, "Max. hitpoints:", 0x834 )
	gump.addInputField( 200, 160, 200, 16, 13, str( item.maxhealth ), 0x834 )
	gump.addText( 50, 180, "Hitpoints:", 0x834 )
	gump.addInputField( 200, 180, 200, 16, 14, str( item.health ), 0x834 )
	gump.addText( 50, 200, "Speed:", 0x834 )
	gump.addInputField( 200, 200, 200, 16, 15, str( item.speed ), 0x834 )
	gump.addText( 50, 220, "Low damage:", 0x834 )
	gump.addInputField( 200, 220, 200, 16, 16, str( item.lodamage ), 0x834 )
	gump.addText( 50, 240, "High damage:", 0x834 )
	gump.addInputField( 200, 240, 200, 16, 17, str( item.hidamage ), 0x834 )
	gump.addText( 50, 260, "Str (required):", 0x834 )
	gump.addInputField( 200, 260, 200, 16, 18, str( item.strength ), 0x834 )
	gump.addText( 50, 280, "Dex (required):", 0x834 )
	gump.addInputField( 200, 280, 200, 16, 19, str( item.dexterity ), 0x834 )
	gump.addText( 50, 300, "Int (required):", 0x834 )
	gump.addInputField( 200, 300, 200, 16, 20, str( item.intelligence ), 0x834 )

	gump.addText( 310, 340, "Page "+str( page_ )+" of "+str( pages ), 0x834 )
	# prev page
	gump.addPageButton( 270, 340, 0x0FC, 0x0FC, page_-1 )
	# next page
	gump.addPageButton( 290, 340, 0x0FA, 0x0FA, page_+1 )

	page_ = page_ + 1
	gump.startPage( page_ )

	gump.addResizeGump( 195, 120, 0xBB8, 215, 20 )
	gump.addResizeGump( 195, 140, 0xBB8, 215, 20 )
	gump.addResizeGump( 195, 160, 0xBB8, 215, 20 )
	gump.addResizeGump( 195, 180, 0xBB8, 215, 20 )
	gump.addResizeGump( 195, 200, 0xBB8, 215, 20 )
	gump.addResizeGump( 195, 220, 0xBB8, 215, 20 )
	gump.addResizeGump( 195, 240, 0xBB8, 215, 20 )
	gump.addResizeGump( 195, 260, 0xBB8, 215, 20 )
	gump.addResizeGump( 195, 280, 0xBB8, 215, 20 )
	gump.addResizeGump( 195, 300, 0xBB8, 215, 20 )

	gump.addText( 50, 120, "Dyable:", 0x834 )
	gump.addInputField( 200, 120, 200, 16, 21, str( item.dye ), 0x834 )
	gump.addText( 50, 140, "Decay:", 0x834 )
	gump.addInputField( 200, 140, 200, 16, 22, str( item.decay ), 0x834 )
	gump.addText( 50, 160, "Newbie:", 0x834 )
	gump.addInputField( 200, 160, 200, 16, 23, str( item.newbie ), 0x834 )
	gump.addText( 50, 180, "Movable:", 0x834 )
	gump.addInputField( 200, 180, 200, 16, 24, str( item.magic ), 0x834 )
	gump.addText( 50, 200, "Visible:", 0x834 )
	gump.addInputField( 200, 200, 200, 16, 25, str( item.visible ), 0x834 )
	gump.addText( 50, 220, "Rank:", 0x834 )
	gump.addInputField( 200, 220, 200, 16, 26, str( item.rank ), 0x834 )
	gump.addText( 50, 240, "Price:", 0x834 )
	gump.addInputField( 200, 240, 200, 16, 27, str( item.price ), 0x834 )
	gump.addText( 50, 260, "Restock:", 0x834 )
	gump.addInputField( 200, 260, 200, 16, 28, str( item.restock ), 0x834 )
	gump.addText( 50, 280, "Poisoned:", 0x834 )
	gump.addInputField( 200, 280, 200, 16, 29, str( item.poisoned ), 0x834 )
	gump.addText( 50, 300, "Spawnregion:", 0x834 )
	gump.addInputField( 200, 300, 200, 16, 30, str( item.spawnregion ), 0x834 )

	gump.addText( 310, 340, "Page "+str( page_ )+" of "+str( pages ), 0x834 )
	# prev page
	gump.addPageButton( 270, 340, 0x0FC, 0x0FC, page_-1 )
	# next page
	gump.addPageButton( 290, 340, 0x0FA, 0x0FA, page_+1 )

	page_ = page_ + 1
	gump.startPage( page_ )

	gump.addResizeGump( 195, 120, 0xBB8, 215, 20 )
	gump.addResizeGump( 195, 140, 0xBB8, 215, 20 )
	gump.addResizeGump( 195, 160, 0xBB8, 215, 20 )
	gump.addResizeGump( 195, 180, 0xBB8, 215, 20 )
	gump.addResizeGump( 195, 200, 0xBB8, 215, 20 )
	gump.addResizeGump( 195, 220, 0xBB8, 215, 20 )
	gump.addResizeGump( 195, 240, 0xBB8, 215, 20 )
	gump.addResizeGump( 195, 260, 0xBB8, 215, 20 )
	gump.addResizeGump( 195, 280, 0xBB8, 215, 20 )
	gump.addResizeGump( 195, 300, 0xBB8, 215, 20 )

	gump.addText( 50, 120, "Disabled:", 0x834 )
	gump.addInputField( 200, 120, 200, 16, 31, str( item.disabled ), 0x834 )
	gump.addText( 50, 140, "Time unused:", 0x834 )
	gump.addInputField( 200, 140, 200, 16, 32, str( item.timeunused ), 0x834 )
	gump.addText( 50, 160, "Creator:", 0x834 )
	gump.addInputField( 200, 160, 200, 16, 33, str( item.creator ), 0x834 )
	gump.addText( 50, 180, "Made with skill no.:", 0x834 )
	gump.addInputField( 200, 180, 200, 16, 34, str( item.madewith ), 0x834 )
	gump.addText( 50, 200, "Morex:", 0x834 )
	gump.addInputField( 200, 200, 200, 16, 35, str( item.morex ), 0x834 )
	gump.addText( 50, 220, "Morey:", 0x834 )
	gump.addInputField( 200, 220, 200, 16, 36, str( item.morey ), 0x834 )
	gump.addText( 50, 240, "Morez:", 0x834 )
	gump.addInputField( 200, 240, 200, 16, 37, str( item.morez ), 0x834 )
	gump.addText( 50, 260, "More1:", 0x834 )
	gump.addInputField( 200, 260, 200, 16, 38, str( item.more1 ), 0x834 )
	gump.addText( 50, 280, "More2:", 0x834 )
	gump.addInputField( 200, 280, 200, 16, 39, str( item.more2 ), 0x834 )
	gump.addText( 50, 300, "More3:", 0x834 )
	gump.addInputField( 200, 300, 200, 16, 40, str( item.more3 ), 0x834 )

	gump.addText( 310, 340, "Page "+str( page_ )+" of "+str( pages ), 0x834 )
	# prev page
	gump.addPageButton( 270, 340, 0x0FC, 0x0FC, page_-1 )
	# next page
	gump.addPageButton( 290, 340, 0x0FA, 0x0FA, page_+1 )

	page_ = page_ + 1
	gump.startPage( page_ )

	gump.addResizeGump( 195, 120, 0xBB8, 215, 20 )
	gump.addResizeGump( 195, 140, 0xBB8, 215, 20 )
	gump.addResizeGump( 195, 160, 0xBB8, 215, 20 )
	gump.addResizeGump( 195, 180, 0xBB8, 215, 20 )
	gump.addResizeGump( 195, 200, 0xBB8, 215, 20 )

	gump.addText( 50, 120, "More4:", 0x834 )
	gump.addInputField( 200, 120, 200, 16, 41, str( item.more4 ), 0x834 )
	gump.addText( 50, 140, "Moreb1:", 0x834 )
	gump.addInputField( 200, 140, 200, 16, 42, str( item.moreb1 ), 0x834 )
	gump.addText( 50, 160, "Moreb2:", 0x834 )
	gump.addInputField( 200, 160, 200, 16, 43, str( item.moreb2 ), 0x834 )
	gump.addText( 50, 180, "Moreb3:", 0x834 )
	gump.addInputField( 200, 180, 200, 16, 44, str( item.moreb3 ), 0x834 )
	gump.addText( 50, 200, "Moreb4:", 0x834 )
	gump.addInputField( 200, 200, 200, 16, 45, str( item.moreb4 ), 0x834 )

	gump.addText( 310, 340, "Page "+str( page_ )+" of "+str( pages ), 0x834 )
	# prev page
	gump.addPageButton( 270, 340, 0x0FC, 0x0FC, page_-1 )
	
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
			item.name2 = textentries[ key ]
		elif key == 3:
			item.id = int( hex2dec( textentries[ key ] ) )
		elif key == 5:
			pos = textentries[key].split(",")
			if len(pos) > 0:
				item.pos.x = int( pos[0] )
			if len(pos) > 1:
				item.pos.y = int( pos[1] )
			if len(pos) > 2:
				item.pos.z = int( pos[2] )
			if len(pos) > 3:
				item.pos.map = int( pos[3] )
		elif key == 6:
			item.color = int( hex2dec( textentries[ key ] ) )
		elif key == 7:
			item.amount = int( hex2dec( textentries[ key ] ) )
		elif key == 8:
			item.weight = int( hex2dec( textentries[ key ] ) )
		elif key == 9:
			item.layer = int( hex2dec( textentries[ key ] ) )
		elif key == 10:
			item.good = int( hex2dec( textentries[ key ] ) )
		elif key == 11:
			item.type = int( hex2dec( textentries[ key ] ) )
		elif key == 12:
			item.type2 = int( hex2dec( textentries[ key ] ) )
		elif key == 13:
			item.maxhealth = int( hex2dec( textentries[ key ] ) )
		elif key == 14:
			item.health = int( hex2dec( textentries[ key ] ) )
		elif key == 15:
			item.speed = int( hex2dec( textentries[ key ] ) )
		elif key == 16:
			item.lodamage = int( hex2dec( textentries[ key ] ) )
		elif key == 17:
			item.hidamage = int( hex2dec( textentries[ key ] ) )
		elif key == 18:
			item.strength = int( hex2dec( textentries[ key ] ) )
		elif key == 19:
			item.dexterity = int( hex2dec( textentries[ key ] ) )
		elif key == 20:
			item.intelligence = int( hex2dec( textentries[ key ] ) )
		elif key == 21:
			item.dye = int( hex2dec( textentries[ key ] ) )
		elif key == 22:
			item.decay = int( hex2dec( textentries[ key ] ) )
		elif key == 23:
			item.newbie = int( hex2dec( textentries[ key ] ) )
		elif key == 24:
			item.magic = int( hex2dec( textentries[ key ] ) )
		elif key == 25:
			item.visible = int( hex2dec( textentries[ key ] ) )
		elif key == 26:
			item.rank = int( hex2dec( textentries[ key ] ) )
		elif key == 27:
			item.price = int( hex2dec( textentries[ key ] ) )
		elif key == 28:
			item.restock = int( hex2dec( textentries[ key ] ) )
		elif key == 29:
			item.poisoned = int( hex2dec( textentries[ key ] ) )
		elif key == 30:
			item.spawnregion = textentries[key]
		elif key == 31:
			item.disabled = int( hex2dec( textentries[ key ] ) )
		elif key == 32:
			item.timeunused = int( hex2dec( textentries[ key ] ) )
		elif key == 33:
			item.creator = textentries[ key ]
		elif key == 34:
			item.madewith = int( hex2dec( textentries[ key ] ) )
		elif key == 35:
			item.morex = int( hex2dec( textentries[ key ] ) )
		elif key == 36:
			item.morey = int( hex2dec( textentries[ key ] ) )
		elif key == 37:
			item.morez = int( hex2dec( textentries[ key ] ) )
		elif key == 38:
			item.more1 = int( hex2dec( textentries[ key ] ) )
		elif key == 39:
			item.more2 = int( hex2dec( textentries[ key ] ) )
		elif key == 40:
			item.more3 = int( hex2dec( textentries[ key ] ) )
		elif key == 41:
			item.more4 = int( hex2dec( textentries[ key ] ) )
		elif key == 42:
			item.moreb1 = int( hex2dec( textentries[ key ] ) )
		elif key == 43:
			item.moreb2 = int( hex2dec( textentries[ key ] ) )
		elif key == 44:
			item.moreb3 = int( hex2dec( textentries[ key ] ) )
		elif key == 45:
			item.moreb4 = int( hex2dec( textentries[ key ] ) )
			
	if choice.button == 1:
		iteminfo( socket, item )
				
	return 1	
	
			
	