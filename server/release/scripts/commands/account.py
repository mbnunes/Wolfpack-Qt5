#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts
#  ((    _/{  "-;  | Created by: Dreoth
#   )).-' {{ ;'`   | Revised by:
#  ( (  ;._ \\ ctr | Last Modification: Created
#===============================================================#
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
from wolfpack.utilities import hex2dec
from wolfpack.consts import LOG_MESSAGE

# Loads the command
def onLoad():
	wolfpack.registercommand( 'account', commandAccount )
	return

# Handles the account command
def commandAccount( socket, cmd, args ):
	char = socket.player
	args = args.strip()
	if len(args) == 0:
		usageerror( socket )
		return False
	elif len( args ) > 0:
		# Command with arguments
		args = args.split( ' ' )
		# Error Check
		if len( args ) >= 5:
			usageerror( socket )
			return False
		# Four Arguments
		elif len( args ) == 4:
			( action, username, key, value ) = args
			action = action.lower()
			username = username.lower()
			key = key.lower()
			# Error Checking
			if len( action ) == 0 or len( username ) == 0 or len( key ) == 0 or len( value ) == 0:
				usageerror( socket )
				return False
			# Set Accounts
			elif action.lower() == 'set':
				accountSet( socket, username, key, value )
				return True
			else:
				usageerror( socket )
				return False
		# Three Arguments
		elif len( args ) == 3:
			( action, username, key ) = args
			action = action.lower()
			username = username.lower()
			# Error Checking
			if len( action ) == 0 or len( username ) == 0 or len( key ) == 0:
				usageerror( socket )
				return False
			# Create Accounts
			elif action == 'create':
				accountCreate( socket, username, key )
				return True
			# Show Accounts
			elif action == 'show':
				accountShow( socket, username, key )
				return True
			else:
				usageerror( socket )
				return False
		# Two Arguments
		elif len( args ) == 2:
			( action, username ) = args
			action = action.lower()
			username = username.lower()
			# Error Checking
			if len( action ) == 0 or len( username ) == 0:
				usageerror( socket )
				return False
			# Remove Accounts
			elif action == 'remove':
				accountRemove( socket, username )
				return True
			else:
				usageerror( socket )
				return False
		# One Argument
		elif len( args ) == 1:
			action = args[0]
			action = action.lower()
			# Error Checking
			if len( action ) == 0:
				usageerror( socket )
				return False
			# Reload Accounts
			elif action == 'reload':
				char.log( LOG_MESSAGE, "Reloaded accounts.\n")
				wolfpack.accounts.reload()
				return True
			# Save Accounts
			elif action == 'save':
				char.log( LOG_MESSAGE, "Saved accounts.\n")
				wolfpack.accounts.save()
				return True
			else:
				usageerror( socket )
				return False
		# Error
		else:
			usageerror( socket )
			return False

# Echo usage
def usageerror( socket ):
	socket.sysmessage( "Account Command Usage:" )
	socket.sysmessage( "- account create username password" )
	socket.sysmessage( "- account set username key value" )
	socket.sysmessage( "- account set username key value" )
	socket.sysmessage( "- account show username key" )
	return

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
				oldname = str( account.name )
				account.delete()
				socket.sysmessage( "Success: Account %s removed!" % oldname )
				char.log( LOG_MESSAGE, "Removed account: %s\n" % oldname )
				return True
		# Failure
		else:
			socket.sysmessage( "Error: Account %s does not exist for removal!" % username )
			return False

# Creates a new account
def accountCreate( socket, username, password ):
	account = None
	characcount = None
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
			socket.sysmessage( "Error: Account %s exists!" % account.name )
			return False
		# Create the Account
		elif not account:
		 	newaccount = wolfpack.accounts.add( username, password )
			newaccount.acl = 'player'
			socket.sysmessage( "You created the account successfully!" )
			char.log( LOG_MESSAGE, "Created account: %s\n" % newaccount.name )
			return True
		# Failure
		else:
			socket.sysmessage( "Error: Account creation failed!" )
			return True

# Shows account properties
def accountShow( socket, username, key ):
	account = None
	characcount = None
	char = socket.player
	characcount = wolfpack.accounts.find( char.account.name )
	account = wolfpack.accounts.find( username )
	if not account:
		return False
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
			if key == 'all':
				socket.sysmessage( "Account properties for %s:" % account.name  )
				socket.sysmessage( "  acl: %s" % account.acl )
				socket.sysmessage( "  email: %s" % account.email )
				socket.sysmessage( "  rank: %s" % account.rank )
				socket.sysmessage( "  flags: %s" % account.flags )
				socket.sysmessage( "  inuse: %s" % account.inuse )
				socket.sysmessage( "  lastlogin: %s" % account.lastlogin )
				socket.sysmessage( "  multigems: %s" % account.multigems )
				socket.sysmessage( "  characters: %s" % account.characters )
			elif key == 'acl':
				socket.sysmessage( "%s.acl = %s" % ( account.name, account.acl ) )
				char.log( LOG_MESSAGE, "Requested %s.acl.\n" % account.name )
				return True
			elif key == 'characters':
				socket.sysmessage( "%s.characters = %s" % ( account.name, account.characters ) )
				char.log( LOG_MESSAGE, "Requested %s.characters.\n" % account.name )
				return True
			elif key == 'flags':
				socket.sysmessage( "%s.flags = %s" % ( account.name, account.flags ) )
				char.log( LOG_MESSAGE, "Requested %s.flags.\n" % account.name )
				return True
			elif key == 'inuse':
				socket.sysmessage( "%s.inuse = %s" % ( account.name, account.inuse ) )
				char.log( LOG_MESSAGE, "Requested %s.inuse.\n" % account.name )
				return True
			elif key == 'lastlogin':
				socket.sysmessage( "%s.lastlogin = %s" % ( account.name, account.lastlogin ) )
				char.log( LOG_MESSAGE, "Requested %s.lastlogin.\n" % account.name )
				return True
			elif key == 'multigems':
				socket.sysmessage( "%s.multigems = %s" % ( account.name, account.multigems ) )
				char.log( LOG_MESSAGE, "Requested %s.multigems.\n" % account.name )
				return True
			elif key == 'name':
				socket.sysmessage( "%s.name = %s" % ( account.name, account.name ) )
				char.log( LOG_MESSAGE, "Requested %s.name.\n" % account.name )
				return True
			elif key == 'password' and ( account.name == characcount.name or characcount.rank == 100 ):
				socket.sysmessage( "%s.password = %s" % ( account.name, account.password ) )
				char.log( LOG_MESSAGE, "Requested %s.password.\n" % account.name )
				return True
			elif key == 'rank':
				socket.sysmessage( "%s.rank = %i" % ( account.name, account.rank ) )
				char.log( LOG_MESSAGE, "Requested %s.rank.\n" % account.name )
				return True
			elif key == 'email':
				socket.sysmessage( "%s.email = %i" % ( account.name, account.email ) )
				char.log( LOG_MESSAGE, "Requested %s.email.\n" % account.name )
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
	account = None
	characcount = None
	char = socket.player
	characcount = wolfpack.accounts.find( char.account.name )
	account = wolfpack.accounts.find( username )
	if not account:
		socket.sysmessage( "Error: No such account exists." )
		return False
	if len( value ) == 0:
		socket.sysmessage( "Error: No value was given." )
		return False
	# Usernames are limited to 16 characters in length
	if len( username ) > 16 or len( username ) == 0:
		if len( username ) > 16:
			socket.sysmessage( "Error: Username exceeds the 16 character limit!" )
		if len( username ) == 0:
			socket.sysmessage( "Error: Username is NULL!" )
		return False
	# Find the account
	elif account:
		if account.rank >= characcount.rank and account.name != characcount.name:
			socket.sysmessage( "Error: Your account rank does not permit this!" )
			return False
		else:
			# ACL
			if key == 'acl':
				acl_list = None
				acl_list = wolfpack.accounts.acls()
				if not acl_list or len( acl_list ) == 0:
					socket.sysmessage( "Critical Error: No ACLs are defined!" )
					return False
				if not value in acl_list:
					socket.sysmessage( "Error: %s is not a valid account.acl!" % value )
					return False
				else:
					oldvalue = None
					oldvalue = str( account.acl )
					if not oldvalue:
						socket.sysmessage( "Warning: This account previously had no ACL!" )
					socket.sysmessage( "Previous: %s.acl = %s" % ( account.name, oldvalue ) )
					account.acl = str( value )
					if str( account.acl ) != str( value ):
						socket.sysmessage( "Error: Failure to set new account ACL!" )
						return False
					socket.sysmessage( "Changed: %s.acl = %s" % ( account.name, account.acl ) )
					char.log( LOG_MESSAGE, "Modified %s.acl ( %s :: %s ).\n" % ( account.name, oldvalue, value ) )
					return True
			# Flags
			elif key == 'flags':
				oldvalue = account.flags
				socket.sysmessage( "Previous: %s.flags = %s" % ( account.name, account.flags ) )
				account.flags = hex2dec( value )
				socket.sysmessage( "Changed: %s.flags = %s" % ( account.name, account.flags ) )
				char.log( LOG_MESSAGE, "Modified %s.flags ( %s :: %s ).\n" % ( account.name, oldvalue, value ) )
				return True
			# MultiGems
			elif key == 'multigems':
				if value.lower() == "true" or value.lower() == "false" or value in [ 0, 1 ]:
					oldvalue = account.multigems
					socket.sysmessage( "Previous: %s.multigems = %s" % ( account.name, account.multigems ) )
					account.multigems = value
					socket.sysmessage( "Changed: %s.multigems = %s" % ( account.name, account.multigems ) )
					char.log( LOG_MESSAGE, "Modified %s.multigems ( %s :: %s ).\n" % ( account.name, oldvalue, value ) )
					return True
				else:
					socket.sysmessage( "Error: The account.multigems property must be boolean!" )
					return False
			# Password
			elif key == 'password':
				if len( value ) > 16 or len( value ) == 0:
					if len( value ) > 16:
						socket.sysmessage( "Error: Password exceeds the 16 character limit!" )
					if len( value ) == 0:
						socket.sysmessage( "Error: Password is NULL!" )
					return False
				else:
					oldvalue = account.password
					account.password = str( value )
					socket.sysmessage( "Changed: %s.password" % account.name )
					char.log( LOG_MESSAGE, "Modified %s.password.\n" % account.name )
					return True
			# Email
			elif key == 'email':
				if len( value ) > 255 or len( value ) == 0:
					if len( value ) > 255:
						socket.sysmessage( "Error: Email exceeds the 255 character limit!" )
					if len( value ) == 0:
						socket.sysmessage( "Error: Email is NULL!" )
					return False
				else:
					oldvalue = account.email
					account.email = str( value )
					socket.sysmessage( "Changed: %s.email" % account.name )
					char.log( LOG_MESSAGE, "Modified %s.email.\n" % account.name )
					return True
			# READ ONLY VALUES
			elif key in ['name','lastlogin','inuse','characters','rank']:
				char.log( LOG_MESSAGE, "Attempted modification of read-only value %s.%s.\n" % ( account.name, key ) )
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
