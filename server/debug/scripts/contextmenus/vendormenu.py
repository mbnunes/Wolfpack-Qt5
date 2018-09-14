#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: Correa                         #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification:                         #
#################################################################
import speech
def onContextEntry( char, target, tag  ):

	if ( tag == 1 ):
		if target.baseid in [ "barber_male", "barber_female" ]:
			speech.barber.gump( target, char )
		else:
			target.vendorbuy( char )
	elif ( tag == 2 ):
		target.vendorsell( char )

	return 1
