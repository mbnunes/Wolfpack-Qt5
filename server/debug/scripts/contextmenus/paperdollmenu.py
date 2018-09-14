#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: Correa                         #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification:                         #
#################################################################

def onContextEntry( char, target, tag  ):
	if( tag == 1 ):
		if ( char.socket ):
			char.socket.sendpaperdoll( target )
	return 1
