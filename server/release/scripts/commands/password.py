"""
	\command password
	\description Change your current password.
	\usage - <code>password [value]</code>
	Value is the new password for your account. Its length is
	limited to 16 characters.
"""

import wolfpack
import wolfpack.accounts
import string
from wolfpack.consts import LOG_MESSAGE

# Loads the command
def onLoad():
	wolfpack.registercommand( 'password', commandPassword )
	return

# Handles the password command
def commandPassword( socket, cmd, args ):
	char = socket.player
	password = str( args.strip() )
	account = wolfpack.accounts.find( char.account.name )

	if len( password ) == 0:
		socket.sysmessage( "Usage: password <newpassword>" )
		return False
	elif len( password ) > 16:
		socket.sysmessage( "Your password can have a maximum of 16 characters." )
		return False

	account.password = str( password )
	socket.sysmessage( "Your password has been changed." )
	char.log( LOG_MESSAGE, "Account %s changed their password.\n" % char.serial )
	return True
