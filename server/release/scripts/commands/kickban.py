"""
	\command kick
	\description Kicks a connected client by target or account name.
"""
"""
	\command ban
	\description Kick-bans an account by target or account name.
"""
"""
	\command unban
	\description Unbans an account with the given account name.

"""

import wolfpack
import wolfpack.accounts

def onLoad():
	wolfpack.registercommand( "kick", commandKick )
	wolfpack.registercommand( "ban", commandBan )
	wolfpack.registercommand( "unban", commandUnban )

def commandKick( socket, cmd, args ):
	if len( args ) > 0:
		accname = args.lower()
		account = wolfpack.accounts.find( accname )
		accountAction( socket, account, 'kick' )
		return
	else:
		socket.sysmessage( "Please select the character to kick." )
		socket.attachtarget( "commands.remove.doKick", [] )
	return True

def commandBan( socket, cmd, args ):
	if len( args ) > 0:
		accname = args.lower()
		account = wolfpack.accounts.find( accname )
		accountAction( socket, account, 'ban' )
		return
	else:
		socket.sysmessage( "Please select the character to kick-ban." )
		socket.attachtarget( "commands.remove.doBan", [] )
	return False

def doKick( char, args, target ):
	if target.char and target.char.player:
		accountAction( char.socket, target.char.account, 'kick' )
	return

def doBan( char, args, target ):
	if target.char and target.char.player:
		accountAction( char.socket, target.char.account, 'ban' )
	return

def commandUnban( socket, cmd, args ):
	player = socket.player
	playeraccount = player.account
	myrank = playeraccount.rank
	if len( args ) > 0:
		accname = args.lower()
		account = wolfpack.accounts.find( accname )
		if account and account.rank <= myrank:
			if account.rank <= 1 or not playeraccount.authorized( 'Misc', 'May Block Staff Accounts' ):
				socket.sysmessage( "You're not authorized to unban staff members." )
				return False
			account.unblock()
			socket.sysmessage( "You've unban the account %s." % account.name )
			return True
	else:
		socket.sysmessage( "You need to specify an account name to unban." )
		return False

def accountAction( socket, account, action ):
	player = socket.player
	playeraccount = player.account
	myrank = playeraccount.rank
	if account == None:
		socket.sysmessage( "Failed to find an account with the given name." )
		return False
	elif account and account.rank <= myrank:
		if account.rank <= 1 or not playeraccount.authorized( 'Misc', 'May Block Staff Accounts' ):
			socket.sysmessage( "You're not authorized to kick or ban staff members." )
			return False

		if action == 'kick':
			for char in account.characters:
				if char.socket:
					char.socket.sysmessage( "You've been kicked from the shard by the staff." )
					char.socket.disconnect()
					socket.sysmessage( "You have disconnected %s." % char.name )
			return True
		elif action == 'ban':
			account.block()
			for char in account.characters:
				if char.socket:
					char.socket.sysmessage( "You've been kick-banned from the shard by the staff." )
					char.socket.disconnect()
					socket.sysmessage( "You have kick-banned %s." % char.name )
			return True
	return False
