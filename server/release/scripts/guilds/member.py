
from wolfpack.consts import *
import wolfpack
from guilds import *
from time import *

# Show queued messages to the guildmember
def onLogin(player):
  if player.hastag('guildmessages'):
    count = int(player.gettag('guildmessages'))

    for i in range(0, count):
      name = 'guildmessage' + str(i)
      text = unicode(player.gettag(name))
      player.socket.sysmessage(text, 0x3A)
      player.deltag(name)
    player.deltag('guildmessages')

  # Make sure that this event is removed as soon as we don't are in a guild anymore
  if not player.guild:
    # Clean voting and or permissions
    tags = player.tags
    for tag in tags:
      if tag.startswith('vote_'):
        player.deltag(tag)
      elif tag.startswith('permissions_'):
        player.deltag(tag)

    events = player.events
    while 'guilds.member' in events:
      events.remove('guilds.member')
    player.events = events
  else:
    # Remove voting tags just in case
    if player.guild.leader:
      player.deltag('vote_%u' % player.guild.serial)

    members = player.guild.members
    online = []

    for member in members:
      if member != player and member.socket:
        online.append(member)

    if len(online) > 1:
      player.socket.sysmessage('There are %u other members of your guild logged on.' % len(online), 0x30)
    elif len(online) == 1:
      player.socket.sysmessage('There is one other member of your guild logged on.', 0x30)
    else:
      player.socket.sysmessage('There is no other member of your guild logged on.', 0x30)

    # Private messaging
    tags = player.tags

    for tag in tags:
      if tag.startswith('privmsg'):
        serial = int(tag[7:])
        char = wolfpack.findchar(serial)
        if char:
          chunks = getPrivateMessage(player, char)
          time_now = localtime()
          time_message = localtime(chunks[0])

          # Hide the date if it's from the same day
          if time_now[0] != time_message[0] or time_now[1] != time_message[1] or time_now[2] != time_message[2]:
            player.socket.sysmessage('[%s] %s: %s' % (strftime(FORMAT_DATETIME, time_message), char.name, chunks[2]), chunks[1])
          else:
            player.socket.sysmessage('[%s] %s: %s' % (strftime(FORMAT_TIME, time_message), char.name, chunks[2]), chunks[1])

        player.deltag(tag)

# Notify all guild members that this member has been
# removed from the guild because he has been deleted
def onDelete(player):
  guild = player.guild
  message = "'%s' has left the game because the character was deleted." % player.name

  # Don't notify anyone if he wasn't in a guild
  if guild:
    members = guild.members
    for member in members:
      if member != player:
        sendMessage(member, message)
