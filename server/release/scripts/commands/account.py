
import wolfpack
import wolfpack.accounts
import string
from wolfpack.consts import *

usage1 = "Usage: account [save,reload]"
usage2 = "Usage: account [create,remove,set,show] username [(key,pass), value]"

# Loads the command
def onLoad():
	wolfpack.registercommand( 'account', commandAccount )
	return

# Handles the account command
def commandAccount( socket, cmd, args ):
	char = socket.player
	args = args.strip()
	# Command with arguments
	if len( args ) > 0:
		args = args.split( ' ' )
		# Error Check
		if len( args ) >= 5:
			return False
		# One Argument
		if len( args ) == 1:
			( action ) = args
			action = action.lower()
			# Reload Accounts
			if action == 'reload':
				char.log( LOG_MESSAGE, "0x%x reloaded accounts." % char.serial )
				wolfpack.accounts.reload()
				return True
			# Save Accounts
			elif action == 'save':
				char.log( LOG_MESSAGE, "0x%x saved accounts." % char.serial )
				wolfpack.accounts.save()
				return True
			else:
				return False
		# Two Arguments
		elif len( args ) == 2:
			( action, username ) = args
			action = action.lower()
			username = username.lower()
			# Remove Accounts
			if action == 'remove':
				accountRemove( socket, username )
				return True
		# Three Arguments
		elif len( args ) == 3:
			( action, username, key ) = args
			action = action.lower()
			username = username.lower()
			# Create Accounts
			if action == 'create':
				accountCreate( socket, username, key )
				return True
			# Show Accounts
			elif action == 'show':
				accountShow( socket, username, key )
				return True
			else:
				return False
		# Four Arguments
		elif len( args ) == 4:
			( action, username, key, value ) = args
			action = action.lower()
			username = username.lower()
			key = key.lower()
			# Set Accounts
			if action.lower() == 'set':
				accountSet( socket, username, key, value )
				return True
			else:
				return False
		# Error
		else:
			socket.sysmessage( usage1 )
			socket.sysmessage( usage2 )
			return False

# Removes an account
def accountRemove( socket, username ):
	char = socket.player
	characcount = wolfpack.accounts.find( char.account.name )
	# Usernames are limited to 16 characters in length
	if len( username ) > 16 or len( username ) == 0:
		if len( username ) > 16:
			socket.sysmessage( "Error: Username exceeds the 16 character limit!" )
		elif len( username ) == 0:
			socket.sysmessage( "Error: Username is NULL!" )
		return False
	# Check if the account exists/Delete
	else:
		account = wolfpack.accounts.find( username )
		if account:
			# Rank Protection
			if account.rank >= characcount.rank:
				socket.sysmessage( "Error: Your account rank does not permit this!" )
				return False
			else:
				account.delete()
				socket.sysmessage( "Success: Account %s removed!" % username )
				char.log( LOG_MESSAGE, "0x%x removed account: %s\n" % ( char.serial, username ) )
				return True
		# Failure
		else:
			socket.sysmessage( "Error: Account %s does not exist for removal!" % username )
			return False

# Creates a new account
def accountCreate( socket, username, password ):
	char = socket.player
	characcount = wolfpack.accounts.find( char.account.name )
	# Usernames and passwords are limited to 16 characters in length
	if len( username ) > 16 or len( password ) > 16:
		if len( username ) > 16:
			socket.sysmessage( "Error: Username exceeds the 16 character limit!" )
		if len( password ) > 16:
			socket.sysmessage( "Error: Password exceeds the 16 character limit!" )
		return True
	elif len( username ) == 0 or len( password ) == 0:
		if len( username ) == 0:
			socket.sysmessage( "Error: Username is NULL!" )
		if len( password ) == 0:
			socket.sysmessage( "Error: Password is NULL!" )
		return True
	# Check if the account exists
	else:
		account = wolfpack.accounts.find( username )
		if account:
			socket.sysmessage( "An account with this name already exists!" )
			return False
		# Create the Account
		elif not account:
		 	wolfpack.accounts.add( username, password )
			socket.sysmessage( "You created the account successfully!" )
			char.log( LOG_MESSAGE, "0x%x created account: %s\n" % ( char.serial, username ) )
			return True
		# Failure
		else:
			socket.sysmessage( "Account creation failed!" )
			return True

# Shows account properties
def accountShow( socket, username, key ):
	char = socket.player
	characcount = wolfpack.accounts.find( char.account )
	key = key.lower()
	# Usernames are limited to 16 characters in length
	if len( username ) > 16:
		socket.sysmessage( "The given username exceeds the 16 character limit!" )
		return True
	# Find the account
	else:
		account = wolfpack.accounts.find( username )
		if account:
			#
			if account.rank >= characcount.rank:
				return False
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
