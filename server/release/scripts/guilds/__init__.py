
import time

# Check if the player already has a private message
# from the specified character waiting and return it.
def getPrivateMessage(recipient, sender):
  tagname = 'privmsg' + str(sender.serial)
  
  if not recipient.hastag(tagname):
    return None

  chunks = recipient.gettag(tagname).split(';', 2)

  return [ float(chunks[0]), int(chunks[1]), chunks[2] ]

# Save a private message for delivery on next login
# If there already was a private message, overwrite it
# If the new message is None, delete the old
def setPrivateMessage(recipient, sender, message):
  tagname = 'privmsg' + str(sender.serial)

  if recipient.hastag(tagname):
    recipient.deltag(tagname)

  if message:
    recipient.settag(tagname, '%f;%u;%s' % (time.time(), sender.saycolor, message))

# Send a guild message to the given player
# If the player is not logged in the message
# is queued up to be displayed on next login.
def sendMessage(player, message, color=0x3b2):
  if player and player.socket:
    # Send the message directly
    player.socket.sysmessage(message)
  elif player and not player.socket:
    # Queue the message
    messagecount = 0
    if player.hastag('guildmessages'):
      messagecount = int(player.gettag('guildmessages'))
    name = 'guildmessage' + str(messagecount)
    
    # Save the information
    player.settag(name, message)
    player.settag('guildmessages', messagecount+1)

PERMISSION_PROPERTIES = 0x01
PERMISSION_RECRUIT = 0x02
PERMISSION_PROMOTE = 0x04
PERMISSION_DIPLOMACY = 0x08
PERMISSION_KICK = 0x10
PERMISSION_EXPEL = 0x20
PERMISSION_TITLES = 0x40
PERMISSION_GRANT = 0x80

# Checks if a given player has the permission
# for certain things related to his guild.
def checkPermission(player, guild, permission, ignoregm = 0):
  if not ignoregm and player.gm:
    return 1

  if not player in guild.members:
    return 0

  if player == guild.leader:
    return 1

  permissions = 0
  tagname = 'permissions_%u' % guild.serial
  if player.hastag(tagname):
    permissions = int(player.gettag(tagname))

  return (permissions & permission) != 0

# Get the current permissions for this guild member
def getPermissions(player, guild):
  if not player in guild.members:
    return 0
  else:
    tagname = 'permissions_%u' % guild.serial
    permissions = 0
    if player.hastag(tagname):
      permissions = int(player.gettag(tagname))
    return permissions

# Sets new permissions for this guild member
def setPermissions(player, guild, permissions):
  if player in guild.members:
    tagname = 'permissions_%u' % guild.serial
    
    if player.hastag(tagname) and permissions == 0:
      player.deltag(tagname)
    elif permissions != 0:
      player.settag(tagname, permissions)

# Gets the serial this character is voting for
# in the current leader elections
def getVote(player, guild):
  if player in guild.members:
    tagname = 'vote_%u' % guild.serial
    
    if player.hastag(tagname):
      return int(player.gettag(tagname))

  return 0

# Gets the serial this character is voting for
# in the current leader elections
def setVote(player, guild, vote):
  tagname = 'vote_%u' % guild.serial

  if not vote:
    player.deltag(tagname)
  elif player in guild.members:
    player.settag(tagname, vote)
