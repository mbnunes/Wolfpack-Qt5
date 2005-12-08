#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: MagnusBr                       #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

####################################################################################
###################     Imports      ###############################################
####################################################################################

import wolfpack

######################################################################################
#############   Tool Tip   ###########################################################
######################################################################################

def onShowTooltip(viewer, object, tooltip):

	tooltip.add(1060584, str(object.gettag('remaining_uses')))

######################################################################################
#############   Context Menu   #######################################################
######################################################################################

def onContextCheckEnabled(player, object, tag):

	ruses = object.gettag('remaining_uses')

	if ruses > 0:
		return True
	else:
		return False

def onContextEntry(player, object, entry):
	if entry == 1:
		player.socket.sysmessage( "Select a Shuriken to load on Ninja Belt" )
		player.socket.attachtarget( "shuriken.targetload", [object.serial] )
	elif entry == 2:
		fillshurikens( player, object )

######################################################################################
#############   Fill Shurikens   #####################################################
######################################################################################

def fillshurikens( player, object ):
	
	backpack = player.getbackpack()
	ruses = object.gettag('remaining_uses')

	for i in range(1, ruses):
		shuriken = wolfpack.additem('27ac')
		backpack.additem( shuriken )

	object.settag('remaining_uses', 0)
	object.resendtooltip()

######################################################################################
#############   Target for Load   ####################################################
######################################################################################

def targetload( char, args, target ):

	object = wolfpack.finditem( args[0] )
	
	if not target:
		return False
	
	if target.item.id == 0x27ac:
		if target.item.container == char.getbackpack():
			ruses = object.gettag('remaining_uses')
			object.settag('remaining_uses', ruses + 1)
			target.item.delete()
			object.resendtooltip()
		else:
			char.socket.sysmessage( "Shuriken have to be in your backpack" )
	else:
		char.socket.clilocmessage( 1063301 )