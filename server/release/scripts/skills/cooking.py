#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: khpae									 #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

import wolfpack
import wolfpack.utilities
from wolfpack.consts import *
from math import *

MILL_RANGE = 2
flour_mill = [ 0x1920, 0x1921, 0x1922, 0x1923, 0x1924, 0x1925, 0x1926, 0x1927, 0x1928, 0x1929, 0x192a, 0x192b, 0x192c, 0x192d, 0x192e, 0x192f, 0x1930, 0x1931, 0x1932, 0x1933 ]
# how many doughs a sack of flour can make
FLOUR_AMOUNT = 20
# all the flours
flours = [ 0x1039, 0x103a, 0x1045, 0x1046 ]
# not used flours
flours1 = [ 0x1039, 0x1045 ]
WATER_RANGE = 2
# how much times a pitch of water can be used
WATER_AMOUNT = 10
# these should be in backpack
waters1 = [ 0x0ff8, 0x0ff9 ]
# these should be in range
waters2 = [ 0x0b41, 0x0b42, 0x0b43, 0x0b44, 0x0e7b, 0x0ffa, 0x154d ]
FIRE_RANGE = 2
ovens1 = range( 0x045d, 0x048e )
ovens2 = range( 0x08cf, 0x08df )
ovens3 = range( 0x092b, 0x096c )
ovens4 = range( 0x11ec, 0x11ef )
campfire = range( 0x0de3, 0x0de8 )
firefield = [ 0x398c, 0x3996 ]
fires = ovens1 + ovens2 + ovens3 + ovens4 + campfire, firefield

#def onLoad():
#	wolfpack.registerglobal( HOOK_CHAR, EVENT_SKILLUSE, "skills.cooking" )

def onUse( char, item ):
	# Needs to be on ourself
	if item.getoutmostchar() != char:
		char.socket.clilocmessage( 0x7A258 )
		return 1

	# cooking menu gump
	char.sendmakemenu( "CRAFTMENU_COOKING" )

	# set response function

	return 1

# sheaf of wheat + mill -> flour
def makeflour( char ):
	if not char:
		return
	backpack = char.getbackpack()
	if not backpack:
		return

	# he/she has a wheat sheaf
	if not backpack.countresource( 0x1ebd ):
		return

	# check if there is a flour mill around
	if not checkmill( char.pos ):
		return

	backpack.useresource( 1, 0x1ebd )
	flour = wolfpack.additem( "1039" )
	if not flour:
		return
	backpack.additem( flour )
	flour.update()

# flour + water -> dough
def makedough( char ):
	if not char:
		return
	backpack = char.getbackpack()
	if not backpack:
		return

	# check the contents of the backpack : flour, pitcher of water
	flour = None
	water = None
	contents = backpack.content
	for content in contents:
		if content.id in flours:
			flour = content
		if content.id in waters1:
			water = content
		if flour and water:
			break
	# we don't have any flour
	if not flour:
		return
	# if we have no water in backpack, check around
	if not water:
		items = wolfpack.items( char.pos.x + i, char.pos.y + j, char.pos.map, WATER_RANGE )
		for item in items:
			if item.id in waters2:
				water = item
				break
	# we don't have any water
	if not water:
		return

	if not flour.hastag( 'amount' ):
		f_amount = FLOUR_AMOUNT
	else:
		f_amount = flour.gettag( 'amount' )
	# we only check the water amount for pitcher of waters
	if water.id in waters2:
		if not water.hastag( 'amount' ):
			w_amount = WATER_AMOUNT
		else:
			w_amount = water.gettag( 'amount' )

	# useup the resources
	f_amount -= 1
	if not f_amount:
		flour.delte()
	else:
		flour.settag( 'amount', f_amount )
	if water.id in waters2:
		w_amount -= 1
		if not w_amount:
			if water.id == 0x0ff9:
				pitcher = "ff6"
			else:
				pitcher = "ff7"
			water.delete()
			new_pitcher = wolfpack.additem( pitcher )
			if not new_pitcher:
				return
			backpack.additem( new_pitcher )
			new_pitcher.update()
		else:
			water.settag( 'amount', w_amount )

	success = char.checkskill( COOKING, 0, 1000 )
	if not success:
		return
	# make a dough
	dough = wolfpack.additem( "103d" )
	if not dough:
		return
	backpack.additem( dough )
	dough.update()

# now all of these are needed 'fire's
# dough + fire -> bread
def makebread( char ):
	if not char:
		return
	backpack = char.getbackpack()
	if not backpack:
		return

	# firstly, we check a dough
	if not backpack.countresource( 0x103d, 0x0 ):
		return
	# second, check the fires to bake
	if not checkfire( char.pos ):
		return

	# useup a dough
	backpack.useresource( 1, 0x103d, 0x0 )
	success = char.checkskill( COOKING, 0, 1000 )
	if not success:
		return
	# make a bread
	bread = wolfpack.additem( "103c" )
	if not bread:
		return
	backpack.additem( bread )
	bread.update()

# cookie mix + fire -> a pan of cookie
def makecookiepan( char ):
	if not char:
		return
	backpack = char.getbackpack()
	if not backpack:
		return

	if not backpack.countresource( 0x103f ):
		return
	if not checkfire( char.pos ):
		return

	# useup a cookie mix
	backpack.useresource( 1, 0x103f )
	success = char.checkskill( COOKING, 0, 1000 )
	if not success:
		return
	# make a pan of cookie
	cookiepan = wolfpack.additem( "160b" )
	if not cookiepan:
		return
	backpack.additem( cookiepan )
	cookiepan.update()

# cake mix + fire -> cake
#def makecake( char ):

# sweat dough + fire -> muffin
#def makemuffin( char ):

# unbaked cheese bacon pie + fire -> baked cheese bacon pie
def makecheesebaconpie( char ):
	makepie( char, "a cheese bacon pie" )

def makemeatpie( char ):
	makepie( char, "a meat pie" )

def makepie( char, name ):
	if not char:
		return
	backpack = char.getbackpack()
	if not backpack:
		return

	if not backpack.countresource( 0x1042 ):
		return
	if not checkfire( char.pos ):
		return

	backpack.useresource( 1, 0x1042 )
	success = char.checkskill( COOKING, 0, 1000 )
	if not success:
		return
	pie = wolfpack.additem( "1041" )
	if not pie:
		return
	pie.name = name
	backpack.additem( pie )
	pie.update()

# unbaked sausage pizza + fire -> sausage pizza
def makesausagepizza( char ):
	makepizza_( char, "a sausage pizza" )

# unbaked pizza + fire -> pizza
def makepizza( char ):
	makepizza_( char, "a pizza" )

def makepizza_( char, name ):
	if not char:
		return
	backpack = char.getbackpack()
	if not backpack:
		return

	if not backpack.countresource( 0x1083 ):
		return

	if not checkfire( char.pos ):
		return

	backpack.useresource( 1, 0x1083 )

	success = char.checkskill( COOKING, 0, 1000 )
	if not success:
		return
	pizza = wolfpack.additem( "1040" )
	if not pizza:
		return
	pizza.name = name
	backpack.additem( pizza )
	pizza.update()

def checkmill( pos ):
	found = 0
	items = wolfpack.items( pos.x + i, pos.y + j, pos.map, MILL_RANGE )
	for item in items:
		if item.id in flour_mill:
			found = 1
			break
	return found

def checkfire( pos ):
	found_fire = 0
	items = wolfpack.items( pos.x + i, pos.y + j, pos.map, FIRE_RANGE )
	for item in items:
		if item.id in fires:
			found_fire = 1
			break
	return found_fire
