def onContextEntry( char, target, tag  ):
    if( tag == 1 ):
	bank = char.getbankbox()
	if bank:
	    target.say( "Here is your bank box, %s." % char.name )
	    char.socket.sendcontainer( bank )
    elif( tag == 2):
	bank = char.getbankbox()
	if bank:
	    amount = bank.countresource( 0xEED, 0x0 )
	    if not amount:
		target.say( "Alas you don't have any money on your bank." )
	    else:
		target.say( "You have %i gold on your bank." % amount )
	
    return 1

