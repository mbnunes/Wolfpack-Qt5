#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: codex                          #
#   )).-' {{ ;'`   # Revised by: Correa                         #
#  ( (  ;._ \\ ctr # Last Modification: check for dead/criminal #
#################################################################


def onContextEntry( char, target, tag  ):
	if( tag == 1 ):
		if ( char.isdead() ):
			return 1
		if ( char.iscriminal() ):
			target.say( 500378 ) # Thou art a criminal and cannot access thy bank box.
			return 1
	bank = char.getbankbox()
	if bank:
		target.say( "Here is your bank box, %s." % char.name,5 )
		char.socket.sendcontainer( bank )
	elif( tag == 2):
		bank = char.getbankbox()
	if bank:
		amount = bank.countresource( 0xEED, 0x0 )
		if not amount:
		target.say( "Alas you don't have any money in your bank.",5 )
		else:
		target.say( "You have %i gold in your bank." % amount,5 )
	
	return 1

