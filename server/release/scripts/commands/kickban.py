"""
	\command kick
	\description Kicks a connected client by target or account name.
"""

"""
	\command ban
	\description Kick-bans an account by target or account name.
"""

import wolfpack
from wolfpack.consts import LOG_MESSAGE

def onLoad():
	wolfpack.registercommand( "kick", commandKick )
	wolfpack.registercommand( "ban", commandBan )

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

def accountAction( socket, account, action ):
	if account == None:
		socket.sysmessage( "Failed to find an account with the given name." )
		return False
	elif account and account.acl == 'player':
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
