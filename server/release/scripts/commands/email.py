"""
	\command email
	\description Change your current account email.
	\usage - <code>email [value]</code>
	Value is the new email for your account. Its length is
	limited to 255 characters.
"""

import wolfpack
import wolfpack.accounts
import string
from wolfpack.consts import LOG_MESSAGE

# Loads the command
def onLoad():
	wolfpack.registercommand( 'email', commandEmail )
	return

# Handles the email command
def commandEmail( socket, cmd, args ):
	char = socket.player
	account = char.account # wolfpack.accounts.find( char.account.name )
	email = str( args.strip() )

	if len( email ) == 0:
		socket.sysmessage( "Usage: email <new email>" )
		return False
	elif len( email ) > 255:
		socket.sysmessage( "Your email can have a maximum of 255 characters." )
		return False

	account.email = str( email )
	socket.sysmessage( "Your email has been changed." )
	char.log( LOG_MESSAGE, "Account %s changed their email.\n" % account.name )
	return True
