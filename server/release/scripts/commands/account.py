
import wolfpack
import wolfpack.accounts
import string
from wolfpack.consts import *

usage1 = "Usage: account [save,reload]"
usage2 = "Usage: account [create,remove,set,show] username [(key,pass), value]"

# Loads the command
def onLoad():
	wolfpack.registercommand( 'account', commandAccount )

# Handles the account command
def commandAccount( socket, cmd, args ):
	args = args.strip()
	if len(args) == 0:
		socket.sysmessage( usage1 )
		socket.sysmessage( usage2 )
		return True
	elif len( args ) > 0:
		try:
			args = args.split( ' ' )
			if len( args ) == 1:
				( action ) = args
				if action.lower() == 'reload':
					wolfpack.accounts.reload()
					return True
				elif action.lower() == 'save':
					wolfpack.accounts.save()
					return True
			elif len( args ) == 2:
				( action, username ) = args
				if action.lower() == 'remove':
					accountRemove( socket, username )
					return True
			elif len( args ) == 3:
				( action, username, key ) = args
				if action.lower() == 'create':
					if len( key ) == 0:
						socket.sysmessage( "Error: Empty password!" )
						return True
					else:
						accountCreate( socket, username.lower(), key )
						return True
				elif action.lower() == 'show':
					accountShow( socket, username.lower(), key )
					return True
			elif len( args ) == 4:
				( action, username, key, value ) = args
				if action.lower() == 'set':
					accountSet( socket, username.lower(), key.lower(), value )
					return True
			else:
				socket.sysmessage( usage1 )
				socket.sysmessage( usage2 )
				return True
		except:
			socket.sysmessage( usage1 )
			socket.sysmessage( usage2 )
			return True

# Removes an account
def accountRemove( socket, username ):
	# Make sure username stays lowercased
	username = username.lower()
	# Usernames are limited to 16 characters in length
	if len( username ) > 16:
		socket.sysmessage( "The given username exceeds the 16 character limit!" )
		return True
	# Check if the account exists/Delete
	else:
		account = wolfpack.accounts.find( username )
		if account:
			account.delete()
			socket.sysmessage( "Removed account: %s" % username )
			return True
		# Failure
		else:
			socket.sysmessage( "The given account name could not be found for removal!" )
			return True

# Creates a new account
def accountCreate( socket, username, password ):
	username = username.lower()
	# Usernames and passwords are limited to 16 characters in length
	if len( username ) > 16 or len( password ) > 16:
		if len( username ) > 16:
			socket.sysmessage( "The given username exceeds the 16 character limit!" )
		if len( password ) > 16:
			socket.sysmessage( "The given password exceeds the 16 character limit!" )
		return True
	# Check if the account exists
	else:
		account = wolfpack.accounts.find( username )
		if account:
			socket.sysmessage( "An account with this name already exists!" )
			return True
		# Create the Account
		elif not account:
		 	wolfpack.accounts.add( username, password )
			socket.sysmessage( "You created the account successfully!" )
			return True
		# Failure
		else:
			socket.sysmessage( "Account creation failed!" )
			return True

# Shows account properties
def accountShow( socket, username, key ):
	username = username.lower()
	key = key.lower()
	# Usernames are limited to 16 characters in length
	if len( username ) > 16:
		socket.sysmessage( "The given username exceeds the 16 character limit!" )
		return True
	# Find the account
	else:
		account = wolfpack.accounts.find( username )
		if account:
			if key == 'acl':
				socket.sysmessage( "%s.acl = %s" % ( account.name, account.acl ) )
				return True
			elif key == 'characters':
				socket.sysmessage( "%s.characters = %s" % ( account.name, account.characters ) )
				return True
			elif key == 'flags':
				socket.sysmessage( "%s.flags = %s" % ( account.name, account.flags ) )
				return True
			elif key == 'inuse':
				socket.sysmessage( "%s.inuse = %s" % ( account.name, account.inuse ) )
				return True
			elif key == 'lastlogin':
				socket.sysmessage( "%s.lastlogin = %s" % ( account.name, account.lastlogin ) )
				return True
			elif key == 'multigems':
				socket.sysmessage( "%s.multigems = %s" % ( account.name, account.multigems ) )
				return True
			elif key == 'name':
				socket.sysmessage( "%s.name = %s" % ( account.name, account.name ) )
				return True
			elif key == 'password':
				socket.sysmessage( "%s.password = %s" % ( account.name, account.password ) )
				return True
			elif key == 'rank':
				socket.sysmessage( "%s.rank = %i" % ( account.name, account.rank ) )
				return True
			else:
				socket.sysmessage( "Unknown account key!" )
				return True
		# Failure to find the account
		else:
			socket.sysmessage( "Failed to locate the given account!" )
			return True

# Sets account properties
def accountSet( socket, username, key, value ):
	username = username.lower()
	# Usernames are limited to 16 characters in length
	if len( username ) > 16:
		socket.sysmessage( "The given username exceeds the 16 character limit!" )
		return True
	# Find the account
	else:
		account = wolfpack.accounts.find( username )
		if account:
			return True
		# Failure to find the account
		else:
			socket.sysmessage( "Failed to locate the given account!" )
			return True
