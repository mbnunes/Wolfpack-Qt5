
import wolfpack
from wolfpack.gumps import WarningGump

def response(player, args, target):
  if player.guild:
    player.socket.sysmessage('You have to leave your guild before founding a new one.')
    return

  deed = wolfpack.finditem(args[0])

  if deed:
    if deed.getoutmostchar() != player:
      player.socket.clilocmessage(500364)
    elif not player.cansee(deed) or not player.canpickup(deed):
      player.socket.clilocmessage(500312)
    #elif not player.cansee(target.pos, 1) or not target.pos.validspawnspot():
    #  player.socket.clilocmessage(500312)
    else:
      if not player.cansee(target.pos, 1):
        player.socket.clilocmessage(500312)
      elif not target.pos.validspawnspot():
        player.socket.clilocmessage(502749)
      elif target.pos.distance(player.pos) > 3:
        player.socket.clilocmessage(500251)
      else:
        # Create a guild
        guild = wolfpack.newguild()
        guild.name = "%s's Guild" % (player.name)
        guild.addmember(player)
        guild.leader = player
        events = player.events
        if 'guilds.member' not in events:
          player.events = events + ['guilds.member']

        # Create a guildstone at the given location
        guildstone = wolfpack.additem("guildstone")
        guildstone.settag('guild', guild.serial)
        guildstone.decay = 0
        guildstone.moveto(target.pos)
        guild.guildstone = guildstone
        guildstone.update()

        deed.delete()
        player.socket.sysmessage('You place the guildstone and your deed vanishes.')

# Allow the player to place a guildstone
def onUse(player, deed):
  if player.guild:
    player.socket.sysmessage('You have to leave your current guild before you found a new one.')
    return 1

  if deed.getoutmostchar() != player:
    player.socket.clilocmessage(500364)
  elif not player.cansee(deed) or not player.canpickup(deed):
    player.socket.clilocmessage(500312)
  else:
    # The guildstone is placed where the character is standing
    player.socket.sysmessage('Where do you want to place the guildstone?')
    player.socket.attachtarget('guilds.deed.response', [deed.serial])

  return 1
