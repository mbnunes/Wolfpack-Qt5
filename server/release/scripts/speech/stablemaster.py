#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: Ambassador Kosh                #
#   )).-' {{ ;'`   | Revised by: (plz, help me. test it.)       #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#
# Speech Script for Stablemaster                                #
#===============================================================#

from wolfpack.consts import *
from wolfpack.gumps import cGump
import wolfpack

prices_shrinks = 100
prices = {
	0x2101:	100,
	0x2103:	100,
	0x20f6:	100,
	0x20e6:	100,
	0x211b:	100,
	0x211c:	100,
	0x211f:	100,
	0x2120:	100,
	0x2121:	100,
	0x2124:	100,
	0x2126:	100,
	0x2127:	100,
	0x20eb:	100
	}
rideables = {
	0x20f6:	0x20f6,
	0x211f:	0x211f,
	0x2120:	0x2120,
	0x2121:	0x2121,
	0x2124:	0x2124
	}
tamed = {
	0x2101:	0x2101,
	0x2103:	0x2103,
	0x20f6:	0x20f6,
	0x20e6:	0x20e6,
	0x211b:	0x211b,
	0x211c:	0x211c,
	0x211f:	0x211f,
	0x2120:	0x2120,
	0x2121:	0x2121,
	0x2124:	0x2124,
	0x2126:	0x2126,
	0x2127:	0x2127,
	0x20eb:	0x20eb
	}
shrinks = {
	0x2101:	0x2101,
	0x2103:	0x2103,
	0x20f6:	0x20f6,
	0x20e6:	0x20e6,
	0x211b:	0x211b,
	0x211c:	0x211c,
	0x211f:	0x211f,
	0x2120:	0x2120,
	0x2121:	0x2121,
	0x2124:	0x2124,
	0x2126:	0x2126,
	0x2127:	0x2127,
	0x20eb:	0x20eb
	}
stables = {
	0x2101:	0x2101,
	0x2103:	0x2103,
	0x20f6:	0x20f6,
	0x20e6:	0x20e6,
	0x211b:	0x211b,
	0x211c:	0x211c,
	0x211f:	0x211f,
	0x2120:	0x2120,
	0x2121:	0x2121,
	0x2124:	0x2124,
	0x2126:	0x2126,
	0x2127:	0x2127,
	0x20eb:	0x20eb
	}
idnpc = {
	0x2101:	'pig',
	0x2103:	'cow',
	0x20f6:	'llama',
	0x20e6:	'sheep',
	0x211b:	'cat',
	0x211c:	'dog',
	0x211f:	'horse_1',
	0x2120:	'horse_2',
	0x2121:	'horse_3',
	0x2124:	'horse_4',
	0x2126:	'packhorse',
	0x2127:	'packllama',
	0x20eb:	'wooly_sheep'
	}
idshrink = {
	0x2101:	'shrink_pig',
	0x2103:	'shrink_cow',
	0x20f6:	'shrink_llama',
	0x20e6:	'shrink_sheep',
	0x211b:	'shrink_cat',
	0x211c:	'shrink_dog',
	0x211f:	'shrink_horse_1',
	0x2120:	'shrink_horse_2',
	0x2121:	'shrink_horse_3',
	0x2124:	'shrink_horse_4',
	0x2126:	'shrink_packhorse',
	0x2127:	'shrink_packllama',
	0x20eb:	'shrink_wooly_sheep'
	}

def onSpeech( listener, speaker, text, keywords ):
	# Check if our name is in the beginning of the string
	#if not text.lower().startswith( listener.name.lower() ):
	#return 0

	gump = cGump( 0, 0, 0, 50, 50 )
	gump.addBackground( 0x24a4, 425, 400 )

	gump.startPage( 0 )
	gump.addHtmlGump( 10, 30, 450, 20, '<basefont size="7" color="#336699"><center>Stablemaster shop</center></basefont>' )
	gump.addTilePic( 140, 31, 0xFB6 )

	addStablePage( speaker, gump, 0x1, 0x2101 )
	addStablePage( speaker, gump, 0x2, 0x2103 )
	addStablePage( speaker, gump, 0x3, 0x20f6 )
	addStablePage( speaker, gump, 0x4, 0x20e6 )
	addStablePage( speaker, gump, 0x5, 0x211b )
	addStablePage( speaker, gump, 0x6, 0x211c )
	addStablePage( speaker, gump, 0x7, 0x211f )
	addStablePage( speaker, gump, 0x8, 0x2120 )
	addStablePage( speaker, gump, 0x9, 0x2121 )
	addStablePage( speaker, gump, 0xa, 0x2124 )
	addStablePage( speaker, gump, 0xb, 0x2126 )
	addStablePage( speaker, gump, 0xc, 0x2127 )
	addStablePage( speaker, gump, 0xd, 0x20eb )

	gump.startPage( 0 )

	gump.addRadioButton(158, 252, 0xd2, 0xd3, 0x0000, 1) #shrink
	gump.addHtmlGump( 183, 252, 150, 20, '<basefont color="#333333">Shrinked: + %s gold</basefont>' % prices_shrinks )
	gump.addRadioButton(158, 272, 0xd2, 0xd3, 0x0001 ) #tamed
	gump.addHtmlGump( 183, 272, 150, 20, '<basefont color="#333333">Tamed</basefont>' )
	gump.addRadioButton(158, 292, 0xd2, 0xd3, 0x0002 ) #rideable
	gump.addHtmlGump( 183, 292, 150, 20, '<basefont color="#333333">Rideable</basefont>' )


	gump.setCallback( "speech.stablemaster.gump_callback" )
	gump.setArgs( [ listener.serial ] )
	gump.send( speaker )

	return 1


def addStablePage( char, gump, page, id ):
	gump.startPage( page )
	addStableButtons( gump, page & 0x000F )
	gump.addResizeGump( 150, 95, 0x2486, 240, 225 )

	if id != 0:
		gump.addTilePic( 250, 175, getStableTilePic( id ))

	gump.addButton( 158, 335, 0x81c, 0x81b, id )
	gump.addHtmlGump( 210, 335, 150, 20, '<basefont color="#336699">Price:<basefont color="#333333"> %i gold</basefont></basefont>' % prices[ id ] )
	gump.addButton( 320, 335, 0xf2, 0xf1, 0 ) # Cancel


def addStableButtons( gump, active=0 ):
	if active == 1:
		gump.addGump( 40, 105, 0x8b0 )
	else:
		gump.addPageButton( 40, 105, 0x837, 0x838, 0x1 )
	gump.addHtmlGump( 60, 100, 100, 20, 'pig' )
	if active == 2:
		gump.addGump( 40, 125, 0x8b0 )
	else:
		gump.addPageButton( 40, 125, 0x837, 0x838, 0x2 )
	gump.addHtmlGump( 60, 120, 100, 20, 'cow' )
	if active == 3:
		gump.addGump( 40, 145, 0x8b0 )
	else:
		gump.addPageButton( 40, 145, 0x837, 0x838, 0x3 )
	gump.addHtmlGump( 60, 140, 100, 20, 'llama' )
	if active == 4:
		gump.addGump( 40, 165, 0x8b0 )
	else:
		gump.addPageButton( 40, 165, 0x837, 0x838, 0x4 )
	gump.addHtmlGump( 60, 160, 100, 20, 'sheep' )
	if active == 5:
		gump.addGump( 40, 185, 0x8b0 )
	else:
		gump.addPageButton( 40, 185, 0x837, 0x838, 0x5 )
	gump.addHtmlGump( 60, 180, 100, 20, 'cat' )
	if active == 6:
		gump.addGump( 40, 205, 0x8b0 )
	else:
		gump.addPageButton( 40, 205, 0x837, 0x838, 0x6 )
	gump.addHtmlGump( 60, 200, 100, 20, 'dog' )
	if active == 7:
		gump.addGump( 40, 225, 0x8b0 )
	else:
		gump.addPageButton( 40, 225, 0x837, 0x838, 0x7 )
	gump.addHtmlGump( 60, 220, 100, 20, 'horse' )
	if active == 8:
		gump.addGump( 40, 245, 0x8b0 )
	else:
		gump.addPageButton( 40, 245, 0x837, 0x838, 0x8 )
	gump.addHtmlGump( 60, 240, 100, 20, 'horse' )
	if active == 9:
		gump.addGump( 40, 265, 0x8b0 )
	else:
		gump.addPageButton( 40, 265, 0x837, 0x838, 0x9 )
	gump.addHtmlGump( 60, 260, 100, 20, 'horse' )
	if active == 10:
		gump.addGump( 40, 285, 0x8b0 )
	else:
		gump.addPageButton( 40, 285, 0x837, 0x838, 0xa )
	gump.addHtmlGump( 60, 280, 100, 20, 'horse' )
	if active == 11:
		gump.addGump( 40, 305, 0x8b0 )
	else:
		gump.addPageButton( 40, 305, 0x837, 0x838, 0xb )
	gump.addHtmlGump( 60, 300, 100, 20, 'packhorse' )
	if active == 12:
		gump.addGump( 40, 325, 0x8b0 )
	else:
		gump.addPageButton( 40, 325, 0x837, 0x838, 0xc )
	gump.addHtmlGump( 60, 320, 100, 20, 'packllama' )
	if active == 13:
		gump.addGump( 40, 345, 0x8b0 )
	else:
		gump.addPageButton( 40, 345, 0x837, 0x838, 0xd )
	gump.addHtmlGump( 60, 340, 100, 20, 'wooly sheep' )




def getStableTilePic( id ):
	if stables.has_key( id ):
		return stables[ id ]
	else:
		return 0




### Callback
def gump_callback( char, args, response ):
	vendor = wolfpack.findchar( args[0] )
	if not vendor:
		return
	if vendor.distanceto( char ) > 5:
		vendor.say( "I can't sell this from over there!",5 )
		char.socket.sysmessage( "You can't reach the vendor." )
		return
	if response.button == 0:
		vendor.say( "Bey!",5 )
		
		return

	button = int( response.switches[0] )
	
	id = response.button
	
	if id == 0:
		return

	if not prices.has_key( id ):
		vendor.say( "I can't sell this!",5 )
		return

	id_npc = idnpc[ id ]
	id_shrink = idshrink[ id ]


	if ( button == 0x0000 ):  	#shrink
		if not shrinks.has_key( id ):
			vendor.say( "They not SHRINKABLE...",5 )
			return
		count = prices[ id ] + prices_shrinks
		if char.countresource( 0xeed, 0 ) < count:
			vendor.say( "You don't have enoug gold!",5 )
			return
		char.useresource( count, 0xeed, 0 )
		char.soundeffect( 0x37, 0 )
		vendor.say( "That is %i gold." % count,5 )
		item = wolfpack.additem( "%s" % id_shrink )
		if not item:
			console.send ( "Invalid defintion: %s\n Vendor error: %s)\n" % ( id_shrink, vendor) )
			return
		item.settag( "npc_id", "%s" % id_npc )
		if rideables.has_key( id ):
			item.settag( "npc_type", "mount" )
		else:
			item.settag( "npc_type", "follow" )
		char.message( item.gettag( "npc_id" ) )
		if not wolfpack.utilities.tocontainer( item, char.getbackpack() ):
			item.update()
		return
	elif ( button == 0x0001 ):  	#tamed

		if not tamed.has_key( id ):
			vendor.say( "They not TAMEABLE...",5 )
			return

		count = prices[ id ]
		if char.countresource( 0xeed, 0 ) < count:
			vendor.say( "You don't have enoug gold!",5 )
			return
		char.useresource( count, 0xeed, 0 )
		char.soundeffect( 0x37, 0 )
		vendor.say( "That is %i gold." % count,5 )
		pos = char.pos
		npc = wolfpack.addnpc( id_npc, pos )
		if not npc:
			console.send ( "Invalid defintion: %s\n Vendor error: %s)\n" % ( id_npc, vendor) )
			return
		npc.owner = char
		npc.tamed = 1
		npc.npcwander = 0
		npc.follow( char )
		return
	elif ( button == 0x0002 ):  	#rideable

		if ( rideables.has_key( id ) != 1 ):
			vendor.say( "They not RIDEABLE...",5 )
			return

		count = prices[ id ]
		if char.countresource( 0xeed, 0 ) < count:
			vendor.say( "You don't have enoug gold!",5 )
			return
		char.useresource( count, 0xeed, 0 )
		char.soundeffect( 0x37, 0 )
		vendor.say( "That is %i gold." % count,5 )
		pos = char.pos
		npcmount = wolfpack.addnpc( id_npc, pos )
		if not npcmount:
			console.send ( "Invalid defintion: %s\n Vendor error: %s)\n" % ( id_npc, vendor) )
			return
		char.mount(npcmount)
		return
	else:
		return
		
	
