#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: DarkStorm                      #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

import wolfpack

def onUse( char, item ):
	if( not char.socket ):
		return 0
	
	if( char.distanceto( item ) > 5 ):
		char.socket.sysmessage( "You are too far away to use this" )
		return 1

	if( char.checkskill( wolfpack.ITEMID, 0, 100 ) ):
		# Not animated
		if( item.id == 0x1230 ):
			item.soundeffect( 0x56 )
			item.id = 0x1245
		elif( item.id == 0x1245 ):
			item.id = 0x1230
		else:
			char.socket.sysmessage( "Wrong item id" )
			return 1
		
		item.update()		

	return 1