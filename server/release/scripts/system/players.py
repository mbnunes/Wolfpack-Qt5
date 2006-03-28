
import wolfpack
from wolfpack import tr
import guilds.stone
from commands.jail import jailPlayer
import wolfpack.settings
from quests.players import openquestplayer
from wolfpack.utilities import staffmessage
#from wolfpack.utilities import isyoung

#enable_young = int( wolfpack.settings.getbool("General", "Enable Young Status", True, True) )

#def onCreate(object, id):
#	if enable_young:
#		object.addscript("system.young")

def onLogin( player ):
	socket = player.socket
	socket.sysmessage( tr("Welcome to %s") % ( wolfpack.serverversion() )  )
	socket.sysmessage( tr("Report Bugs: http://bugs.wpdev.org/") )
	player.hidden = False

	# send to jail if account is jailed
	if player.account.flags & 0x80:
		if not player.jailed:
			jailPlayer( player, player )
	return False

def onConnect( player, reconnecting ):
	socket = player.socket
	if reconnecting:
		socket.sysmessage( tr("Reconnecting.") )
	player.update()
	socket.resendplayer()

	# send to jail if account is jailed
	if player.account.flags & 0x80:
		if not player.jailed:
			jailPlayer( player, player )

def onLogout( player ):
	player.removefromview()
	player.hidden = 1
	player.update()

def onDamage(char, type, amount, source):
#	if isyoung(char):
#		return 0

	socket = char.socket

	if source and source.player:
		slip_amount = 18
	else:
		slip_amount = 25

	if socket and amount > slip_amount and socket.hastag('bandage_slipped'):
		socket.settag('bandage_slipped', int(socket.gettag('bandage_slipped')) + 1)
		socket.clilocmessage(500961) # Your fingers slip!

	return amount

def onGuildButton(player):
	if not player.guild:
		player.socket.sysmessage(tr('You aren\'t in a guild.'))
	else:
		guilds.stone.mainMenu(player, player.guild)

def onQuestButton(player):
	openquestplayer(player)

def onTalk(player, type, color, font, text, lang):
	wolfpack.utilities.staffmessage(player, text, 0)
