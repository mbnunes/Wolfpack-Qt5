#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: khpae									 #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

# wool
import wolfpack
import wolfpack.utilities
from wolfpack.consts import *

# reagents
ids = [ 0x0f85, 0x0f84, 0x0f86, 0x0f7b, 0x0f7a, 0x0f8d, 0x0f8f, 0x0f88 ]

#def onLoad():
#	wolfpack.registerglobal( HOOK_CHAR, EVENT_SKILLUSE, "skills.alchemy" )

def onUse( char, item ):
	# Needs to be on ourself
	if item.getoutmostchar() != char:
		char.socket.clilocmessage( 0x7A258 )
		return 1
	# is it in use already ?
	#if item.gettag( "use" ):
		#char.socket.clilocmessage( )
	#	return 1

	# alchemy menu gump
	char.sendmakemenu( "CRAFTMENU_ALCHEMY" )

	# set response function

	return 1

