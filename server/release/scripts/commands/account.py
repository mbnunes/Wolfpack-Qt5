
"""
	\command account
	\description This command will modify account information.
	\usage - <code>account create username password</code>
	- <code>account remove username</code>
	- <code>account set username key value</code>
	- <code>account show username key</code>
	 Use the create subcommand to create a new account with the given username and password.
	To remove an account along with all player characters on that account, use the remove
	subcommand and pass the username to it.
	To change properties of a given account, use the set subcommand and pass the username,
	the property key and the new property value to it. See the notes for a list of valid property keys.
	To view properties of an account, use the show subcommand and pass the property key to it.
	\notes The following properties can be set for accounts:
	<i>password</i>
	The account password.
	<br />
	<i>acl</i>
	The name of the access control list.
	<br />
	<i>block</i>
	Block status of the account.
	<br />
	In addition to the writeable properties, the following properties can be shown:
	<br />
	<i>loginattempts</i>
	How many failed login attempts have been made since the last successful login.
	<br />
	<i>lastlogin</i>
	When was the last successful login made.
	<br />
	<i>chars</i>
	Prints a list of player characters on this account.
	<br />
	Valid values for the block property are either on, off or for how long the account should be blocked.
	<br />
	If you have enabled MD5 passwords, you can only view the hashed password when showing the password property.
"""

import wolfpack
import wolfpack.accounts
import wolfpack.settings
import string
from wolfpack.consts import *
from wolfpack.utilities import hex2dec

usage0 = "Account Command Usage:"
usage1 = "- account create username password"
usage2 = "- account remove username"
usage3 = "- account set username key value"
usage4 = "- account show username key"

# Loads the command
def onLoad():
	wolfpack.registercommand( 'account', commandAccount )
	return

# Handles the account command
def commandAccount( socket, cmd, args ):
	char = socket.player
	args = args.strip()
	if len(args) == 0:
		socket.sysmessage( usage0 )
		socket.sysmessage( usage1 )
		socket.sysmessage( usage2 )
		socket.sysmessage( usage3 )
		socket.sysmessage( usage4 )
		return False
	elif len( args ) > 0:
		# Command with arguments
		try:
			args = args.split( ' ' )
			# Error Check
			if len( args ) >= 5:
				return False
			# One Argument
			if len( args ) == 1:
				action = args[0]
				action = action.lower()
				# Reload Accounts
				if action == 'reload':
					char.log( LOG_MESSAGE, "0x%x reloaded accounts.\n" % char.serial )
					wolfpack.accounts.reload()
					return True
				# Save Accounts
				elif action == 'save':
					char.log( LOG_MESSAGE, "0x%x saved accounts.\n" % char.serial )
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
				else:
					return False
			# Three Arguments
			elif len( args ) == 3:
				( action, username, key ) = args
				action = action.lower()
				username = username.lower()
				if len( action ) == 0 or len( username ) == 0 or len( key ) == 0:
					return False
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
				if len( action ) == 0 or len( username ) == 0 or len( key ) == 0 or len( value ) == 0:
					return False
				# Set Accounts
				if action.lower() == 'set':
					accountSet( socket, username, key, value )
					return True
				else:
					return False
			# Error
			else:
				return False
		except:
			socket.sysmessage( usage0 )
			socket.sysmessage( usage1 )
			socket.sysmessage( usage2 )
			socket.sysmessage( usage3 )
			socket.sysmessage( usage4 )
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
	account = wolfpack.accounts.find( username )
	# Usernames and passwords are limited to 16 characters in length
	if len( username ) > 16 or len( password ) > 16:
		if len( username ) > 16:
			socket.sysmessage( "Error: Username exceeds the 16 character limit!" )
		if len( password ) > 16:
			socket.sysmessage( "Error: Password exceeds the 16 character limit!" )
		return False
	elif len( username ) == 0 or len( password ) == 0:
		if len( username ) == 0:
			socket.sysmessage( "Error: Username is NULL!" )
		if len( password ) == 0:
			socket.sysmessage( "Error: Password is NULL!" )
		return False
	# Check if the account exists
	else:
		if account:
			socket.sysmessage( "Error: Account %s exists!" % username )
			return False
		# Create the Account
		elif not account:
		 	newaccount = wolfpack.accounts.add( username, password )
			newaccount.acl = 'player'
			socket.sysmessage( "You created the account successfully!" )
			char.log( LOG_MESSAGE, "0x%x created account: %s\n" % ( char.serial, username ) )
			return True
		# Failure
		else:
			socket.sysmessage( "Error: Account creation failed!" )
			return True

# Shows account properties
def accountShow( socket, username, key ):
	char = socket.player
	characcount = wolfpack.accounts.find( char.account.name )
	account = wolfpack.accounts.find( username )
	# Usernames are limited to 16 characters in length
	if len( username ) > 16 or len( username ) == 0:
		if len( username ) > 16:
			socket.sysmessage( "Error: Username exceeds the 16 character limit!" )
		if len( username ) == 0:
			socket.sysmessage( "Error: Username is NULL!" )
		return False
	# Find the account
	else:
		if account:
			# Rank Checking
			if account.rank >= characcount.rank and account.name != characcount.name:
				socket.sysmessage( "Error: Your account rank does not permit this!" )
				return False
			if key == 'acl':
				socket.sysmessage( "%s.acl = %s" % ( account.name, account.acl ) )
				char.log( LOG_MESSAGE, "0x%x requested %s.acl.\n" % ( char.serial, account.name ) )
				return True
			elif key == 'characters':
				socket.sysmessage( "%s.characters = %s" % ( account.name, account.characters ) )
				char.log( LOG_MESSAGE, "0x%x requested %s.characters.\n" % ( char.serial, account.name ) )
				return True
			elif key == 'flags':
				socket.sysmessage( "%s.flags = %s" % ( account.name, account.flags ) )
				char.log( LOG_MESSAGE, "0x%x requested %s.flags.\n" % ( char.serial, account.name ) )
				return True
			elif key == 'inuse':
				socket.sysmessage( "%s.inuse = %s" % ( account.name, account.inuse ) )
				char.log( LOG_MESSAGE, "0x%x requested %s.inuse.\n" % ( char.serial, account.name ) )
				return True
			elif key == 'lastlogin':
				socket.sysmessage( "%s.lastlogin = %s" % ( account.name, account.lastlogin ) )
				char.log( LOG_MESSAGE, "0x%x requested %s.lastlogin.\n" % ( char.serial, account.name ) )
				return True
			elif key == 'multigems':
				socket.sysmessage( "%s.multigems = %s" % ( account.name, account.multigems ) )
				char.log( LOG_MESSAGE, "0x%x requested %s.multigems.\n" % ( char.serial, account.name ) )
				return True
			elif key == 'name':
				socket.sysmessage( "%s.name = %s" % ( account.name, account.name ) )
				char.log( LOG_MESSAGE, "0x%x requested %s.name.\n" % ( char.serial, account.name ) )
				return True
			elif key == 'password' and ( account.name == characcount.name or characcount.rank == 100 ):
				socket.sysmessage( "%s.password = %s" % ( account.name, account.password ) )
				char.log( LOG_MESSAGE, "0x%x requested %s.password.\n" % ( char.serial, account.name ) )
				return True
			elif key == 'rank':
				socket.sysmessage( "%s.rank = %i" % ( account.name, account.rank ) )
				char.log( LOG_MESSAGE, "0x%x requested %s.rank.\n" % ( char.serial, account.name ) )
				return True
			else:
				socket.sysmessage( "Error: Unknown account key!" )
				return True
		# Failure to find the account
		else:
			socket.sysmessage( "Error: Account %s could not be located!" % username )
			return False

# Sets account properties
def accountSet( socket, username, key, value ):
	char = socket.player
	characcount = wolfpack.accounts.find( char.account.name )
	account = wolfpack.accounts.find( username )
	# Usernames are limited to 16 characters in length
	if len( username ) > 16 or len( username ) == 0:
		if len( username ) > 16:
			socket.sysmessage( "Error: Username exceeds the 16 character limit!" )
		if len( username ) == 0:
			socket.sysmessage( "Error: Username is NULL!" )
		return False
	# Find the account
	else:
		if account:
			if account.rank >= characcount.rank and account.name != characcount.name:
				socket.sysmessage( "Error: Your account rank does not permit this!" )
				return False
			else:
				# ACL
				if key == 'acl':
					if value in wolfpack.accounts.acls():
						oldvalue = account.acl
						socket.sysmessage( "Previous: %s.acl = %s" % ( account.name, account.acl ) )
						account.acl == value
						socket.sysmessage( "Changed: %s.acl = %s" % ( account.name, account.acl ) )
						char.log( LOG_MESSAGE, "0x%x modified %s.acl.\n" % ( char.serial, account.name ) )
						char.log( LOG_MESSAGE, "%s.acl = %s :: %s.flags = %s\n" % ( account.name, oldvalue, account.name, value  ) )
						return True
					else:
						socket.sysmessage( "Error: %s is not a valid account.acl!" % value )
						return False
				# Flags
				elif key == 'flags':
					oldvalue = account.flags
					socket.sysmessage( "Previous: %s.flags = %s" % ( account.name, account.flags ) )
					account.flags = hex2dec(value)
					socket.sysmessage( "Changed: %s.acl = %s" % ( account.name, account.flags ) )
					char.log( LOG_MESSAGE, "0x%x modified %s.flags.\n" % ( char.serial, account.name ) )
					char.log( LOG_MESSAGE, "%s.flags = 0x%x :: %s.flags = 0x%x\n" % ( account.name, oldvalue, account.name, value  ) )
					return True
				# MultiGems
				elif key == 'multigems':
					if value.lower() == "true" or value.lower() == "false" or value == 1 or value == 0:
						oldvalue = value
						socket.sysmessage( "Previous: %s.acl = %s" % ( account.name, account.acl ) )
						account.multigems == value
						socket.sysmessage( "Changed: %s.acl = %s" % ( account.name, account.acl ) )
						char.log( LOG_MESSAGE, "0x%x modified %s.multigems.\n" % ( char.serial, account.name ) )
						char.log( LOG_MESSAGE, "%s.multigems = %s :: %s.multigems = %s\n" % ( account.name, oldvalue, account.name, value  ) )
						return True
					else:
						socket.sysmessage( "Error: The account.multigems property must be boolean!" )
						return False
				# Password
				elif key == 'password':
					if len( key ) > 16 or len( key ) == 0:
						if len( key ) > 16:
							socket.sysmessage( "Error: Password exceeds the 16 character limit!" )
						if len( key ) == 0:
							socket.sysmessage( "Error: Password is NULL!" )
						return False
					else:
						oldvalue = key
						account.password = key
						socket.sysmessage( "Changed: %s.password" % ( account.name, account.password ) )
						char.log( LOG_MESSAGE, "0x%x modified %s.password.\n" % ( char.serial, account.name ) )
						return True
				# READ ONLY VALUES
				elif key == 'name' or key == 'lastlogin' or key == 'inuse' or key == 'characters' or key == 'rank':
					char.log( LOG_MESSAGE, "0x%x attempted modification of read only value %s.%s.\n" % ( char.serial, account.name, key ) )
					socket.sysmessage( "Error: The account.%s property is read only!" % key )
					return False
				# Unknown
				else:
					socket.sysmessage( "Error: Unknown account property given!" )
					return False
		# Failure to find the account
		else:
			socket.sysmessage( "Error: Account %s could not be located!" % username )
			return True
