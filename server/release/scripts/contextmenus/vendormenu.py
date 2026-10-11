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
	elif ( tag == 3 ):
		vname = target.name.lower()
		bod_type = "smith" if ("blacksmith" in vname or "armorer" in vname or "weaponsmith" in vname or "iron" in vname) else "tailor" if ("tailor" in vname or "weaver" in vname or "cloth" in vname) else None
		if bod_type:
			from commands.bod import commandGetBod
			commandGetBod(char.socket, "getbod", bod_type)
		else:
			target.say("I have no bulk orders to offer you.")

	return 1
