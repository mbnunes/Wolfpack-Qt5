#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: DarkStorm                      #
#   )).-' {{ ;'`   | Revised by: Correa                         #
#  ( (  ;._ \\ ctr | Last Modification: Custom House Gump#
#===============================================================#

import wolfpack
import housing
from housing.house import *
from wolfpack.gumps import cGump
from wolfpack.consts import EVENT_CHLEVELCHANGE

#
# Graphics definitions matching client
#
HANGER_NUMBERS = [2968, 2970, 2972, 2974, 2976, 2978]

FOUNDATION_NUMBERS = [
	20, 189, 765, 65, 101, 11767, 11771, 11207, 11715, 11181,
	13938, 13942, 16806, 16732, 19208, 39614, 39888
]

POST_NUMBERS = [
	9, 29, 54, 90, 147, 169, 177, 204, 251, 257,
	263, 298, 347, 353, 424, 441, 466, 514, 553,
	600, 601, 602, 603, 660, 666, 672, 898, 970,
	974, 982, 11212, 11720, 11186, 13788, 13849,
	17190, 16796, 16733, 16663, 20758, 19214,
	39603, 39809
]

HOUSE_SIGNS = [2980 + (i * 2) for i in range(54)] + [2966, 3140]

def get_house(item):
	if not item:
		return None
	multi = getattr(item, 'multi', None)
	if not multi and hasattr(item, 'hastag') and item.hastag('house'):
		multi = wolfpack.findobject(item.gettag('house'))
	if not multi and hasattr(item, 'pos'):
		multi = wolfpack.findmulti(item.pos)
		if multi and hasattr(item, 'settag'):
			item.settag('house', multi.serial)
	return multi

# Register as a global script
def onLoad():
	wolfpack.registerglobal( EVENT_CHLEVELCHANGE, "signpost" )

def onCHLevelChange( char, level ):
	if not char.hastag( 'customizing' ):
		return
	import housing.customhouse as ch
	ch.onLevelChange( char, level )
	return 1

def onUse( char, item ):
	gump0( char, gumpcallback, item )
	return True

def gumpcallback( char, args, target ):
	switchgump( char, target, args )

#
# Helper: Find or spawn sign hanger
#
def get_or_find_hanger(sign):
	if sign.hastag('hanger'):
		try:
			hanger = wolfpack.finditem(int(sign.gettag('hanger')))
			if hanger:
				return hanger
		except:
			pass
	for it in wolfpack.items(sign.pos, 1):
		if it.serial != sign.serial and (it.id in HANGER_NUMBERS or it.id in [0xB98, 0xB9A, 0xB9C, 0xB9E, 0xBA0, 0xBA2]):
			sign.settag('hanger', it.serial)
			return it
	hanger = wolfpack.additem("b98")
	hanger.decay = False
	hanger.moveto(sign.pos)
	hanger.update()
	sign.settag('hanger', hanger.serial)
	return hanger

#
# Helper: Find or spawn signpost
#
def get_or_find_post(sign):
	if sign.hastag('signpost'):
		try:
			post = wolfpack.finditem(int(sign.gettag('signpost')))
			if post:
				return post
		except:
			pass
	for it in wolfpack.items(sign.pos, 1):
		if it.serial != sign.serial and it.id in POST_NUMBERS:
			sign.settag('signpost', it.serial)
			return it
	hanger = get_or_find_hanger(sign)
	sign.settag('signpost', hanger.serial)
	return hanger

#
# Page 0: Information Page
#
def gump0( char, callback, item ):
	multi = get_house(item)
	if not multi:
		return
	builton = str(multi.gettag('builton') or '')
	mvalue = str(multi.gettag('value') or '0')
	owner_name = str(getHouseOwnerName(multi))

	gump_params = [
		"a house",
		"sign",
		owner_name,
		builton,
		"never",
		mvalue,
		"0"
	]

	mygump = cGump()
	mygump.addRawLayout( "{page 0}" )
	mygump.addRawLayout( "{resizepic 0 0 5054 420 440}" )
	mygump.addRawLayout( "{gumppictiled 10 10 400 100 2624}" )
	mygump.addRawLayout( "{checkertrans 10 10 400 100}" )
	mygump.addRawLayout( "{gumppictiled 10 120 400 260 2624}" )
	mygump.addRawLayout( "{checkertrans 10 120 400 260}" )
	mygump.addRawLayout( "{gumppictiled 10 390 400 40 2624}" )
	mygump.addRawLayout( "{checkertrans 10 390 400 40}" )
	mygump.addRawLayout( "{button 250 410 4005 4007 1 0 0}" ) # close 
	mygump.addRawLayout( "{xmfhtmlgumpcolor 285 410 240 20 1060675 0 0 32767}" ) # Close
	mygump.addRawLayout( "{gumppic 10 10 100}" )
	mygump.addRawLayout( "{text 62 43 0 0}" )
	mygump.addRawLayout( "{text 74 57 0 1}" )
	
	# Grant / Revoke Access
	if checkAccess(char, multi, ACCESS_COOWNER):
		mygump.addRawLayout( "{button 10 390 4005 4007 1 0 31}" ) # Grant Access
		mygump.addRawLayout( "{xmfhtmlgumpcolor 45 390 240 20 1060676 0 0 32767}" )
		mygump.addRawLayout( "{button 10 410 4005 4007 1 0 46}" ) # Revoke Access
		mygump.addRawLayout( "{xmfhtmlgumpcolor 45 410 240 20 1060677 0 0 32767}" )
	else:
		mygump.addRawLayout( "{xmfhtmlgumpcolor 45 390 240 20 1060676 0 0 16912}" )
		mygump.addRawLayout( "{xmfhtmlgumpcolor 45 410 240 20 1060677 0 0 16912}" )

	# Navigation
	mygump.addRawLayout( "{button 150 10 4006 4007 1 0 2}" ) # Information
	mygump.addRawLayout( "{xmfhtmlgumpcolor 195 10 200 20 1060668 0 0 16927}" )
	mygump.addRawLayout( "{button 150 30 4005 4007 1 0 17}" ) # Security
	mygump.addRawLayout( "{xmfhtmlgumpcolor 195 30 200 20 1060669 0 0 32767}" )
	mygump.addRawLayout( "{button 150 50 4005 4007 1 0 32}" ) # Storage
	mygump.addRawLayout( "{xmfhtmlgumpcolor 195 50 200 20 1060670 0 0 32767}" )
	mygump.addRawLayout( "{button 150 70 4005 4007 1 0 47}" ) # Customize
	mygump.addRawLayout( "{xmfhtmlgumpcolor 195 70 200 20 1060671 0 0 32767}" )
	mygump.addRawLayout( "{button 150 90 4005 4007 1 0 62}" ) # Ownership
	mygump.addRawLayout( "{xmfhtmlgumpcolor 195 90 200 20 1060672 0 0 32767}" )

	mygump.addRawLayout( "{xmfhtmlgumpcolor 20 130 200 20 1011242 0 0 32767}" ) # Owned By: 
	mygump.addRawLayout( "{text 210 130 1152 2}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 20 170 380 20 1018032 0 0 16927}" ) # This house is properly placed.
	
	if multi.hastag('ch_revision') or item.hasscript('signpost'):
		mygump.addRawLayout( "{xmfhtmlgumpcolor 20 190 380 20 1060681 0 0 16927}" ) # This is a custom-built house.
	else:
		mygump.addRawLayout( "{xmfhtmlgumpcolor 20 190 380 20 1018035 0 0 16927}" ) # This house is of classic design.

	if isPublic(multi):
		mygump.addRawLayout( "{xmfhtmlgumpcolor 20 230 380 20 1060678 0 0 16927}" ) # This house is open to the public.
	else:
		mygump.addRawLayout( "{xmfhtmlgumpcolor 20 230 380 20 1060679 0 0 16927}" ) # This house is private.

	mygump.addRawLayout( "{xmfhtmlgumpcolor 20 250 380 20 1062209 0 0 16927}" ) # Automatically refreshed.
	mygump.addRawLayout( "{xmfhtmlgumpcolor 20 290 200 20 1060692 0 0 16927}" ) # Built On:
	mygump.addRawLayout( "{text 250 290 1152 3}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 20 310 200 20 1060693 0 0 16927}" ) # Last traded:
	mygump.addRawLayout( "{text 250 310 1152 4}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 20 330 200 20 1061793 0 0 16927}" ) # House value:
	mygump.addRawLayout( "{text 250 330 1152 5}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 20 360 300 20 1011241 0 0 16927}" ) # Number of visits:
	mygump.addRawLayout( "{text 350 360 1152 6}" )

	mygump.setCallback( callback )
	mygump.setArgs( [item] )
	for line in gump_params:
		mygump.addRawText( line )
	mygump.send( char )

#
# Page 1: Security Page
#
def gump1( char, callback, item ):
	multi = get_house(item)
	if not multi:
		return
	is_owner = checkAccess(char, multi, ACCESS_OWNER)
	is_coowner = checkAccess(char, multi, ACCESS_COOWNER)
	is_friend = checkAccess(char, multi, ACCESS_FRIEND)

	gump_params = [
		"a house",
		"sign"
	]

	mygump = cGump()
	mygump.addRawLayout( "{page 0}" )
	mygump.addRawLayout( "{resizepic 0 0 5054 420 440}" )
	mygump.addRawLayout( "{gumppictiled 10 10 400 100 2624}" )
	mygump.addRawLayout( "{checkertrans 10 10 400 100}" )
	mygump.addRawLayout( "{gumppictiled 10 120 400 260 2624}" )
	mygump.addRawLayout( "{checkertrans 10 120 400 260}" )
	mygump.addRawLayout( "{gumppictiled 10 390 400 40 2624}" )
	mygump.addRawLayout( "{checkertrans 10 390 400 40}" )
	mygump.addRawLayout( "{button 250 410 4005 4007 1 0 0}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 285 410 240 20 1060675 0 0 32767}" )
	mygump.addRawLayout( "{gumppic 10 10 100}" )
	mygump.addRawLayout( "{text 62 43 0 0}" )
	mygump.addRawLayout( "{text 74 57 0 1}" )

	# Grant / Revoke Access
	if is_coowner:
		mygump.addRawLayout( "{button 10 390 4005 4007 1 0 31}" )
		mygump.addRawLayout( "{xmfhtmlgumpcolor 45 390 240 20 1060676 0 0 32767}" )
		mygump.addRawLayout( "{button 10 410 4005 4007 1 0 46}" )
		mygump.addRawLayout( "{xmfhtmlgumpcolor 45 410 240 20 1060677 0 0 32767}" )
	else:
		mygump.addRawLayout( "{xmfhtmlgumpcolor 45 390 240 20 1060676 0 0 16912}" )
		mygump.addRawLayout( "{xmfhtmlgumpcolor 45 410 240 20 1060677 0 0 16912}" )

	# Navigation
	mygump.addRawLayout( "{button 150 10 4005 4007 1 0 2}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 195 10 200 20 1060668 0 0 32767}" )
	mygump.addRawLayout( "{button 150 30 4006 4007 1 0 17}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 195 30 200 20 1060669 0 0 16927}" )
	mygump.addRawLayout( "{button 150 50 4005 4007 1 0 32}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 195 50 200 20 1060670 0 0 32767}" )
	mygump.addRawLayout( "{button 150 70 4005 4007 1 0 47}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 195 70 200 20 1060671 0 0 32767}" )
	mygump.addRawLayout( "{button 150 90 4005 4007 1 0 62}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 195 90 200 20 1060672 0 0 32767}" )

	# Co-owners
	if is_coowner:
		mygump.addRawLayout( "{button 10 130 4005 4007 1 0 4}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 45 130 240 20 1011266 0 0 32767}" ) # View Co-Owner List
	if is_owner:
		mygump.addRawLayout( "{button 10 150 4005 4007 1 0 19}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 45 150 240 20 1011267 0 0 32767}" ) # Add a Co-Owner
	if is_owner:
		mygump.addRawLayout( "{button 10 170 4005 4007 1 0 34}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 45 170 240 20 1018036 0 0 32767}" ) # Remove Co-Owner
	if is_owner:
		mygump.addRawLayout( "{button 10 190 4005 4007 1 0 49}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 45 190 240 20 1011268 0 0 32767}" ) # Clear Co-Owner List

	# Friends
	if is_coowner:
		mygump.addRawLayout( "{button 10 220 4005 4007 1 0 64}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 45 220 240 20 1011243 0 0 32767}" ) # View Friends List
	if is_coowner:
		mygump.addRawLayout( "{button 10 240 4005 4007 1 0 79}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 45 240 240 20 1011244 0 0 32767}" ) # Add a Friend
	if is_coowner:
		mygump.addRawLayout( "{button 10 260 4005 4007 1 0 94}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 45 260 240 20 1018037 0 0 32767}" ) # Remove a Friend
	if is_coowner:
		mygump.addRawLayout( "{button 10 280 4005 4007 1 0 109}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 45 280 240 20 1011245 0 0 32767}" ) # Clear Friends List

	# Ban List & Access List
	if is_friend:
		mygump.addRawLayout( "{button 10 310 4005 4007 1 0 154}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 45 310 240 20 1060699 0 0 32767}" ) # View Ban List
	if is_friend:
		mygump.addRawLayout( "{button 10 330 4005 4007 1 0 169}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 45 330 240 20 1060700 0 0 32767}" ) # View Access List

	# Public / Private Toggle
	if is_owner:
		mygump.addRawLayout( "{button 210 130 4005 4007 1 0 199}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 245 130 240 20 1060694 0 0 32767}" ) # Declare Public/Private

	mygump.setCallback( callback )
	mygump.setArgs( [item] )
	for line in gump_params:
		mygump.addRawText( line )
	mygump.send( char )

#
# Page 2: Storage Page
#
def gump2( char, callback, item ):
	gump_params = [
		"a house",
		"sign",
		"0",
		"425",
		"212"
	]

	mygump = cGump()
	mygump.addRawLayout( "{page 0}" )
	mygump.addRawLayout( "{resizepic 0 0 5054 420 440}" )
	mygump.addRawLayout( "{gumppictiled 10 10 400 100 2624}" )
	mygump.addRawLayout( "{checkertrans 10 10 400 100}" )
	mygump.addRawLayout( "{gumppictiled 10 120 400 260 2624}" )
	mygump.addRawLayout( "{checkertrans 10 120 400 260}" )
	mygump.addRawLayout( "{gumppictiled 10 390 400 40 2624}" )
	mygump.addRawLayout( "{checkertrans 10 390 400 40}" )
	mygump.addRawLayout( "{button 250 410 4005 4007 1 0 0}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 285 410 240 20 1060675 0 0 32767}" )
	mygump.addRawLayout( "{gumppic 10 10 100}" )
	mygump.addRawLayout( "{text 62 43 0 0}" )
	mygump.addRawLayout( "{text 74 57 0 1}" )

	# Navigation
	mygump.addRawLayout( "{button 150 10 4005 4007 1 0 2}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 195 10 200 20 1060668 0 0 32767}" )
	mygump.addRawLayout( "{button 150 30 4005 4007 1 0 17}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 195 30 200 20 1060669 0 0 32767}" )
	mygump.addRawLayout( "{button 150 50 4006 4007 1 0 32}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 195 50 200 20 1060670 0 0 16927}" )
	mygump.addRawLayout( "{button 150 70 4005 4007 1 0 47}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 195 70 200 20 1060671 0 0 32767}" )
	mygump.addRawLayout( "{button 150 90 4005 4007 1 0 62}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 195 90 200 20 1060672 0 0 32767}" )

	mygump.addRawLayout( "{xmfhtmlgumpcolor 10 130 400 20 1060682 0 0 32767}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 10 170 275 20 1011237 0 0 32767}" )
	mygump.addRawLayout( "{text 310 170 1152 2}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 10 190 275 20 1011238 0 0 32767}" )
	mygump.addRawLayout( "{text 310 190 1152 3}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 10 210 275 20 1011239 0 0 32767}" )
	mygump.addRawLayout( "{text 310 210 1152 2}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 10 230 275 20 1011240 0 0 32767}" )
	mygump.addRawLayout( "{text 310 230 1152 4}" )

	mygump.setCallback( callback )
	mygump.setArgs( [item] )
	for line in gump_params:
		mygump.addRawText( line )
	mygump.send( char )

#
# Page 3: Customization Page
#
def gump3( char, callback, item ):
	multi = get_house(item)
	if not multi:
		return
	is_owner = checkAccess(char, multi, ACCESS_OWNER)
	is_coowner = checkAccess(char, multi, ACCESS_COOWNER)
	is_customizable = item.hasscript('signpost') or multi.hastag('ch_revision')
	is_public = isPublic(multi)
	has_moving_crate = multi.hastag('moving_crate')

	gump_params = [
		str(multi.name or "a house"),
		"sign"
	]

	mygump = cGump()
	mygump.addRawLayout( "{page 0}" )
	mygump.addRawLayout( "{resizepic 0 0 5054 420 440}" )
	mygump.addRawLayout( "{gumppictiled 10 10 400 100 2624}" )
	mygump.addRawLayout( "{checkertrans 10 10 400 100}" )
	mygump.addRawLayout( "{gumppictiled 10 120 400 260 2624}" )
	mygump.addRawLayout( "{checkertrans 10 120 400 260}" )
	mygump.addRawLayout( "{gumppictiled 10 390 400 40 2624}" )
	mygump.addRawLayout( "{checkertrans 10 390 400 40}" )
	mygump.addRawLayout( "{button 250 410 4005 4007 1 0 0}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 285 410 240 20 1060675 0 0 32767}" )
	mygump.addRawLayout( "{gumppic 10 10 100}" )
	mygump.addRawLayout( "{text 62 43 0 0}" )
	mygump.addRawLayout( "{text 74 57 0 1}" )

	# Grant / Revoke Access
	if is_coowner:
		mygump.addRawLayout( "{button 10 390 4005 4007 1 0 31}" )
		mygump.addRawLayout( "{xmfhtmlgumpcolor 45 390 240 20 1060676 0 0 32767}" )
		mygump.addRawLayout( "{button 10 410 4005 4007 1 0 46}" )
		mygump.addRawLayout( "{xmfhtmlgumpcolor 45 410 240 20 1060677 0 0 32767}" )
	else:
		mygump.addRawLayout( "{xmfhtmlgumpcolor 45 390 240 20 1060676 0 0 16912}" )
		mygump.addRawLayout( "{xmfhtmlgumpcolor 45 410 240 20 1060677 0 0 16912}" )

	# Navigation
	mygump.addRawLayout( "{button 150 10 4005 4007 1 0 2}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 195 10 200 20 1060668 0 0 32767}" )
	mygump.addRawLayout( "{button 150 30 4005 4007 1 0 17}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 195 30 200 20 1060669 0 0 32767}" )
	mygump.addRawLayout( "{button 150 50 4005 4007 1 0 32}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 195 50 200 20 1060670 0 0 32767}" )
	mygump.addRawLayout( "{button 150 70 4006 4007 1 0 47}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 195 70 200 20 1060671 0 0 16927}" )
	mygump.addRawLayout( "{button 150 90 4005 4007 1 0 62}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 195 90 200 20 1060672 0 0 32767}" )

	# 1. Convert Into Customizable House (disabled for custom house)
	mygump.addRawLayout( "{xmfhtmlgumpcolor 45 120 240 20 1060759 0 0 16912}" )

	# 2. Customize this House (1060765)
	if is_owner and is_customizable:
		mygump.addRawLayout( "{button 10 160 4005 4007 1 0 21}" )
		mygump.addRawLayout( "{xmfhtmlgumpcolor 45 160 240 20 1060765 0 0 32767}" )
	else:
		mygump.addRawLayout( "{xmfhtmlgumpcolor 45 160 240 20 1060765 0 0 16912}" )

	# 3. Relocate Moving Crate (1060760)
	if is_owner and has_moving_crate:
		mygump.addRawLayout( "{button 10 180 4005 4007 1 0 180}" )
		mygump.addRawLayout( "{xmfhtmlgumpcolor 45 180 240 20 1060760 0 0 32767}" )
	else:
		mygump.addRawLayout( "{xmfhtmlgumpcolor 45 180 240 20 1060760 0 0 16912}" )

	# 4. Change House Sign (1060761)
	if is_owner and is_public:
		mygump.addRawLayout( "{button 10 210 4005 4007 1 0 210}" )
		mygump.addRawLayout( "{xmfhtmlgumpcolor 45 210 240 20 1060761 0 0 32767}" )
	else:
		mygump.addRawLayout( "{xmfhtmlgumpcolor 45 210 240 20 1060761 0 0 16912}" )

	# 5. Change House Sign Hanger (1060762)
	if is_owner and is_customizable:
		mygump.addRawLayout( "{button 10 230 4005 4007 1 0 66}" )
		mygump.addRawLayout( "{xmfhtmlgumpcolor 45 230 240 20 1060762 0 0 32767}" )
	else:
		mygump.addRawLayout( "{xmfhtmlgumpcolor 45 230 240 20 1060762 0 0 16912}" )

	# 6. Change Signpost (1060763)
	if is_owner and is_customizable:
		mygump.addRawLayout( "{button 10 250 4005 4007 1 0 81}" )
		mygump.addRawLayout( "{xmfhtmlgumpcolor 45 250 240 20 1060763 0 0 32767}" )
	else:
		mygump.addRawLayout( "{xmfhtmlgumpcolor 45 250 240 20 1060763 0 0 16912}" )

	# 7. Change Foundation Style (1062004)
	if is_owner and is_customizable:
		mygump.addRawLayout( "{button 10 280 4005 4007 1 0 96}" )
		mygump.addRawLayout( "{xmfhtmlgumpcolor 45 280 240 20 1062004 0 0 32767}" )
	else:
		mygump.addRawLayout( "{xmfhtmlgumpcolor 45 280 240 20 1062004 0 0 16912}" )

	# 8. Rename House (1060764)
	if is_coowner:
		mygump.addRawLayout( "{button 10 310 4005 4007 1 0 111}" )
		mygump.addRawLayout( "{xmfhtmlgumpcolor 45 310 240 20 1060764 0 0 32767}" )
	else:
		mygump.addRawLayout( "{xmfhtmlgumpcolor 45 310 240 20 1060764 0 0 16912}" )

	mygump.setCallback( callback )
	mygump.setArgs( [item] )
	for line in gump_params:
		mygump.addRawText( line )
	mygump.send( char )

#
# Page 4: Ownership Page
#
def gump4( char, callback, item ):
	multi = get_house(item)
	if not multi:
		return
	is_owner = checkAccess(char, multi, ACCESS_OWNER)

	gump_params = [
		"a house",
		"sign"
	]
	mygump = cGump()

	mygump.addRawLayout( "{page 0}" )
	mygump.addRawLayout( "{resizepic 0 0 5054 420 440}" )
	mygump.addRawLayout( "{gumppictiled 10 10 400 100 2624}" )
	mygump.addRawLayout( "{checkertrans 10 10 400 100}" )
	mygump.addRawLayout( "{gumppictiled 10 120 400 260 2624}" )
	mygump.addRawLayout( "{checkertrans 10 120 400 260}" )
	mygump.addRawLayout( "{gumppictiled 10 390 400 40 2624}" )
	mygump.addRawLayout( "{checkertrans 10 390 400 40}" )
	mygump.addRawLayout( "{button 250 410 4005 4007 1 0 0}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 285 410 240 20 1060675 0 0 32767}" )
	mygump.addRawLayout( "{gumppic 10 10 100}" )
	mygump.addRawLayout( "{text 62 43 0 0}" )
	mygump.addRawLayout( "{text 74 57 0 1}" )

	# Navigation
	mygump.addRawLayout( "{button 150 10 4005 4007 1 0 2}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 195 10 200 20 1060668 0 0 32767}" )
	mygump.addRawLayout( "{button 150 30 4005 4007 1 0 17}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 195 30 200 20 1060669 0 0 32767}" )
	mygump.addRawLayout( "{button 150 50 4005 4007 1 0 32}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 195 50 200 20 1060670 0 0 32767}" )
	mygump.addRawLayout( "{button 150 70 4005 4007 1 0 47}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 195 70 200 20 1060671 0 0 32767}" )
	mygump.addRawLayout( "{button 150 90 4006 4007 1 0 62}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 195 90 200 20 1060672 0 0 16927}" )

	# Demolish
	if is_owner:
		mygump.addRawLayout( "{button 10 130 4005 4007 1 0 7}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 45 130 240 20 1061794 0 0 32767}" ) # Demolish House

	# Trade House
	if is_owner:
		mygump.addRawLayout( "{button 10 150 4005 4007 1 0 22}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 45 150 240 20 1061797 0 0 32767}" ) # Trade House

	mygump.addRawLayout( "{xmfhtmlgumpcolor 45 190 240 20 1061798 0 0 16912}" )

	mygump.setCallback( callback )
	mygump.setArgs( [item] )
	for line in gump_params:
		mygump.addRawText( line )
	mygump.send( char )

#
# Sub-pages: Co-owners, Friends, Bans, Access Lists
#
def gump10( char, callback, item ):
	# List Co-owners
	multi = get_house(item)
	gump_params = ["a house", "sign"]
	mygump = cGump()
	mygump.addRawLayout( "{page 0}" )
	mygump.addRawLayout( "{resizepic 0 0 5054 420 440}" )
	mygump.addRawLayout( "{gumppictiled 10 10 400 100 2624}" )
	mygump.addRawLayout( "{checkertrans 10 10 400 100}" )
	mygump.addRawLayout( "{gumppictiled 10 120 400 260 2624}" )
	mygump.addRawLayout( "{checkertrans 10 120 400 260}" )
	mygump.addRawLayout( "{gumppictiled 10 390 400 40 2624}" )
	mygump.addRawLayout( "{checkertrans 10 390 400 40}" )
	mygump.addRawLayout( "{button 250 410 4005 4007 1 0 0}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 285 410 240 20 1060675 0 0 32767}" )
	mygump.addRawLayout( "{gumppic 10 10 100}" )
	mygump.addRawLayout( "{text 62 43 0 0}" )
	mygump.addRawLayout( "{text 74 57 0 1}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 45 130 240 20 1060730 0 0 32767}" ) # CO-OWNER LIST

	offsetx = 0
	offsety = 0
	members = getCoOwners(multi)
	for memberserial in members:
		try:
			name = getCharName(int(memberserial))
			if name and name != "nobody":
				mygump.addText( 15 + offsety, 150 + offsetx, ' *', 1152 )
				mygump.addText( 49 + offsety, 150 + offsetx, name, 1152 )
				if offsetx == 192:
					offsetx = 0
					offsety += 200
				else:
					offsetx += 24
		except:
			pass
	mygump.setCallback( callback )
	mygump.setArgs( [item] )
	for line in gump_params:
		mygump.addRawText( line )
	mygump.send( char )

def gump11( char, callback, item ):
	# Remove Co-owner
	multi = get_house(item)
	gump_params = ["a house", "sign"]
	mygump = cGump()
	mygump.addRawLayout( "{page 0}" )
	mygump.addRawLayout( "{resizepic 0 0 5054 420 440}" )
	mygump.addRawLayout( "{gumppictiled 10 10 400 100 2624}" )
	mygump.addRawLayout( "{checkertrans 10 10 400 100}" )
	mygump.addRawLayout( "{gumppictiled 10 120 400 260 2624}" )
	mygump.addRawLayout( "{checkertrans 10 120 400 260}" )
	mygump.addRawLayout( "{gumppictiled 10 390 400 40 2624}" )
	mygump.addRawLayout( "{checkertrans 10 390 400 40}" )
	mygump.addRawLayout( "{button 250 410 4005 4007 1 0 0}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 285 410 240 20 1060675 0 0 32767}" )
	mygump.addRawLayout( "{gumppic 10 10 100}" )
	mygump.addRawLayout( "{text 62 43 0 0}" )
	mygump.addRawLayout( "{text 74 57 0 1}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 45 130 240 20 1011274 0 0 32767}" ) # Remove a co-owner

	offsetx = 0
	offsety = 0
	members = getCoOwners(multi)
	for memberserial in members:
		try:
			name = getCharName(int(memberserial))
			if name and name != "nobody":
				mygump.addButton( 15 + offsety, 150 + offsetx, 0xFA5, 0xFA7, int(memberserial) )
				mygump.addText( 49 + offsety, 150 + offsetx, name, 1152 )
				if offsetx == 192:
					offsetx = 0
					offsety += 200
				else:
					offsetx += 24
		except:
			pass
	mygump.setCallback( callbackremovecoowner )
	mygump.setArgs( [item] )
	for line in gump_params:
		mygump.addRawText( line )
	mygump.send( char )

def gump12( char, callback, item ):
	# List Friends
	multi = get_house(item)
	gump_params = ["a house", "sign"]
	mygump = cGump()
	mygump.addRawLayout( "{page 0}" )
	mygump.addRawLayout( "{resizepic 0 0 5054 420 440}" )
	mygump.addRawLayout( "{gumppictiled 10 10 400 100 2624}" )
	mygump.addRawLayout( "{checkertrans 10 10 400 100}" )
	mygump.addRawLayout( "{gumppictiled 10 120 400 260 2624}" )
	mygump.addRawLayout( "{checkertrans 10 120 400 260}" )
	mygump.addRawLayout( "{gumppictiled 10 390 400 40 2624}" )
	mygump.addRawLayout( "{checkertrans 10 390 400 40}" )
	mygump.addRawLayout( "{button 250 410 4005 4007 1 0 0}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 285 410 240 20 1060675 0 0 32767}" )
	mygump.addRawLayout( "{gumppic 10 10 100}" )
	mygump.addRawLayout( "{text 62 43 0 0}" )
	mygump.addRawLayout( "{text 74 57 0 1}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 45 130 240 20 1060731 0 0 32767}" ) # FRIENDS LIST

	offsetx = 0
	offsety = 0
	members = getFriends(multi)
	for memberserial in members:
		try:
			name = getCharName(int(memberserial))
			if name and name != "nobody":
				mygump.addText( 15 + offsety, 150 + offsetx, ' *', 1152 )
				mygump.addText( 49 + offsety, 150 + offsetx, name, 1152 )
				if offsetx == 192:
					offsetx = 0
					offsety += 200
				else:
					offsetx += 24
		except:
			pass
	mygump.setCallback( callback )
	mygump.setArgs( [item] )
	for line in gump_params:
		mygump.addRawText( line )
	mygump.send( char )

def gump13( char, callback, item ):
	# Remove Friend
	multi = get_house(item)
	gump_params = ["a house", "sign"]
	mygump = cGump()
	mygump.addRawLayout( "{page 0}" )
	mygump.addRawLayout( "{resizepic 0 0 5054 420 440}" )
	mygump.addRawLayout( "{gumppictiled 10 10 400 100 2624}" )
	mygump.addRawLayout( "{checkertrans 10 10 400 100}" )
	mygump.addRawLayout( "{gumppictiled 10 120 400 260 2624}" )
	mygump.addRawLayout( "{checkertrans 10 120 400 260}" )
	mygump.addRawLayout( "{gumppictiled 10 390 400 40 2624}" )
	mygump.addRawLayout( "{checkertrans 10 390 400 40}" )
	mygump.addRawLayout( "{button 250 410 4005 4007 1 0 0}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 285 410 240 20 1060675 0 0 32767}" )
	mygump.addRawLayout( "{gumppic 10 10 100}" )
	mygump.addRawLayout( "{text 62 43 0 0}" )
	mygump.addRawLayout( "{text 74 57 0 1}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 45 130 240 20 1011272 0 0 32767}" ) # Remove a friend

	offsetx = 0
	offsety = 0
	members = getFriends(multi)
	for memberserial in members:
		try:
			name = getCharName(int(memberserial))
			if name and name != "nobody":
				mygump.addButton( 15 + offsety, 150 + offsetx, 0xFA5, 0xFA7, int(memberserial) )
				mygump.addText( 49 + offsety, 150 + offsetx, name, 1152 )
				if offsetx == 192:
					offsetx = 0
					offsety += 200
				else:
					offsetx += 24
		except:
			pass
	mygump.setCallback( callbackremovefriend )
	mygump.setArgs( [item] )
	for line in gump_params:
		mygump.addRawText( line )
	mygump.send( char )

#
# Ban List Gump
#
def gump_ban_list( char, callback, item ):
	multi = get_house(item)
	bans = getBans(multi)
	gump_params = ["a house", "sign"]
	mygump = cGump()
	mygump.addRawLayout( "{page 0}" )
	mygump.addRawLayout( "{resizepic 0 0 5054 420 440}" )
	mygump.addRawLayout( "{gumppictiled 10 10 400 100 2624}" )
	mygump.addRawLayout( "{checkertrans 10 10 400 100}" )
	mygump.addRawLayout( "{gumppictiled 10 120 400 260 2624}" )
	mygump.addRawLayout( "{checkertrans 10 120 400 260}" )
	mygump.addRawLayout( "{gumppictiled 10 390 400 40 2624}" )
	mygump.addRawLayout( "{checkertrans 10 390 400 40}" )
	mygump.addRawLayout( "{button 250 410 4005 4007 1 0 0}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 285 410 240 20 1060675 0 0 32767}" )
	mygump.addRawLayout( "{gumppic 10 10 100}" )
	mygump.addRawLayout( "{text 62 43 0 0}" )
	mygump.addRawLayout( "{text 74 57 0 1}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 45 130 240 20 1060734 0 0 32767}" ) # BAN LIST

	offsetx = 0
	offsety = 0
	for serial_str in bans:
		try:
			name = getCharName(int(serial_str))
			if name and name != "nobody":
				mygump.addButton( 15 + offsety, 150 + offsetx, 0xFA5, 0xFA7, int(serial_str) )
				mygump.addText( 49 + offsety, 150 + offsetx, name, 1152 )
				if offsetx == 192:
					offsetx = 0
					offsety += 200
				else:
					offsetx += 24
		except:
			pass
	mygump.setCallback( callback_remove_ban )
	mygump.setArgs( [item] )
	for line in gump_params:
		mygump.addRawText( line )
	mygump.send( char )

#
# Access List Gump
#
def gump_access_list( char, callback, item ):
	multi = get_house(item)
	guests = getGuests(multi)
	gump_params = ["a house", "sign"]
	mygump = cGump()
	mygump.addRawLayout( "{page 0}" )
	mygump.addRawLayout( "{resizepic 0 0 5054 420 440}" )
	mygump.addRawLayout( "{gumppictiled 10 10 400 100 2624}" )
	mygump.addRawLayout( "{checkertrans 10 10 400 100}" )
	mygump.addRawLayout( "{gumppictiled 10 120 400 260 2624}" )
	mygump.addRawLayout( "{checkertrans 10 120 400 260}" )
	mygump.addRawLayout( "{gumppictiled 10 390 400 40 2624}" )
	mygump.addRawLayout( "{checkertrans 10 390 400 40}" )
	mygump.addRawLayout( "{button 250 410 4005 4007 1 0 0}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 285 410 240 20 1060675 0 0 32767}" )
	mygump.addRawLayout( "{gumppic 10 10 100}" )
	mygump.addRawLayout( "{text 62 43 0 0}" )
	mygump.addRawLayout( "{text 74 57 0 1}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 45 130 240 20 1060732 0 0 32767}" ) # ACCESS LIST

	offsetx = 0
	offsety = 0
	for serial_str in guests:
		try:
			name = getCharName(int(serial_str))
			if name and name != "nobody":
				mygump.addButton( 15 + offsety, 150 + offsetx, 0xFA5, 0xFA7, int(serial_str) )
				mygump.addText( 49 + offsety, 150 + offsetx, name, 1152 )
				if offsetx == 192:
					offsetx = 0
					offsety += 200
				else:
					offsetx += 24
		except:
			pass
	mygump.setCallback( callback_remove_access )
	mygump.setArgs( [item] )
	for line in gump_params:
		mygump.addRawText( line )
	mygump.send( char )

#
# Change Hanger Gump
#
def gump_change_hanger( char, callback, item ):
	mygump = cGump()
	mygump.addRawLayout( "{page 0}" )
	mygump.addRawLayout( "{resizepic 0 0 5054 420 440}" )
	mygump.addRawLayout( "{gumppictiled 10 10 400 100 2624}" )
	mygump.addRawLayout( "{checkertrans 10 10 400 100}" )
	mygump.addRawLayout( "{gumppictiled 10 120 400 260 2624}" )
	mygump.addRawLayout( "{checkertrans 10 120 400 260}" )
	mygump.addRawLayout( "{gumppictiled 10 390 400 40 2624}" )
	mygump.addRawLayout( "{checkertrans 10 390 400 40}" )
	mygump.addRawLayout( "{button 250 410 4005 4007 1 0 0}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 285 410 240 20 1060675 0 0 32767}" )
	mygump.addRawLayout( "{gumppic 10 10 100}" )
	mygump.addRawLayout( "{text 62 43 0 0}" )
	mygump.addRawLayout( "{text 74 57 0 1}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 45 130 320 20 1060762 0 0 32767}" ) # Change Sign Hanger

	for i in range(len(HANGER_NUMBERS)):
		x = 50 + ((i % 3) * 100)
		y = 180 + ((i // 3) * 80)
		mygump.addRawLayout( "{button %i %i 4005 4007 1 0 %i}" % (x, y, i + 1) )
		mygump.addRawLayout( "{tilepic %i %i %u}" % (x + 20, y, HANGER_NUMBERS[i]) )

	mygump.setCallback( change_hanger_callback )
	mygump.setArgs( [item] )
	mygump.addRawText( "a house" )
	mygump.addRawText( "sign" )
	mygump.send( char )

#
# Change Signpost Gump
#
def gump_change_post( char, callback, item ):
	mygump = cGump()
	mygump.addRawLayout( "{page 0}" )
	mygump.addRawLayout( "{resizepic 0 0 5054 420 440}" )
	mygump.addRawLayout( "{gumppictiled 10 10 400 100 2624}" )
	mygump.addRawLayout( "{checkertrans 10 10 400 100}" )
	mygump.addRawLayout( "{gumppictiled 10 120 400 260 2624}" )
	mygump.addRawLayout( "{checkertrans 10 120 400 260}" )
	mygump.addRawLayout( "{gumppictiled 10 390 400 40 2624}" )
	mygump.addRawLayout( "{checkertrans 10 390 400 40}" )
	mygump.addRawLayout( "{button 250 410 4005 4007 1 0 0}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 285 410 240 20 1060675 0 0 32767}" )
	mygump.addRawLayout( "{gumppic 10 10 100}" )
	mygump.addRawLayout( "{text 62 43 0 0}" )
	mygump.addRawLayout( "{text 74 57 0 1}" )

	total_pages = 3
	index = 0
	for p in range(total_pages):
		mygump.addRawLayout( "{page %i}" % (p + 1) )
		next_page = ((p + 1) % total_pages) + 1
		btn_pic = 4005 if p == 0 else 4014
		mygump.addRawLayout( "{button 10 360 %i %i 0 %i 0}" % (btn_pic, btn_pic + 2, next_page) )
		mygump.addRawLayout( "{xmfhtmlgumpcolor 45 360 200 20 1060763 0 0 32767}" ) # Change Signpost
		for j in range(16):
			if index >= len(POST_NUMBERS):
				break
			x = 15 + ((j % 8) * 50)
			y = 130 + ((j // 8) * 110)
			mygump.addRawLayout( "{button %i %i 4005 4007 1 0 %i}" % (x, y, index + 1) )
			mygump.addRawLayout( "{tilepic %i %i %u}" % (x + 10, y, POST_NUMBERS[index]) )
			index += 1

	mygump.setCallback( change_post_callback )
	mygump.setArgs( [item] )
	mygump.addRawText( "a house" )
	mygump.addRawText( "sign" )
	mygump.send( char )

#
# Change Foundation Style Gump
#
def gump_change_foundation( char, callback, item ):
	mygump = cGump()
	mygump.addRawLayout( "{page 0}" )
	mygump.addRawLayout( "{resizepic 0 0 5054 420 440}" )
	mygump.addRawLayout( "{gumppictiled 10 10 400 100 2624}" )
	mygump.addRawLayout( "{checkertrans 10 10 400 100}" )
	mygump.addRawLayout( "{gumppictiled 10 120 400 260 2624}" )
	mygump.addRawLayout( "{checkertrans 10 120 400 260}" )
	mygump.addRawLayout( "{gumppictiled 10 390 400 40 2624}" )
	mygump.addRawLayout( "{checkertrans 10 390 400 40}" )
	mygump.addRawLayout( "{button 250 410 4005 4007 1 0 0}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 285 410 240 20 1060675 0 0 32767}" )
	mygump.addRawLayout( "{gumppic 10 10 100}" )
	mygump.addRawLayout( "{text 62 43 0 0}" )
	mygump.addRawLayout( "{text 74 57 0 1}" )

	total_pages = 2
	index = 0
	for p in range(total_pages):
		mygump.addRawLayout( "{page %i}" % (p + 1) )
		next_page = ((p + 1) % total_pages) + 1
		btn_pic = 4005 if p == 0 else 4014
		mygump.addRawLayout( "{button 10 360 %i %i 0 %i 0}" % (btn_pic, btn_pic + 2, next_page) )
		mygump.addRawLayout( "{xmfhtmlgumpcolor 45 360 200 20 1062004 0 0 32767}" ) # Change Foundation Style
		for j in range(15):
			if index >= len(FOUNDATION_NUMBERS):
				break
			x = 15 + ((j % 5) * 80)
			y = 150 + ((j // 5) * 80)
			mygump.addRawLayout( "{button %i %i 4005 4007 1 0 %i}" % (x, y, index + 1) )
			mygump.addRawLayout( "{tilepic %i %i %u}" % (x + 25, y, FOUNDATION_NUMBERS[index]) )
			index += 1

	mygump.setCallback( change_foundation_callback )
	mygump.setArgs( [item] )
	mygump.addRawText( "a house" )
	mygump.addRawText( "sign" )
	mygump.send( char )

#
# Change House Sign Gump
#
def gump_change_sign( char, callback, item ):
	mygump = cGump()
	mygump.addRawLayout( "{page 0}" )
	mygump.addRawLayout( "{resizepic 0 0 5054 420 440}" )
	mygump.addRawLayout( "{gumppictiled 10 10 400 100 2624}" )
	mygump.addRawLayout( "{checkertrans 10 10 400 100}" )
	mygump.addRawLayout( "{gumppictiled 10 120 400 260 2624}" )
	mygump.addRawLayout( "{checkertrans 10 120 400 260}" )
	mygump.addRawLayout( "{gumppictiled 10 390 400 40 2624}" )
	mygump.addRawLayout( "{checkertrans 10 390 400 40}" )
	mygump.addRawLayout( "{button 250 410 4005 4007 1 0 0}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 285 410 240 20 1060675 0 0 32767}" )
	mygump.addRawLayout( "{gumppic 10 10 100}" )
	mygump.addRawLayout( "{text 62 43 0 0}" )
	mygump.addRawLayout( "{text 74 57 0 1}" )

	signs_per_page = 24
	total_signs = len(HOUSE_SIGNS)
	pages = (total_signs + signs_per_page - 1) // signs_per_page
	index = 0
	for p in range(pages):
		mygump.addRawLayout( "{page %i}" % (p + 1) )
		next_page = ((p + 1) % pages) + 1
		btn_pic = 4005 if p == 0 else 4014
		mygump.addRawLayout( "{button 10 360 %i %i 0 %i 0}" % (btn_pic, btn_pic + 2, next_page) )
		mygump.addRawLayout( "{xmfhtmlgumpcolor 45 360 200 20 1060761 0 0 32767}" ) # Change House Sign
		for j in range(signs_per_page):
			if index >= total_signs:
				break
			x = 30 + ((j % 6) * 60)
			y = 130 + ((j // 6) * 60)
			mygump.addRawLayout( "{button %i %i 4005 4007 1 0 %i}" % (x, y, index + 1) )
			mygump.addRawLayout( "{tilepic %i %i %u}" % (x + 20, y, HOUSE_SIGNS[index]) )
			index += 1

	mygump.setCallback( change_sign_callback )
	mygump.setArgs( [item] )
	mygump.addRawText( "a house" )
	mygump.addRawText( "sign" )
	mygump.send( char )

#
# Rename House Dialog
#
def gump_rename_house( char, callback, item ):
	mygump = cGump( nomove=1, x=100, y=150 )
	mygump.addResizeGump( 0, 0, 9200, 260, 160 )
	mygump.addText( 50, 15, "Rename your House", 0 )
	mygump.addResizeGump( 20, 45, 9350, 220, 60 )
	current_name = item.name or "a house"
	mygump.addInputField( 28, 65, 200, 20, 0, 1, current_name )
	mygump.addButton( 40, 115, 247, 248, 1 )
	mygump.addButton( 140, 115, 241, 242, 0 )

	mygump.setCallback( rename_house_callback )
	mygump.setArgs( [item] )
	mygump.send( char.socket )

#
# Callbacks for new gumps
#
def change_hanger_callback( char, args, choice ):
	item = args[0]
	button = choice.button
	if button == 0:
		gump3( char, gumpcallback, item )
		return
	index = button - 1
	if 0 <= index < len(HANGER_NUMBERS):
		new_id = HANGER_NUMBERS[index]
		hanger = get_or_find_hanger(item)
		if hanger:
			hanger.id = new_id
			hanger.update()
			char.socket.sysmessage("Sign hanger has been updated.")
	gump3( char, gumpcallback, item )

def change_post_callback( char, args, choice ):
	item = args[0]
	button = choice.button
	if button == 0:
		gump3( char, gumpcallback, item )
		return
	index = button - 1
	if 0 <= index < len(POST_NUMBERS):
		new_id = POST_NUMBERS[index]
		post = get_or_find_post(item)
		if post:
			post.id = new_id
			post.update()
			char.socket.sysmessage("Signpost has been updated.")
	gump3( char, gumpcallback, item )

def change_foundation_callback( char, args, choice ):
	item = args[0]
	button = choice.button
	if button == 0:
		gump3( char, gumpcallback, item )
		return
	index = button - 1
	if 0 <= index < len(FOUNDATION_NUMBERS):
		multi = get_house(item)
		if multi:
			import housing.customhouse as ch
			ch.apply_foundation_change(multi, index)
			char.socket.sysmessage("Foundation style updated.")
	gump3( char, gumpcallback, item )

def change_sign_callback( char, args, choice ):
	item = args[0]
	button = choice.button
	if button == 0:
		gump3( char, gumpcallback, item )
		return
	index = button - 1
	if 0 <= index < len(HOUSE_SIGNS):
		new_id = HOUSE_SIGNS[index]
		item.id = new_id
		item.update()
		char.socket.sysmessage("House sign has been updated.")
	gump3( char, gumpcallback, item )

def rename_house_callback( char, args, target ):
	item = args[0]
	button = target.button
	if button == 1:
		new_name = target.text.get(1, '').strip()
		if len(new_name) > 0:
			item.name = new_name
			item.resendtooltip()
			multi = get_house(item)
			if multi:
				multi.name = new_name
			char.socket.sysmessage("House has been renamed to '%s'." % new_name)
	gump3( char, gumpcallback, item )

def callback_remove_ban( char, args, choice ):
	item = args[0]
	serial = choice.button
	if serial == 0:
		gump1( char, gumpcallback, item )
		return
	multi = get_house(item)
	banned_char = wolfpack.findchar(serial)
	if banned_char:
		removeBan(multi, banned_char)
		char.socket.sysmessage('You have removed %s from the ban list.' % banned_char.name)
	else:
		bans = getBans(multi)
		s_str = str(serial)
		if s_str in bans:
			bans.remove(s_str)
			multi.settag('bans', ','.join(bans))
			char.socket.sysmessage('Removed from ban list.')
	gump_ban_list( char, gumpcallback, item )

def callback_remove_access( char, args, choice ):
	item = args[0]
	serial = choice.button
	if serial == 0:
		gump1( char, gumpcallback, item )
		return
	multi = get_house(item)
	guest_char = wolfpack.findchar(serial)
	if guest_char:
		removeGuest(multi, guest_char)
		char.socket.sysmessage('You have removed %s from the access list.' % guest_char.name)
	else:
		guests = getGuests(multi)
		s_str = str(serial)
		if s_str in guests:
			guests.remove(s_str)
			multi.settag('guests', ','.join(guests))
			char.socket.sysmessage('Removed from access list.')
	gump_access_list( char, gumpcallback, item )

def callbackremovecoowner( char, args, choice ):
	item = args[0]
	serial = choice.button
	if serial == 0:
		gump1( char, gumpcallback, item )
		return
	name = getCharName(serial)
	removeCoOwner(get_house(item), serial)
	char.socket.sysmessage('You have just removed %s from the co-owners list' % name)
	gump11( char, gumpcallback, item )

def callbackremovefriend( char, args, choice ):
	item = args[0]
	serial = choice.button
	if serial == 0:
		gump1( char, gumpcallback, item )
		return
	name = getCharName(serial)
	removeFriend(get_house(item), serial)
	char.socket.sysmessage('You have just removed %s from the friends list' % name)
	gump13( char, gumpcallback, item )

#
# Action Handlers: Co-owners, Friends, Access, Ban, Public/Private, Trade, Demolish
#
def addcoown_target(char, arguments, target):
	item = arguments[0]
	if not target.char or not target.char.player:
		char.socket.sysmessage('You must target a player.')
		return
	addCoOwner(get_house(item), target.char)
	char.socket.sysmessage('You added %s as a new co-owner.' % target.char.name)

def addcoown( char, item ):
	if not checkAccess(char, get_house(item), ACCESS_OWNER):
		char.socket.sysmessage('You are not the owner of this house.')
		return
	char.socket.sysmessage('Who do you want to add as co-owner?')
	char.socket.attachtarget("signpost.addcoown_target", [item])

def removeallcoown(char, item):
	if not checkAccess(char, get_house(item), ACCESS_OWNER):
		char.socket.sysmessage('You are not the owner of this house.')
		return
	removeAllCoOwner(get_house(item))
	char.socket.sysmessage('You have just removed all co-owners.')
	gump1( char, gumpcallback, item )

def addfriend_target(char, arguments, target):
	item = arguments[0]
	if not target.char or not target.char.player:
		char.socket.sysmessage('You must target a player.')
		return
	addFriend(get_house(item), target.char)
	char.socket.sysmessage('You added %s as a new friend.' % target.char.name)

def addfriend( char, item ):
	if not checkAccess(char, get_house(item), ACCESS_COOWNER):
		char.socket.sysmessage('You are not authorized to add friends.')
		return
	char.socket.sysmessage('Who do you want to add as friend?')
	char.socket.attachtarget("signpost.addfriend_target", [item])

def removeallfriend(char, item):
	if not checkAccess(char, get_house(item), ACCESS_COOWNER):
		char.socket.sysmessage('You are not authorized to remove friends.')
		return
	removeAllFriend(get_house(item))
	char.socket.sysmessage('You have just removed all friends.')
	gump1( char, gumpcallback, item )

def grant_access_target(char, arguments, target):
	item = arguments[0]
	if not target.char or not target.char.player:
		char.socket.sysmessage('You must target a player.')
		return
	multi = get_house(item)
	addGuest(multi, target.char)
	char.socket.sysmessage('You granted access to %s.' % target.char.name)

def grant_access(char, item):
	multi = get_house(item)
	if not checkAccess(char, multi, ACCESS_COOWNER):
		char.socket.sysmessage('You are not authorized to grant access.')
		return
	char.socket.sysmessage('Who do you want to grant access to?')
	char.socket.attachtarget("signpost.grant_access_target", [item])

def revoke_access_target(char, arguments, target):
	item = arguments[0]
	if not target.char or not target.char.player:
		char.socket.sysmessage('You must target a player.')
		return
	multi = get_house(item)
	removeGuest(multi, target.char)
	char.socket.sysmessage('You revoked access from %s.' % target.char.name)

def revoke_access(char, item):
	multi = get_house(item)
	if not checkAccess(char, multi, ACCESS_COOWNER):
		char.socket.sysmessage('You are not authorized to revoke access.')
		return
	char.socket.sysmessage('Who do you want to revoke access from?')
	char.socket.attachtarget("signpost.revoke_access_target", [item])

def toggle_public_private(char, item):
	multi = get_house(item)
	if not checkAccess(char, multi, ACCESS_OWNER):
		char.socket.sysmessage('You are not the owner of this house.')
		return
	currently_public = isPublic(multi)
	setPublic(multi, not currently_public)
	item.resendtooltip()
	if not currently_public:
		char.socket.sysmessage('This house is now open to the public.')
	else:
		char.socket.sysmessage('This house is now private.')
	gump1(char, gumpcallback, item)

def trade_house_target(char, arguments, target):
	item = arguments[0]
	if not target.char or not target.char.player:
		char.socket.sysmessage('You must target a player.')
		return
	if target.char == char:
		char.socket.sysmessage('You cannot transfer the house to yourself.')
		return
	if not char.canreach(target.char.pos, 5):
		char.socket.sysmessage('The player must be near you to transfer the house.')
		return
	multi = get_house(item)
	multi.owner = target.char
	item.resendtooltip()
	char.socket.sysmessage('You have transferred ownership of the house to %s.' % target.char.name)
	if target.char.socket:
		target.char.socket.sysmessage('%s has transferred ownership of this house to you.' % char.name)

def trade_house(char, item):
	multi = get_house(item)
	if not checkAccess(char, multi, ACCESS_OWNER):
		char.socket.sysmessage('You are not the owner of this house.')
		return
	char.socket.sysmessage('Target the player you wish to transfer this house to.')
	char.socket.attachtarget("signpost.trade_house_target", [item])

def relocate_moving_crate(char, item):
	multi = get_house(item)
	if not checkAccess(char, multi, ACCESS_OWNER):
		char.socket.sysmessage('You are not the owner of this house.')
		return
	if multi and multi.hastag('moving_crate'):
		crate = wolfpack.finditem(int(multi.gettag('moving_crate')))
		if crate:
			crate.moveto(char.pos)
			crate.update()
			char.socket.sysmessage('Moving crate has been relocated to your position.')
			return
	char.socket.sysmessage('There is no moving crate for this house.')

def customize( char, item ):
	multi = get_house(item)
	if not multi:
		return
	if not checkAccess(char, multi, ACCESS_OWNER):
		char.socket.sysmessage('You are not the owner of this house.')
		return
	import housing.customhouse as ch
	ch.start_customizing( char, multi )
	return 1

def demolish( char, item ):
	multi = get_house(item)
	if not multi:
		return
	if not checkAccess(char, multi, ACCESS_OWNER):
		char.socket.sysmessage('You are not the owner of this house.')
		return

	listitems = multi.objects
	contador = 0
	for multiitem in listitems:
		try:
			n = multiitem.player
		except:
			multiitem.delete()
			contador += 1

	char.socket.sysmessage( "Deleted %i items in house!" % contador )
	housing.unregisterHouse(multi)
	multi.delete()

#
# Central Gump Routing
#
def switchgump( char, target, args ):
	item = args[0]
	button = target.button

	gumphandler = {
		2: gump0, # information
		17: gump1, # security
		32: gump2, # storage
		47: gump3, # customize
		62: gump4, # ownership
		4: gump10, # list co owner
		34: gump11, # remove a co owner
		64: gump12, # list friend
		94: gump13, # remove a friend
		154: gump_ban_list, # view ban list
		169: gump_access_list, # view access list
		66: gump_change_hanger, # change sign hanger
		81: gump_change_post, # change signpost
		96: gump_change_foundation, # change foundation style
		111: gump_rename_house, # rename house
		210: gump_change_sign, # change house sign
	}

	actionhandler = {
		7: demolish,
		21: customize,
		19: addcoown,
		49: removeallcoown,
		79: addfriend,
		109: removeallfriend,
		31: grant_access,
		46: revoke_access,
		199: toggle_public_private,
		22: trade_house,
		180: relocate_moving_crate,
	}

	if button in gumphandler:
		gumphandler[ button ]( char, gumpcallback, item )
	elif button in actionhandler:
		actionhandler[ button ]( char, item )
	elif button == 0:
		return
	else:
		char.socket.sysmessage( "Button %i isn't working yet" % button )
		return 1
