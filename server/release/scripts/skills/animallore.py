#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: DarkStorm                      #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

from wolfpack.consts import *
import wolfpack
from wolfpack.time import *
from wolfpack.gumps import cGump
import skills

ANIMALLORE_DELAY = 1000

def animallore( char, skill ):
	if skill != ANIMALLORE:
		return 0

	if char.socket.hastag( 'skill_delay' ):
		cur_time = servertime()
		if cur_time < char.socket.gettag( 'skill_delay' ):
			char.socket.clilocmessage( 500118, "", 0x3b2, 3 )
			return 1
		else:
			char.socket.deltag( 'skill_delay' )

	char.socket.clilocmessage( 0x7A268, "", 0x3b2, 3 ) # What animal should I look at?
	char.socket.attachtarget( "skills.animallore.response" )

	return 1

def response( char, args, target ):
	if not target.char:
		char.socket.clilocmessage( 0x7A269, "", 0x3b2, 3, char ) # That is not an animal
		return

	if target.char.socket:
		char.socket.clilocmessage ( 0x7A269, "", 0x3b2, 3, char ) # That is not an animal
		return

	if not char.canreach( target.char, 13 ):
		return # no msg sent when you fail los check on OSI, wonder why...

	if target.char.totame >= 1100 and not target.char.tamed:
		if char.skill[ ANIMALLORE ] == 1000:
			char.socket.clilocmessage( 0x10044B, "", 0x3b2, 3 ) # At your skill level, you can only lore tamed or tameable creatures.
			return

		elif char.skill[ ANIMALLORE ] < 1000:
			char.socket.clilocmessage( 0x10044A, "", 0x3b2, 3 ) # At your skill level, you can only lore tamed creatures
			return

	if target.char.totame < 1100 and not target.char.tamed:
		if char.skill[ ANIMALLORE ] < 1000:
			char.socket.clilocmessage( 0x10044A, "", 0x3b2, 3 ) # At your skill level, you can only lore tamed creatures.
			return

	if not char.checkskill( ANIMALLORE, 0, 1000 ):
		char.socket.clilocmessage( 0x7A26E, "", 0x3b2, 3 ) # You can't think of anything you know offhand.
		return

	sendGump( char, args, target )

def sendGump( char, args, target ):

	char.socket.closegump( 0x10101010 )

	loreGump = cGump ( 0, 0, 0, 250, 50 )
	#page 1
	loreGump.startPage( 1 )
	loreGump.addGump( 100, 100, 2080 )
	loreGump.addGump( 118, 137, 2081 )
	loreGump.addGump( 118, 207, 2081 )
	loreGump.addGump( 118, 277, 2081 )
	loreGump.addGump( 118, 347, 2083 )
	loreGump.addGump( 140, 137, 2091 )
	loreGump.addGump( 140, 334, 2091 )
	loreGump.addPageButton( 340, 358, 0x15E1, 0x15E5, 2 )
	loreGump.addPageButton( 317, 358, 0x15E3, 0x15E7, 1 )

	loreGump.addHtmlGump( 147, 108, 210, 18, "<div align=center><i>%s</i></div>" %target.char.name, 0, 0 )

	loreGump.addGump( 128,152, 2086 )
	loreGump.addXmfHtmlGump( 147, 150, 160, 18, 0x1003F9, 0, 0, 200 ) # Attributes

	loreGump.addXmfHtmlGump( 153, 168, 160, 18, 0x1003EA, 0, 0, 16000229 ) # Hits
	loreGump.addHtmlGump( 280, 168, 75, 18, "<div align=right>%i/%i</div>" %( target.char.health, target.char.strength ), 0, 0 )

	loreGump.addXmfHtmlGump( 153, 186, 160, 18, 0x1003EB, 0, 0, 16000229 ) # Stamina
	loreGump.addHtmlGump( 280, 186, 75, 18, "<div align=right>%i/%i</div>" %( target.char.stamina, target.char.dexterity ), 0, 0 )

	loreGump.addXmfHtmlGump( 153, 204, 160, 18, 0x1003EC, 0, 0, 16000229 ) # Mana
	loreGump.addHtmlGump( 280, 204, 75, 18, "<div align=right>%i/%i</div>" %( target.char.mana, target.char.intelligence ), 0, 0 )

	loreGump.addXmfHtmlGump( 153, 222, 160, 18, 0xFB0EF, 0, 0, 16000229 ) # Strength
	loreGump.addHtmlGump( 320, 222, 35, 18, "<div align=right>%i</div>" %target.char.strength, 0, 0 )

	loreGump.addXmfHtmlGump( 153, 240, 160, 18, 0x2DC731, 0, 0, 16000229 ) # Dexterity
	loreGump.addHtmlGump( 320, 240, 35, 18, "<div align=right>%i</div>" %target.char.dexterity, 0, 0 )

	loreGump.addXmfHtmlGump( 153, 258, 160, 18, 0x2DC730, 0, 0, 16000229 ) # Intelligence
	loreGump.addHtmlGump( 320, 258, 35, 18, "<div align=right>%i</div>" %target.char.intelligence, 0, 0 )

	loreGump.addGump( 128, 278, 2086 )

	loreGump.addXmfHtmlGump( 147, 276, 160, 18, 0x2DCAB8, 0, 0, 200 ) # Miscellaneous

	#loreGump.addXmfHtmlGump( 153, 294, 160, 18, 0x1003ED, 0, 0, 16000229 ) # Armor Rating
	#loreGump.addHtmlGump( 320, 294, 35, 18, "<div align=right>%i</div>" % target.char.defense, 0, 0 )

	#page 2
	loreGump.startPage( 2 )
	loreGump.addGump( 100, 100, 2080 )
	loreGump.addGump( 118, 137, 2081 )
	loreGump.addGump( 118, 207, 2081 )
	loreGump.addGump( 118, 277, 2081 )
	loreGump.addGump( 118, 347, 2083 )
	loreGump.addGump( 140, 137, 2091 )
	loreGump.addGump( 140, 334, 2091 )
	loreGump.addPageButton( 340, 358, 0x15E1, 0x15E5, 3 )
	loreGump.addPageButton( 317, 358, 0x15E3, 0x15E7, 1 )

	loreGump.addHtmlGump( 147, 108, 210, 18, "<div align=center><i>%s</i></center>" %target.char.name, 0, 0 )

	loreGump.addGump( 128,152, 2086 )
	loreGump.addXmfHtmlGump( 147, 150, 160, 18, 0x2DCAC6, 0, 0, 200 ) # Combat Skills

	loreGump.addXmfHtmlGump( 153, 168, 160, 18, 0xFEE87, 0, 0, 16000229 ) # Wrestling
	loreGump.addHtmlGump( 280, 168, 75, 18, "<div align=right>%.1f</div>" %( target.char.skill[ WRESTLING ] / 10.0 ), 0, 0 )

	loreGump.addXmfHtmlGump( 153, 186, 160, 18, 0xFEE77, 0, 0, 16000229 ) # Tactics
	loreGump.addHtmlGump( 280, 186, 75, 18, "<div align=right>%.1f</div>" %( target.char.skill[ TACTICS ] / 10.0 ), 0, 0 )

	loreGump.addXmfHtmlGump( 153, 204, 160, 18, 0xFEE76, 0, 0, 16000229 ) # Magic Resistance
	loreGump.addHtmlGump( 280, 204, 75, 18, "<div align=right>%.1f</div>" %( target.char.skill[ MAGICRESISTANCE ] / 10.0 ), 0, 0 )

	loreGump.addXmfHtmlGump( 153, 222, 160, 18, 0xFEE5D, 0, 0, 16000229 ) # Anatomy
	loreGump.addHtmlGump( 280, 222, 75, 18, "<div align=right>%.1f</div>" %( target.char.skill[ ANATOMY ] / 10.0 ), 0, 0 )

	loreGump.addXmfHtmlGump( 153, 240, 160, 18, 0xFEE7A, 0, 0, 16000229 ) # Poisoning

	if char.poison > 0:
		loreGump.addHtmlGump( 280, 240, 75, 18, "<div align=right>%.1f</div>" %( target.char.skill[ POISONGING ] / 10.0 ), 0, 0 )
	else:
		loreGump.addHtmlGump( 280, 240, 75, 18, "<div align=right>--</div>", 0, 0 )

	loreGump.addGump( 128, 260, 2086 )
	loreGump.addXmfHtmlGump( 147, 256, 160, 18, 0x2DCAC8, 0, 0, 200 ) # Lore and Knowledge

	loreGump.addXmfHtmlGump( 153, 276, 160, 18, 0xFEE75, 0, 0, 16000229 ) # Magery
	loreGump.addHtmlGump( 280, 276, 75, 18, "<div align=right>%.1f</div>" %( target.char.skill[ MAGERY ] / 10.0 ), 0, 0 )

	loreGump.addXmfHtmlGump( 153, 294, 160, 18, 0xFEE6C, 0, 0, 16000229 ) # Evaluating Intelligence
	loreGump.addHtmlGump( 280, 294, 75, 18, "<div align=right>%.1f</div>" %( target.char.skill[ EVALUATINGINTEL ] / 10.0 ), 0, 0 )

	loreGump.addXmfHtmlGump( 153, 312, 160, 18, 0xFEE8A, 0, 0, 16000229 ) # Meditation
	loreGump.addHtmlGump( 280, 312, 75, 18, "<div align=right>%.1f</div>" %( target.char.skill[ MEDITATION ] / 10.0 ), 0, 0 )

	#page 3
	loreGump.startPage( 3 )
	loreGump.addGump( 100, 100, 2080 )
	loreGump.addGump( 118, 137, 2081 )
	loreGump.addGump( 118, 207, 2081 )
	loreGump.addGump( 118, 277, 2081 )
	loreGump.addGump( 118, 347, 2083 )
	loreGump.addGump( 140, 137, 2091 )
	loreGump.addGump( 140, 334, 2091 )
	loreGump.addPageButton( 340, 358, 0x15E1, 0x15E5, 3 )
	loreGump.addPageButton( 317, 358, 0x15E3, 0x15E7, 2 )

	loreGump.addHtmlGump( 147, 108, 210, 18, "<div align=center><i>%s</i></center>" %target.char.name, 0, 0 )

	#loreGump.addGump( 128, 152, 2086 )
	#loreGump.addXmfHtmlGump( 147, 150, 160, 18, 0x1003DB, 0, 0, 200 ) # Preferred Foods
	#if target.char.food == 1 or target.char.food == 8:
	#	loreGump.addXmfHtmlGump( 153, 168, 160, 18, 0x1003DC, 0, 0, 16000229 ) # Meat
	#elif target.char.food == 2 or target.char.food == 9:
	#	loreGump.addXmfHtmlGump( 153, 168, 160, 18, 0x1003E0, 0, 0, 16000229 ) # Fish
	#elif target.char.food == 4 or target.char.food == 5:
	#	loreGump.addXmfHtmlGump( 153, 168, 160, 18, 0x1003DD, 0, 0, 16000229 ) # Fruits and Vegetables
	#elif target.char.food == 6 or target.char.food == 7:
	#	loreGump.addXmfHtmlGump( 153, 168, 160, 18, 0x1003DE, 0, 0, 16000229 ) # Grains and Hay
	#elif target.food == 13:
	#	loreGump.addXmfHtmlGump( 153, 168, 160, 18, 0x1003DF, 0, 0, 16000229 ) # Metal
	#else:
	#	loreGump.addXmfHtmlGump( 153, 168, 160, 18, 0xF6D6B, 0, 0, 16000229 ) # None

	loreGump.addGump( 128, 188, 2086 )

	loreGump.addXmfHtmlGump( 147, 186, 160, 18, 0x1003E1, 0, 0, 200 ) # Pack Instinct
	loreGump.addXmfHtmlGump( 153, 204, 160, 18, 0xF6D6B, 0, 0, 16000229 ) # None

	loreGump.addGump( 128, 224, 2086 )

	loreGump.addXmfHtmlGump( 147, 222, 160, 18, 0x1003FA, 0, 0, 200 ) # Loyalty

	if not target.char.tamed:
		loreGump.addXmfHtmlGump( 153, 240, 160, 18, 0xF6D6B, 0, 0, 16000229 ) # None
	else:
		if target.char.hunger == 6:
			loreGump.addXmfHtmlGump( 153, 240, 160, 18, 0x100405, 0, 0, 16000229 ) # Wonderfully happy
		elif target.char.hunger == 5:
			loreGump.addXmfHtmlGump( 153, 240, 160, 18, 0x100403, 0, 0, 16000229 ) # Very Happy
		elif target.char.hunger == 4:
			loreGump.addXmfHtmlGump( 153, 240, 160, 18, 0x100402, 0, 0, 16000229 ) # Rather Happy
		elif target.char.hunger == 3:
			loreGump.addXmfHtmlGump( 153, 240, 160, 18, 0x100400, 0, 0, 16000229 ) # Content
		elif target.char.hunger == 2:
			loreGump.addXmfHtmlGump( 153, 240, 160, 18, 0x1003FE, 0, 0, 16000229 ) # Unhappy
		elif target.char.hunger == 1:
			loreGump.addXmfHtmlGump( 153, 240, 160, 18, 0x1003FD, 0, 0, 16000229 ) # Rather Unhappy
		elif target.char.hunger == 0:
			loreGump.addXmfHtmlGump( 153, 240, 160, 18, 0x1003FC, 0, 0, 16000229 ) # Extremely Unhappy

	loreGump.setArgs( [target] )
	loreGump.setType( 0x10101010 )
	loreGump.send( char )

	cur_time = servertime()
	char.socket.settag( 'skill_delay', cur_time + ANIMALLORE_DELAY )

def onLoad():
	skills.register( ANIMALLORE, animallore )
