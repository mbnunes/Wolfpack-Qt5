#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: khpae                          #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

from wolfpack.consts import *
from wolfpack.utilities import *
import wolfpack
import skills

# mana used when we create a scroll
mana_coast = [ 4, 6, 9, 11, 14, 20, 40, 50 ]
# inscription skill needed to create a scroll
ins_skill =	[ 0, 0, 35, 178, 321, 464, 607, 750 ]
# reagents name:id
reagents =	{ 'ginseng':0xf85, 'bloodmoss':0xf7b, 'mandrake':0xf86, 'blackperl':0xff7a, 'spidersilk':0xf8d, 'sulfurash':0xf8c, 'garlic':0xf84, 'nightshade':0xf88 }
# inscription skill needed to create a runebook
INS_RUNEBOOK = 450
INS_EXCEPTIONAL = 750

# skill is used via the blue button on skill gump
# this will copy a book
def inscription( char, skill ):
	if skill != INSCRIPTION:
		return 0
	char.socket.sysmessage( "copying a book is not implemented yet" )
	return 1

# skill is used via a scribe's pen
def onUse( char, item ):
	if item.getoutmostchar() != char:
		return 1
	# send makemenu
	char.sendmakemenu( "CRAFTMENU_INSCRIPTION" )
	return 1

def clumsy( char ):
	return make_scroll( char, 0 )

def createfood( char ):
	return make_scroll( char, 1 )

def feeblemind( char ):
	return make_scroll( char, 2 )

def heal( char ):
	return make_scroll( char, 3 )

def magicarrow( char ):
	return make_scroll( char, 4 )

def nightsight( char ):
	return make_scroll( char, 5 )

def reactivearmor( char ):
	return make_scroll( char, 6 )

def weaken( char ):
	return make_scroll( char, 7 )

def agility( char ):
	return make_scroll( char, 8 )

def cunning( char ):
	return make_scroll( char, 9 )

def cure( char ):
	return make_scroll( char, 10 )

def harm( char ):
	return make_scroll( char, 11 )

def magictrap( char ):
	return make_scroll( char, 12 )

def magicuntrap( char ):
	return make_scroll( char, 13 )

def prortection( char ):
	return make_scroll( char, 14 )

def strength( char ):
	return make_scroll( char, 15 )

def bless( char ):
	return make_scroll( char, 16 )

def fireball( char ):
	return make_scroll( char, 17 )

def magiclock( char ):
	return make_scroll( char, 18 )

def poison( char ):
	return make_scroll( char, 19 )

def telekinesis( char ):
	return make_scroll( char, 20 )

def teleport( char ):
	return make_scroll( char, 21 )

def unlock( char ):
	return make_scroll( char, 22 )

def wallofstone( char ):
	return make_scroll( char, 23 )

def archcure( char ):
	return make_scroll( char, 24 )

def archprotection( char ):
	return make_scroll( char, 25 )

def curse( char ):
	return make_scroll( char, 26 )

def firefield( char ):
	return make_scroll( char, 27 )

def greaterheal( char ):
	return make_scroll( char, 28 )

def lightning( char ):
	return make_scroll( char, 29 )

def manadrain( char ):
	return make_scroll( char, 30 )

def recall( char ):
	return make_scroll( char, 31 )

def bladespirits( char ):
	return make_scroll( char, 32 )

def dispelfield( char ):
	return make_scroll( char, 33 )

def incognito( char ):
	return make_scroll( char, 34 )

def magicreflection( char ):
	return make_scroll( char, 35 )

def mindblast( char ):
	return make_scroll( char, 36 )

def paralyze( char ):
	return make_scroll( char, 37 )

def poisonfield( char ):
	return make_scroll( char, 38 )

def summoncreature( char ):
	return make_scroll( char, 39 )

def dispel( char ):
	return make_scroll( char, 40 )

def energybolt( char ):
	return make_scroll( char, 41 )

def explosion( char ):
	return make_scroll( char, 42 )

def invisibility( char ):
	return make_scroll( char, 43 )

def mark( char ):
	return make_scroll( char, 44 )

def masscurse( char ):
	return make_scroll( char, 45 )

def paralyzefield( char ):
	return make_scroll( char, 46 )

def reveal( char ):
	return make_scroll( char, 47 )

def chainlightning( char ):
	return make_scroll( char, 48 )

def energyfield( char ):
	return make_scroll( char, 49 )

def flamestrike( char ):
	return make_scroll( char, 50 )

def gatetravel( char ):
	return make_scroll( char, 51 )

def manavampire( char ):
	return make_scroll( char, 52 )

def massdispel( char ):
	return make_scroll( char, 53 )

def meteoswarm( char ):
	return make_scroll( char, 54 )

def polymorph( char ):
	return make_scroll( char, 55 )

def earthquake( char ):
	return make_scroll( char, 56 )

def energyvortex( char ):
	return make_scroll( char, 57 )

def resurrection( char ):
	return make_scroll( char, 58 )

def airelemental( char ):
	return make_scroll( char, 59 )

def summondaemon( char ):
	return make_scroll( char, 60 )

def earthelemental( char ):
	return make_scroll( char, 61 )

def fireelemental( char ):
	return make_scroll( char, 62 )

def waterelemental( char ):
	return make_scroll( char, 63 )

def make_scroll( char, num ):
	if not char:
		return 0
	if not 0 <= num < 64:
		return 0
	
	# check if we have a spellbook which containing the spell
	backpack = char.getbackpack()
	if not backpack:
		return 0
	contents = backpack.content
	found = 0
	for item in contents:
		if item.id == 0xefa:
			book = item
			found = 1
			break
	if not found:
		return 0
	if not book.hasspell( num ):
		return 0

	# check if we have a empty scroll
	snum1 = char.countresource( 0xe34 )
	snum2 = char.countresource( 0xef3 )
	if snum1 and snum2:
		return 0

	# skill info : for name and reagents
	spell_info = wolfpack.spell( num )
	if not spell_info:
		return 0

	# check if we have enough reagents
	regs = 1
	for reg in reagents.keys():
		if spell_info[ reg ] > char.countresource( reagents[ reg ] ):
			regs = 0
			break
	if not regs:
		return 0

	circle = num / 8
	mana = mana_coast[ circle ]
	skill = ins_skill[ circle ]
	# not enough mana or skill
	if char.mana < mana or char.skill[ INSCRIPTION ] < skill:
		return 0

	# effects : will be added

	# useup regs and scroll
	for reg in reagents.keys():
		if spell_info[ reg ]:
			char.useresource( spell_info[ reg ], reagents[ reg ] )
	if snum1:
		char.useresource( 1, 0xe34 )
	else:
		char.useresource( 1, 0xef3 )

	# checkskill
	success = char.checkskill( INSCRIPTION, ins_skill[ circle ], ins_skill[ circle + 1 ] )
	if not success:
		return 0
	
	# useup mana and make a scroll
	char.mana -= mana
	scroll = wolfpack.additem( spell_info[ "scroll" ] )
	if not scroll:
		return 0
	scroll.name = "%s scroll" % spell_info[ "name" ]
	backpack.additem( scroll )
	scroll.update()
	return 1

def runebook( char ):
	if not char:
		return 0

	# check if we have enough skill ( 45.0 )
	if char.skill[ MAGERY ] < INS_RUNEBOOK:
		return 0

	# check if we have enough empty scrolls ( 8 )
	snum1 = char.countresource( 0xe34 )
	snum2 = char.countresource( 0xef3 )
	if snum1 + snum2 < 8:
		return 0

	# check if we have enough rune ( 1 )
	rnum = [ 0 ] * 4
	rnum[ 0 ] = char.countresource( 0x1f14 )
	rnum[ 1 ] = char.countresource( 0x1f15 )
	rnum[ 2 ] = char.countresource( 0x1f16 )
	rnum[ 3 ] = char.countresource( 0x1f17 )
	rune_num = rnum[ 0 ] + rnum[ 1 ] + rnum[ 2 ] + rnum[ 3 ]
	if not rune_num:
		return 0

	# check if we have enough recall scroll ( 1 )
	if not char.countresource( 0x1f4c ):
		return 0

	# check if we have enough gate travel scroll ( 1 )
	if not char.countresource( 0x1f60 ):
		return 0

	# checkskill
	sucess = char.checkskill( INSCRIPTION, INS_RUNEBOOK, 1000 )
	if not success:
		return 0

	# now, we create runebook
	book = wolfpack.additem( "22c5" )
	if not book:
		return 0

	# useup 8 scrolls
	if snum1 >= 8:
		char.useresource( 8, 0xe34 )
	elif snum1 and snum1 < 8:
		char.useresource( snum1, 0xe34 )
		char.useresource( 8 - snum1, 0xef3 )
	else:
		char.useresource( 8, 0xef3 )
	
	# useup 1 recall rune
	for i in range( 0, 4 ):
		if rnum[ i ]:
			char.useresource( 1, 0x1f14 + i )
			break
	
	# useup 1 recall scroll and 1 gate travel scroll
	char.useresource( 1, 0x1f4c )
	char.useresource( 1, 0x1f60 )
	
	# max charge
	chance = char.skill[ INSCRIPTION ] - INS_RUNEBOOK
	max = 5 + chance / 100
	chance2 = whrandom.randint( 0, 100 )
	max_charge = 5
	# 10% max
	if chance2 < 10:
		max_charge = max
	# 60% max - 1
	elif chance2 < 70:
		max_charge = max - 1
	# 30% max - 2
	else:
		max_charge = max - 2
	if max_charge < 5:
		max_charge = 5
	book.settag( 'maxcharges', max_charge )

	# exceptional : 15% at 75.0 - 65% at 100.0
	if char.skill[ INSCRIPTION ] >= INS_EXCEPTIONAL:
		ex_chance = 15 + ( char.skill[ INSCRIPTION ] - INS_EXCEPTIONAL ) / 5
		if whrandom.randint( 0, 100 ) < ex_chance:
			book.settag( 'exceptional', 1 )

	backpack.additem( book )
	book.update()
	return 1

def onLoad():
	skills.register( INSCRIPTION, inscription )
