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
from wolfpack.consts import LOG_MESSAGE

def onLoad():
	wolfpack.registercommand( "kick", commandKick )
	wolfpack.registercommand( "ban", commandBan )
	wolfpack.registercommand( "unban", commandUnban )

def commandKick( socket, cmd, args ):
	if len( args ) > 0:
		accname = args.lower()
		account = None
		account = wolfpack.accounts.find( accname )
		accountAction( socket, account, 'kick' )
		return
	else:
		socket.sysmessage( "Please select the object for removal." )
		socket.attachtarget( "commands.remove.doKick", [] )
	return True

def commandBan( socket, cmd, args ):
	if len( args ) > 0:
		accname = args.lower()
		account = None
		account = wolfpack.accounts.find( accname )
		accountAction( socket, account, 'ban' )
		return
	else:
		socket.sysmessage( "Please select the object for removal." )
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
		account = None
		account = wolfpack.accounts.find( accname )
		if account:
			if account.rank > 1:
				if playeraccount.authorized( 'Misc', 'May Block Staff Accounts' ):
					continue
				else:
					socket.sysmessage( "You're not authorized to unblock staff members." )
					return False
			account.unblock()
			socket.sysmessage( "You've unblocked the account %s." % account.name )
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
	elif account and myrank > account.rank:
		if account.rank == 100:
			socket.sysmessage( "You can not kick or ban admins!" )
			return False
		if account.rank > 1:
			if playeraccount.authorized( 'Misc', 'May Block Staff Accounts' ):
				continue
			else:
				socket.sysmessage( "You're not authorized to kick or ban staff members." )
				return False

		if action == 'kick':
			account.block()
			for char in account.characters:
				if char.socket:
					char.soscket.sysmessage( "You've been kicked from the shard by the staff." )
					char.socket.disconnect()
					socket.sysmessage( "You have disconnected %s." % char.name )
			return True
		elif action == 'ban':
			account.block()
			for char in account.characters:
				if char.socket:
					char.soscket.sysmessage( "You've been kick-banned from the shard by the staff." )
					char.socket.disconnect()
					socket.sysmessage( "You have kick-banned %s." % char.name )
			return True
		return False
