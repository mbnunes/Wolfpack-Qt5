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

# Handles the password command
def commandEmail( socket, cmd, args ):
	account = None
	password = None
	char = socket.player
	args = args.strip()
	account = wolfpack.accounts.find( char.account.name )
	email = str( args )
	if len( email ) == 0 or len( email ) > 255:
		if len( email ) > 0:
			socket.sysmessage( "Usage: email <newemail>" )
		elif len( password ) > 255:
			socket.sysmessage( "Your email can have a maximum of 255 characters." )
		return False
	else:
		account.email = str( email )
		socket.sysmessage( "Your email has been changed." )
		char.log( LOG_MESSAGE, "Account %s changed their email.\n" % account.name )
		return True
