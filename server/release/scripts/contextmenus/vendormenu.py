#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: Correa                         #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification:                         #
#################################################################

def onContextEntry( char, target, tag  ):

	if ( tag == 1 ):
		target.vendorbuy( char )
	elif ( tag == 2 ):
		target.vendorsell( char )

	return 1
