
"""
	\command guilds
	\description Shows a dialog with all guilds.
"""

import wolfpack
from math import ceil
import time
import guilds.stone

def commandGuilds(socket, command, arguments):
  dialog = wolfpack.gumps.cGump()
  dialog.setCallback("guilds.commands.overview_response")
  dialog.startPage(0)
  dialog.addResizeGump(64, 34, 9260, 464, 462)
  dialog.addGump(30, 33, 10421, 0)
  dialog.addGump(59, 22, 10420, 0)
  dialog.addGump(43, 173, 10422, 0)
  dialog.addResizeGump(100, 79, 9200, 405, 65)
  dialog.addTilePic(106, 49, 3805)
  dialog.addText(160, 90, "Gamemaster Guild Management", 380)
  dialog.addText(160, 113, "Select the guildstone you want to see", 980)
  dialog.addResizeGump(100, 151, 9200, 404, 283)
  dialog.addGump(13, 339, 10402, 0)
  dialog.addButton(102, 450, 247, 248, 1)
  dialog.addButton(177, 450, 242, 241, 0) 

  guilds = wolfpack.guilds()

  dialog.startGroup(1)  

  offset = 0
  first = 1
  page = 0
  pages = ceil(len(guilds) / 3.0)

  for guild in guilds:
    # Start a new page
    if offset == 0:
      page += 1
      dialog.startPage(page)

      if page > 1:
        dialog.addPageButton(112, 403, 9909, 9911, page - 1)
        dialog.addText(138, 405, "Previous Page", 905)

      if page < pages:
        dialog.addPageButton(470, 403, 9903, 9905, page + 1)
        dialog.addText(403, 404, "Next Page", 905)

    name = guild.name
    abbreviation = ''
    if len(guild.abbreviation) > 0:
      abbreviation = '[%s]' % guild.abbreviation
    members = len(guild.members)
    canidates = len(guild.canidates)

    leader = 'None'
    if guild.leader:
      leader = guild.leader.name

    founded = time.strftime("%c", time.localtime(guild.founded))

    dialog.addText(152, 160 + offset, "%s %s" % (name, abbreviation), 2100)
    dialog.addText(152, 180 + offset, "%u Members, %u Canidates" % (members, canidates), 905)
    dialog.addText(152, 200 + offset, "Leader: %s" % leader, 905)
    dialog.addText(300, 200 + offset, "Founded: %s" % founded, 905)
    dialog.addTiledGump(112, 230 + offset, 380, 3, 96, 975)

    if first:
      dialog.addRadioButton(113, 176 + offset, 9721, 9724, guild.serial, 1)
      first = 0
    else:
      dialog.addRadioButton(113, 176 + offset, 9721, 9724, guild.serial, 0)      

    if offset == 160:
      offset = 0
    else:
      offset += 80

  dialog.send(socket)

def overview_response(player, args, response):
  if response.button == 0:
    return

  if len(response.switches) != 1:
    commandGuilds(player.socket, "guilds", "")
  else:
    guild = wolfpack.findguild(response.switches[0])

    if guild:
      guilds.stone.mainMenu(player, guild)

#def addGuild(player, command, arguments):
#  guild = wolfpack.newguild()
  

def onLoad():
  wolfpack.registercommand("guilds", commandGuilds)
#  wolfpack.registercommand("addguild", addGuild)
