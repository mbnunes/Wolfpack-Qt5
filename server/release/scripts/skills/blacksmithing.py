#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: khpae									 #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

import wolfpack
import wolfpack.utilities

def onUse( char, item ):
	# Needs to be on ourself
	if item.getoutmostchar() != char:
		char.socket.clilocmessage( 0x7A258 )
		return 1
	# is it in use already ?
	#if item.gettag( "use" ):
		#char.socket.clilocmessage( )
	#	return 1

	# blacksmithing menu gump
	char.sendmakemenu( "CRAFTMENU_BLACKSMITHING" )

	# set response function

	return 1

