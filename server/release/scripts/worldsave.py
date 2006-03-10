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

from wolfpack.gumps import cGump
from wolfpack.consts import EVENT_WORLDSAVE
from wolfpack import tr

######################################################################################
#############   Constants   ##########################################################
######################################################################################

ENABLEDFANCYSTATUS = int( wolfpack.settings.getbool("General", "Fancy Worldsave Status", True, True) )

######################################################################################
#############   Initializing Global Event   ##########################################
######################################################################################

def onLoad():
	wolfpack.registerglobal(EVENT_WORLDSAVE, "worldsave")

######################################################################################
#############   The Event   ##########################################################
######################################################################################

def onWorldSave():
	if ENABLEDFANCYSTATUS:
		# Creating Dialog
		dialog = cGump( nomove=1, nodispose=1, x=150, y=150 )

		dialog.setType( 0x98FA2C10 ) # NEVER change this Type or Gump will not be closed in the end of save

		dialog.addResizeGump( 0, 0, 9200, 291, 90 )
		dialog.addCheckerTrans( 0, 0, 291, 90 )
		dialog.addText( 47, 19, tr( "WORLDSAVE IN PROGRESS" ), 2122 )
		dialog.addText( 47, 37, tr( "Saving %s items." )%( str(wolfpack.itemcount()) ), 2100 )
		dialog.addText( 47, 55, tr( "Saving %s characters." )%( str(wolfpack.charcount()) ), 2100 )
		dialog.addTilePic( 3, 25, 4167 )

		# Sending to players

		worldsocket = wolfpack.sockets.first()
		while worldsocket:
			dialog.send( worldsocket )
			worldsocket = wolfpack.sockets.next()
