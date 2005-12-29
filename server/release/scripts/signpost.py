
import wolfpack
import housing
from housing.house import *
from wolfpack.gumps import cGump
from wolfpack.consts import EVENT_CHLEVELCHANGE

# Register as a global script
def onLoad():
	wolfpack.registerglobal( EVENT_CHLEVELCHANGE, "signpost" )

def onCHLevelChange( char, level ):
	if not char.hastag( 'customizing' ):
		return
	multi = wolfpack.findobject( char.gettag( 'customizing' ) )

	char.socket.sysmessage( 'Level selected: ' + str( level ) )
	char.socket.sysmessage( 'multi at: ' + str( multi.pos.z ) )
	char.socket.sysmessage( 'char at: ' + str( char.pos.z ) )
	
	#alt = (level-1) * 49

	if level == 3:
		char.moveto( wolfpack.coord( multi.pos.x, multi.pos.y, multi.pos.z+47, multi.pos.map ) )
	if level == 2:
		char.moveto( wolfpack.coord( multi.pos.x, multi.pos.y, multi.pos.z+27, multi.pos.map ) )
	if level == 1:
		char.moveto( wolfpack.coord( multi.pos.x, multi.pos.y, multi.pos.z+7, multi.pos.map ) )
	char.update()
	return 1

def onUse( char, item ):
	gump0( char, gumpcallback, item )
	return True

def gumpcallback( char, args, target ):
	switchgump( char, target, args )


def gump0( char, callback, item ):

	# Lets try Multi and some informations
	multi = wolfpack.findobject( item.gettag( 'house' ) )
	builton = str(multi.gettag('builton'))
	mvalue = str(multi.gettag('value'))

	gump_params = [
	"a house",
	"sign",
	str(item.multi.owner.name),
	str(builton),
	"never",
	str(mvalue),
	"0"]

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
	mygump.addRawLayout( "{xmfhtmlgumpcolor 285 410 240 20 1060675 0 0 32767}" ) #write close
	mygump.addRawLayout( "{gumppic 10 10 100}" )
	mygump.addRawLayout( "{text 62 43 0 0}" )
	mygump.addRawLayout( "{text 74 57 0 1}" )
	mygump.addRawLayout( "{button 10 390 4005 4007 1 0 31}" ) #grant access 
	mygump.addRawLayout( "{xmfhtmlgumpcolor 45 390 240 20 1060676 0 0 32767}" )# write grant access
	mygump.addRawLayout( "{button 10 410 4005 4007 1 0 46}" ) #revoke access
	mygump.addRawLayout( "{xmfhtmlgumpcolor 45 410 240 20 1060677 0 0 32767}" )
	mygump.addRawLayout( "{button 150 10 4006 4007 1 0 2}" ) #open information page
	mygump.addRawLayout( "{xmfhtmlgumpcolor 195 10 200 20 1060668 0 0 16927}" )
	mygump.addRawLayout( "{button 150 30 4005 4007 1 0 17}" )#open  security page
	mygump.addRawLayout( "{xmfhtmlgumpcolor 195 30 200 20 1060669 0 0 32767}" )
	mygump.addRawLayout( "{button 150 50 4005 4007 1 0 32}" )#open  storage page
	mygump.addRawLayout( "{xmfhtmlgumpcolor 195 50 200 20 1060670 0 0 32767}" )
	mygump.addRawLayout( "{button 150 70 4005 4007 1 0 47}" )#open  customize page
	mygump.addRawLayout( "{xmfhtmlgumpcolor 195 70 200 20 1060671 0 0 32767}" )
	mygump.addRawLayout( "{button 150 90 4005 4007 1 0 62}" )#open  ownership page
	mygump.addRawLayout( "{xmfhtmlgumpcolor 195 90 200 20 1060672 0 0 32767}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 20 130 200 20 1011242 0 0 32767}" )#write :Owned By: 
	mygump.addRawLayout( "{text 210 130 1152 2}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 20 170 380 20 1018032 0 0 16927}" ) #This house is properly placed.
	mygump.addRawLayout( "{xmfhtmlgumpcolor 20 190 380 20 1018035 0 0 16927}" ) #This house is of classic design.
	mygump.addRawLayout( "{xmfhtmlgumpcolor 20 210 380 20 1060681 0 0 16927}" ) #This is a custom-built house.
	if isPublic(item.multi):
		mygump.addRawLayout( "{xmfhtmlgumpcolor 20 230 380 20 1060678 0 0 16927}" ) # This house is open to the public.
	else:
		mygump.addRawLayout( "{xmfhtmlgumpcolor 20 230 380 20 1060679 0 0 16927}" ) #This house is private
	mygump.addRawLayout( "{xmfhtmlgumpcolor 20 250 380 20 1062209 0 0 16927}" ) #This house is <a href = "?ForceTopic97">Automatically</a> refreshed.
	mygump.addRawLayout( "{xmfhtmlgumpcolor 20 290 200 20 1060692 0 0 16927}" ) #Built On:
	mygump.addRawLayout( "{text 250 290 1152 3}" ) #write third parameter 3 in white
	mygump.addRawLayout( "{xmfhtmlgumpcolor 20 310 200 20 1060693 0 0 16927}" )#last traded
	mygump.addRawLayout( "{text 250 310 1152 4}" ) # write param4
	mygump.addRawLayout( "{xmfhtmlgumpcolor 20 330 200 20 1061793 0 0 16927}" )#house value
	mygump.addRawLayout( "{text 250 330 1152 5}" ) #write param5
	mygump.addRawLayout( "{xmfhtmlgumpcolor 20 360 300 20 1011241 0 0 16927}" )#nbr of visit
	mygump.addRawLayout( "{text 350 360 1152 6}" ) #write param 6
	mygump.setCallback( callback )
	mygump.setArgs( [item] )
	#Add params
	for line in gump_params:
		mygump.addRawText( line )
	mygump.send( char )

def gump1( char, callback, item ):
	#security
	gump_params = [
	"a house",
	"sign"]

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
	mygump.addRawLayout( "{button 10 390 4005 4007 1 0 31}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 45 390 240 20 1060676 0 0 32767}" )
	mygump.addRawLayout( "{button 10 410 4005 4007 1 0 46}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 45 410 240 20 1060677 0 0 32767}" )
	mygump.addRawLayout( "{button 150 10 4005 4007 1 0 2}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 195 10 200 20 1060668 0 0 32767}" )
	mygump.addRawLayout( "{button 150 30 4006 4007 1 0 17}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 195 30 200 20 1060669 0 0 16927}" )
	mygump.addRawLayout( "{button 150 50 4005 4007 1 0 32}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 195 50 200 20 1060670 0 0 32767}" )
	mygump.addRawLayout( "{button 150 70 4005 4007 1 0 47}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 195 70 200 20 1060671 0 0 32767}" )
	mygump.addRawLayout( "{button 150 90 4005 4007 1 0 62}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 195 90 200 20 1060672 0 0 32767}" ) #end of common part with gump0
	if checkAccess(char, item.multi, ACCESS_COOWNER):
		mygump.addRawLayout( "{button 10 130 4005 4007 1 0 4}" ) # button see title next line
	mygump.addRawLayout( "{xmfhtmlgumpcolor 45 130 240 20 1011266 0 0 32767}" ) #View Co-Owner List
	if checkAccess(char, item.multi, ACCESS_OWNER):
		mygump.addRawLayout( "{button 10 150 4005 4007 1 0 19}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 45 150 240 20 1011267 0 0 32767}" ) #Add a Co-Owner
	if checkAccess(char, item.multi, ACCESS_OWNER):
		mygump.addRawLayout( "{button 10 170 4005 4007 1 0 34}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 45 170 240 20 1018036 0 0 32767}" )
	if checkAccess(char, item.multi, ACCESS_OWNER):
		mygump.addRawLayout( "{button 10 190 4005 4007 1 0 49}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 45 190 240 20 1011268 0 0 32767}" )
	if checkAccess(char, item.multi, ACCESS_COOWNER):
		mygump.addRawLayout( "{button 10 220 4005 4007 1 0 64}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 45 220 240 20 1011243 0 0 32767}" )
	if checkAccess(char, item.multi, ACCESS_COOWNER):
		mygump.addRawLayout( "{button 10 240 4005 4007 1 0 79}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 45 240 240 20 1011244 0 0 32767}" )
	if checkAccess(char, item.multi, ACCESS_COOWNER):
		mygump.addRawLayout( "{button 10 260 4005 4007 1 0 94}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 45 260 240 20 1018037 0 0 32767}" )
	if checkAccess(char, item.multi, ACCESS_COOWNER):
		mygump.addRawLayout( "{button 10 280 4005 4007 1 0 109}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 45 280 240 20 1011245 0 0 32767}" )
	if checkAccess(char, item.multi, ACCESS_FRIEND):
		mygump.addRawLayout( "{button 10 310 4005 4007 1 0 154}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 45 310 240 20 1060699 0 0 32767}" )
	if checkAccess(char, item.multi, ACCESS_FRIEND):
		mygump.addRawLayout( "{button 10 330 4005 4007 1 0 169}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 45 330 240 20 1060700 0 0 32767}" )
	if checkAccess(char, item.multi, ACCESS_OWNER):
		mygump.addRawLayout( "{button 210 130 4005 4007 1 0 199}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 245 130 240 20 1060694 0 0 32767}" )
	mygump.setCallback( callback )
	mygump.setArgs( [item] )
	#Add params
	for line in gump_params:
		mygump.addRawText( line )
	mygump.send( char )


def gump2( char, callback, item ):
	gump_params = [
	"a house",
	"sign",
	"0",
	"425",
	"212"]

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
	mygump.addRawLayout( "{button 10 390 4005 4007 1 0 31}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 45 390 240 20 1060676 0 0 32767}" )
	mygump.addRawLayout( "{button 10 410 4005 4007 1 0 46}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 45 410 240 20 1060677 0 0 32767}" )
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
	#Add params
	for line in gump_params:
		mygump.addRawText( line )
	mygump.send( char )


# Customization Gump
def gump3( char, callback, item ):
	gump_params = [
	str(item.multi.name),
	"sign"]

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
	mygump.addRawLayout( "{button 10 390 4005 4007 1 0 31}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 45 390 240 20 1060676 0 0 32767}" )
	mygump.addRawLayout( "{button 10 410 4005 4007 1 0 46}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 45 410 240 20 1060677 0 0 32767}" )
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
	mygump.addRawLayout( "{xmfhtmlgumpcolor 45 120 240 20 1060759 0 0 16912}" )
	# Customize this House - Just to Custom Houses (Checked by wich script sign has)
	if item.hasscript( 'signpost' ):
		mygump.addRawLayout( "{button 10 160 4005 4007 1 0 21}" )
		mygump.addRawLayout( "{xmfhtmlgumpcolor 45 160 240 20 1060765 0 0 32767}" )
	else:
		mygump.addRawLayout( "{xmfhtmlgumpcolor 45 160 240 20 1060765 0 0 16912}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 45 180 240 20 1060760 0 0 16912}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 45 210 240 20 1060761 0 0 16912}" )
	mygump.addRawLayout( "{button 10 230 4005 4007 1 0 66}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 45 230 240 20 1060762 0 0 32767}" )
	mygump.addRawLayout( "{button 10 250 4005 4007 1 0 81}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 45 250 240 20 1060763 0 0 32767}" )
	mygump.addRawLayout( "{button 10 280 4005 4007 1 0 96}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 45 280 240 20 1062004 0 0 32767}" )
	mygump.addRawLayout( "{button 10 310 4005 4007 1 0 111}" ) #rename house
	mygump.addRawLayout( "{xmfhtmlgumpcolor 45 310 240 20 1060764 0 0 32767}" )
	mygump.setCallback( callback )
	mygump.setArgs( [item] )
	#Add params
	for line in gump_params:
		mygump.addRawText( line )

	mygump.send( char )

def gump4( char, callback, item ):
	gump_params = [
	"a house",
	"sign"]
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
	mygump.addRawLayout( "{button 10 390 4005 4007 1 0 31}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 45 390 240 20 1060676 0 0 32767}" )
	mygump.addRawLayout( "{button 10 410 4005 4007 1 0 46}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 45 410 240 20 1060677 0 0 32767}" )
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
	if checkAccess(char, item.multi, ACCESS_OWNER):
		mygump.addRawLayout( "{button 10 130 4005 4007 1 0 7}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 45 130 240 20 1061794 0 0 32767}" )
	if checkAccess(char, item.multi, ACCESS_OWNER):
		mygump.addRawLayout( "{button 10 150 4005 4007 1 0 22}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 45 150 240 20 1061797 0 0 32767}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 45 190 240 20 1061798 0 0 16912}" )
	mygump.setCallback( callback )
	mygump.setArgs( [item] )
	#Add params
	for line in gump_params:
		mygump.addRawText( line )
	mygump.send( char )

def gump10( char, callback, item ):
	#security list co owner
	gump_params = [
	"a house",
	"sign"]

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
	mygump.addRawLayout( "{button 10 390 4005 4007 1 0 31}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 45 390 240 20 1060676 0 0 32767}" )
	mygump.addRawLayout( "{button 10 410 4005 4007 1 0 46}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 45 410 240 20 1060677 0 0 32767}" )
	mygump.addRawLayout( "{button 150 10 4005 4007 1 0 2}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 195 10 200 20 1060668 0 0 32767}" )
	mygump.addRawLayout( "{button 150 30 4006 4007 1 0 17}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 195 30 200 20 1060669 0 0 16927}" )
	mygump.addRawLayout( "{button 150 50 4005 4007 1 0 32}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 195 50 200 20 1060670 0 0 32767}" )
	mygump.addRawLayout( "{button 150 70 4005 4007 1 0 47}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 195 70 200 20 1060671 0 0 32767}" )
	mygump.addRawLayout( "{button 150 90 4005 4007 1 0 62}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 195 90 200 20 1060672 0 0 32767}" ) #end of common part with gump0
	#mygump.addRawLayout( "{button 10 130 4005 4007 1 0 4}" ) # button see title next line
	mygump.addRawLayout( "{xmfhtmlgumpcolor 45 130 240 20 1011266 0 0 32767}" ) #View Co-Owner List
	
	offsetx = 0
	offsety= 0
	members = getCoOwners(item.multi)
	for memberserial in members:
		try :
			member = wolfpack.findchar(int(memberserial))
			#~ mygump.addButton( 15 +offsety, 150+offsetx, 0xFA5, 0xFA7, member.serial )
			mygump.addText( 15 +offsety, 150+offsetx, unicode(' *'), 1152)
			mygump.addText(49 +offsety, 150+offsetx, unicode(member.name), 1152)
			if offsetx == 192:
				offsetx = 0
				offsety += 200
			else:
				offsetx += 24			
		except:
			pass
	mygump.setCallback( callback )
	mygump.setArgs( [item] )
	#Add params
	for line in gump_params:
		mygump.addRawText( line )
	mygump.send( char )

def gump11( char, callback, item ):
	#security remove a co owner
	gump_params = [
	"a house",
	"sign"]

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
	#remove button cause change of callback
	#~ mygump.addRawLayout( "{button 10 390 4005 4007 1 0 31}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 45 390 240 20 1060676 0 0 32767}" )
	#~ mygump.addRawLayout( "{button 10 410 4005 4007 1 0 46}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 45 410 240 20 1060677 0 0 32767}" )
	#~ mygump.addRawLayout( "{button 150 10 4005 4007 1 0 2}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 195 10 200 20 1060668 0 0 32767}" )
	#~ mygump.addRawLayout( "{button 150 30 4006 4007 1 0 17}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 195 30 200 20 1060669 0 0 16927}" )
	#~ mygump.addRawLayout( "{button 150 50 4005 4007 1 0 32}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 195 50 200 20 1060670 0 0 32767}" )
	#~ mygump.addRawLayout( "{button 150 70 4005 4007 1 0 47}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 195 70 200 20 1060671 0 0 32767}" )
	#~ mygump.addRawLayout( "{button 150 90 4005 4007 1 0 62}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 195 90 200 20 1060672 0 0 32767}" ) #end of common part with gump0
	#mygump.addRawLayout( "{button 10 130 4005 4007 1 0 4}" ) # button see title next line
	mygump.addRawLayout( "{xmfhtmlgumpcolor 45 130 240 20 1011274 0 0 32767}" ) #View Remove a co-owner from the house
	#~ mygump.addRawLayout( "{text 45 170 1152 2}" )#~ mygump.addRawLayout( "{text 45 150 1152 2}" ) #ecrit le parametre 2 en blanc
	
	offsetx = 0
	offsety= 0
	members = getCoOwners(item.multi)
	#~ char.socket.sysmessage('liste %s'%members)
	for memberserial in members:
		try :
			member = wolfpack.findchar(int(memberserial))
			mygump.addButton( 15 +offsety, 150+offsetx, 0xFA5, 0xFA7, member.serial )
			#~ mygump.addText( 15 +offsety, 150+offsetx, unicode(' *'), 1152)
			mygump.addText(49 +offsety, 150+offsetx, unicode(member.name), 1152)
			if offsetx == 192:
				offsetx = 0
				offsety += 200
			else:
				offsetx += 24			
		except:
			pass
	mygump.setCallback( callbackremovecoowner )
	mygump.setArgs( [item] )
	#Add params
	for line in gump_params:
		mygump.addRawText( line )
	mygump.send( char )

def gump12( char, callback, item ):
	#security view friends list
	gump_params = [
	"a house",
	"sign"]

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
	mygump.addRawLayout( "{button 10 390 4005 4007 1 0 31}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 45 390 240 20 1060676 0 0 32767}" )
	mygump.addRawLayout( "{button 10 410 4005 4007 1 0 46}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 45 410 240 20 1060677 0 0 32767}" )
	mygump.addRawLayout( "{button 150 10 4005 4007 1 0 2}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 195 10 200 20 1060668 0 0 32767}" )
	mygump.addRawLayout( "{button 150 30 4006 4007 1 0 17}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 195 30 200 20 1060669 0 0 16927}" )
	mygump.addRawLayout( "{button 150 50 4005 4007 1 0 32}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 195 50 200 20 1060670 0 0 32767}" )
	mygump.addRawLayout( "{button 150 70 4005 4007 1 0 47}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 195 70 200 20 1060671 0 0 32767}" )
	mygump.addRawLayout( "{button 150 90 4005 4007 1 0 62}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 195 90 200 20 1060672 0 0 32767}" ) #end of common part with gump0
	#mygump.addRawLayout( "{button 10 130 4005 4007 1 0 4}" ) # button see title next line
	mygump.addRawLayout( "{xmfhtmlgumpcolor 45 130 240 20 1011243 0 0 32767}" ) #View friend List
	
	offsetx = 0
	offsety= 0
	members = getFriends(item.multi)
	for memberserial in members:
		try :
			member = wolfpack.findchar(int(memberserial))
			#~ mygump.addButton( 15 +offsety, 150+offsetx, 0xFA5, 0xFA7, member.serial )
			mygump.addText( 15 +offsety, 150+offsetx, unicode(' *'), 1152)
			mygump.addText(49 +offsety, 150+offsetx, unicode(member.name), 1152)
			if offsetx == 192:
				offsetx = 0
				offsety += 200
			else:
				offsetx += 24			
		except:
			pass
	mygump.setCallback( callback )
	mygump.setArgs( [item] )
	#Add params
	for line in gump_params:
		mygump.addRawText( line )
	mygump.send( char )

def gump13( char, callback, item ):
	#security remove a friend
	gump_params = [
	"a house",
	"sign"]

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
	#remove button cause change of callback
	#~ mygump.addRawLayout( "{button 10 390 4005 4007 1 0 31}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 45 390 240 20 1060676 0 0 32767}" )
	#~ mygump.addRawLayout( "{button 10 410 4005 4007 1 0 46}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 45 410 240 20 1060677 0 0 32767}" )
	#~ mygump.addRawLayout( "{button 150 10 4005 4007 1 0 2}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 195 10 200 20 1060668 0 0 32767}" )
	#~ mygump.addRawLayout( "{button 150 30 4006 4007 1 0 17}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 195 30 200 20 1060669 0 0 16927}" )
	#~ mygump.addRawLayout( "{button 150 50 4005 4007 1 0 32}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 195 50 200 20 1060670 0 0 32767}" )
	#~ mygump.addRawLayout( "{button 150 70 4005 4007 1 0 47}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 195 70 200 20 1060671 0 0 32767}" )
	#~ mygump.addRawLayout( "{button 150 90 4005 4007 1 0 62}" )
	mygump.addRawLayout( "{xmfhtmlgumpcolor 195 90 200 20 1060672 0 0 32767}" ) #end of common part with gump0
	#mygump.addRawLayout( "{button 10 130 4005 4007 1 0 4}" ) # button see title next line
	mygump.addRawLayout( "{xmfhtmlgumpcolor 45 130 240 20 1011272 0 0 32767}" ) #View Remove a friend from the house
	
	offsetx = 0
	offsety= 0
	members = getFriends(item.multi)
	#~ char.socket.sysmessage('liste %s'%members)
	for memberserial in members:
		try :
			member = wolfpack.findchar(int(memberserial))
			mygump.addButton( 15 +offsety, 150+offsetx, 0xFA5, 0xFA7, member.serial )
			#~ mygump.addText( 15 +offsety, 150+offsetx, unicode(' *'), 1152)
			mygump.addText(49 +offsety, 150+offsetx, unicode(member.name), 1152)
			if offsetx == 192:
				offsetx = 0
				offsety += 200
			else:
				offsetx += 24			
		except:
			pass
	mygump.setCallback( callbackremovefriend )
	mygump.setArgs( [item] )
	#Add params
	for line in gump_params:
		mygump.addRawText( line )
	mygump.send( char )

def callbackremovecoowner( char,args , choice):
	item = args[0]
	#~ elif choice.button > 2:
	serial = choice.button
	if serial ==0:
		return
	player = wolfpack.findchar(serial)
	#~ details(char, player)
	removeCoOwner(item.multi, player)
	char.socket.sysmessage('You have just remove %s from the co-owners list'%(player.name))
	return

def callbackremovefriend( char,args , choice):
	item = args[0]
	#~ elif choice.button > 2:
	serial = choice.button
	if serial ==0:
		return
	player = wolfpack.findchar(serial)
	#~ details(char, player)
	removeFriend(item.multi, player)
	char.socket.sysmessage('You have just remove %s from the friends list'%player.name)
	return

def addcoown_target(char, arguments, target):
	item = arguments[0]
	if not target.char.player :
		char.socket.sysmessage('You have to target a char.')
		return
	addCoOwner(item.multi, target.char)
	char.socket.sysmessage('You add %s as new coowner' % (target.char.name))

def addcoown( char , item ):
	if not checkAccess(char, item.multi, 1):
		char.socket.sysmessage('You are not the owner or a co-owners of this house.')
		return
	char.socket.sysmessage('Who do you want to add as coowner ?')
	char.socket.attachtarget("signpost.addcoown_target", [item])

def removeallcoown(char,item):
	if not checkAccess(char, item.multi, ACCESS_OWNER):
		char.socket.sysmessage('You are not the owner of this house.')
		return
	#TODO need to had a confirmation of this action before doing it
	removeAllCoOwner(item.multi)
	char.socket.sysmessage('you have just remove all coowners')

def addfriend_target(char, arguments, target):
	item = arguments[0]
	if not target.char.player :
		char.socket.sysmessage('You have to target a char.')
		return
	addFriend(item.multi, target.char)
	char.socket.sysmessage('You add %s as new friend' % (target.char.name))

def addfriend( char , item ):
	if not checkAccess(char, item.multi, ACCESS_COOWNER):
		char.socket.sysmessage('You are not the owner or a co-owners of this house.')
		return
	char.socket.sysmessage('Who do you want to add as friend ?')
	char.socket.attachtarget("signpost.addfriend_target", [item])
	
def removeallfriend(char,item):
	if not checkAccess(char, item.multi, ACCESS_COOWNER):
		char.socket.sysmessage('You are not the owner or a co-owners of this house.')
		return
	#TODO need to add a confirmation of this action before doing it
	removeAllFriend(item.multi)
	char.socket.sysmessage('you have just remove all friends')
	
def customize( char, item ):
	if not checkAccess(char, item.multi, ACCESS_OWNER):
		char.socket.sysmessage('You are not the owner of this house.')
		return
	
	if not item.hastag( 'house' ):
		return

	multi = wolfpack.findobject( item.gettag( 'house' ) )
	# I Commented the last line... it servers for what exactly? no references for this
	#multi.sendcustomhouse( char )
	char.socket.sysmessage( str( multi.serial ) )
	#char.socket.sysmessage( "Multi serial : %i" % multi.serial )
	char.moveto( wolfpack.coord( multi.pos.x, multi.pos.y, multi.pos.z+7, multi.pos.map ) )
	char.update()
	#woodenpost = wolfpack.finditem( item.morez )
	#woodenpost.delete()
	char.socket.customize( item )
	char.settag( 'customizing', multi.serial )

	return 1

def demolish( char, item ):
	if not checkAccess(char, item.multi, ACCESS_OWNER):
		char.socket.sysmessage('You are not the owner of this house.')
		return
	# If Sign has no Multi, then we have problems here
	if not item.hastag( 'house' ):
		return

	# Assign the Multi
	multi = wolfpack.findobject( item.gettag( 'house' ) )

	# Looking for the list of items in the Multi
	listitems = multi.objects

	# Now... the Loop to remove all items
	contador = 0
	for multiitem in listitems:
		multiitem.delete()
		contador += 1
	
	# Message about how many items are deleted in the house
	char.socket.sysmessage( "Deleted %i items in house!" % contador )

	# Unregistering the House
	housing.unregisterHouse(multi)

	# Erasing Multi
	multi.delete()

def switchgump( char, target, args ):
	item = args[0]
	gumphandler = {
	2: gump0, #information
	17: gump1, #security
	32: gump2, #storage
	47: gump3, #customize
	62: gump4, #ownership
	4: gump10, #list co owner
	34: gump11, #remove a co owner
	64: gump12, #list friend
	94: gump13}  #remove a friend
	actionhandler = {
	7: demolish, 
	21: customize,
	19: addcoown,
	49: removeallcoown,
	79: addfriend,
	109: removeallfriend}

	button = target.button
	if gumphandler.has_key( button ):
		gumphandler[ button ]( char, gumpcallback, item )
	else:
		if actionhandler.has_key( button ):
			actionhandler[ button ]( char, item )
		elif button == 0:
			return
		else:
			char.socket.sysmessage( "Button %i isn't working yet" % button )
			return 1
