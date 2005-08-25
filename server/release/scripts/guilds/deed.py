
import wolfpack
from wolpack import tr

def response(player, args, target):
	if player.guild:
		player.socket.sysmessage(tr('You have to leave your guild before founding a new one.'))
		return

	deed = wolfpack.finditem(args[0])

	if deed:
		if deed.getoutmostchar() != player:
			player.socket.clilocmessage(500364)
		elif not player.cansee(deed) or not player.canpickup(deed):
			player.socket.clilocmessage(500312)
		#elif not player.cansee(target.pos, 1) or not target.pos.validspawnspot():
		#	player.socket.clilocmessage(500312)
		else:
			if not player.canreach(target, 3):
				player.socket.clilocmessage(500312)
			elif not target.pos.validspawnspot():
				player.socket.clilocmessage(502749)
			else:
				# Create a guild
				guild = wolfpack.newguild()
				guild.name = tr("%s's Guild") % (player.name)
				guild.addmember(player)
				guild.leader = player

				player.addscript( 'guilds.member' )

				# Create a guildstone at the given location
				guildstone = wolfpack.additem("guildstone")
				guildstone.settag('guild', guild.serial)
				guildstone.decay = 0
				guildstone.moveto(target.pos)
				guild.guildstone = guildstone
				guildstone.update()

				deed.delete()
				player.socket.sysmessage(tr('You place the guildstone and your deed vanishes.'))
				player.resendtooltip()
	return True

# Allow the player to place a guildstone
def onUse(player, deed):
	if player.guild:
		player.socket.sysmessage(tr('You have to leave your current guild before you found a new one.'))
		return True

	if deed.getoutmostchar() != player:
		player.socket.clilocmessage(500364)
	elif not player.cansee(deed) or not player.canpickup(deed):
		player.socket.clilocmessage(500312)
	else:
		# The guildstone is placed where the character is standing
		player.socket.sysmessage(tr('Where do you want to place the guildstone?'))
		player.socket.attachtarget('guilds.deed.response', [deed.serial])

	return True
