
"""
	\command skillinfo
	\description Show a dialog to modify the skill values and caps of a character.
"""

from wolfpack.utilities import tobackpack
import wolfpack
from wolfpack.consts import *
import wolfpack.gumps
from math import ceil, floor
from commands.info import iteminfo

#
# Gump response
#
def response(player, arguments, response):
  if response.button == 0:
    return

  target = wolfpack.findchar(arguments[0])

  if not target:
    return

  # Iterate trough all skills and see what changed
  for skill in range(0, ALLSKILLS):
    try:
      newvalue = int(floor(float(response.text[0x1000 | skill]) * 10))
      newcap = int(floor(float(response.text[0x2000 | skill]) * 10))
    except:
      player.socket.sysmessage('You have entered invalid values for %s.' % SKILLNAMES[skill])
      return

    oldvalue = target.skill[skill]
    oldcap = target.skillcap[skill]

    if oldvalue != newvalue or oldcap != newcap:
      message = "Changed %s for character 0x%x to value %u [%d] and cap %u [%d].\n"
      message = message % (SKILLNAMES[skill], target.serial, newvalue, newvalue - oldvalue, newcap, newcap - oldcap)
      player.log(LOG_MESSAGE, message)
      player.socket.sysmessage(message)

      target.skill[skill] = newvalue
      target.skillcap[skill] = newcap

#
# Show the skillinfo gump
#
def callback(player, arguments, target):
  if not target.char:
    return

  dialog = wolfpack.gumps.cGump()
  dialog.setCallback("commands.skillinfo.response")
  dialog.setArgs([target.char.serial])

  dialog.startPage(0)
  dialog.addResizeGump(35, 12, 9260, 460, 504)
  dialog.addGump(1, 12, 10421, 0)
  dialog.addGump(30, -1, 10420, 0)
  dialog.addResizeGump(66, 40, 9200, 405, 65)
  dialog.addText(108, 52, "Wolfpack Skillinfo Command", 2100)
  dialog.addTiledGump(90, 11, 164, 17, 10250, 0)
  dialog.addGump(474, 12, 10431, 0)
  dialog.addGump(439, -1, 10430, 0)
  dialog.addGump(14, 200, 10422, 0)
  dialog.addGump(468, 200, 10432, 0)
  dialog.addGump(249, 11, 10254, 0)
  dialog.addGump(74, 45, 10464, 0)
  dialog.addGump(435, 45, 10464, 0)
  dialog.addGump(461, 408, 10412, 0)
  dialog.addGump(-15, 408, 10402, 0)
  dialog.addTiledGump(281, 11, 158, 17, 10250, 0)
  dialog.addGump(265, 11, 10252, 0)
  dialog.addButton(60, 476, 247, 248, 1)
  dialog.addButton(136, 476, 242, 241, 0)

  pages = int(ceil(ALLSKILLS / 5.0))

  for page in range(1, pages + 1):
    dialog.startPage(page)

    if page > 1:
      dialog.addPageButton(60, 444, 9909, 9911, page - 1)
      dialog.addText(88, 444, "Previous Page", 2100)

    if page < pages:
      dialog.addPageButton(448, 444, 9903, 9905, page + 1)
      dialog.addText(376, 448, "Next Page", 2100)

    yoffset = 0

    for i in range(0, 5):
      skill = (page - 1) * 5 + i

      if skill >= ALLSKILLS:
        break

      skillname = SKILLNAMES[skill]
      skillname = skillname[0].upper() + skillname[1:]

      dialog.addResizeGump(65, 109 + yoffset, 9200, 405, 62)
      dialog.addText(76, 115 + yoffset, "Skill: %s (%u)" % (skillname, skill), 2100)
      dialog.addResizeGump(123, 135 + yoffset, 9300, 63, 26)
      dialog.addText(76, 137 + yoffset, "Value:", 2100)
      dialog.addText(187, 138 + yoffset, "%", 2100)
      dialog.addInputField(128, 138 + yoffset, 50, 20, 2100, 0x1000 | skill, "%0.01f" % (target.char.skill[skill] / 10.0))
      dialog.addText(232, 138 + yoffset, "Cap:", 2100)
      dialog.addText(329, 139 + yoffset, "%", 2100)
      dialog.addResizeGump(264, 135 + yoffset, 9300, 63, 26)
      dialog.addInputField(268, 139 + yoffset, 53, 20, 2100, 0x2000 | skill, "%0.01f" % (target.char.skillcap[skill] / 10.0))

      yoffset += 65

  dialog.send(player)

#
# Show the skillinfo target
#
def edit(socket, command, arguments):
  socket.sysmessage('Please select a character whose skills you want to view.')
  socket.attachtarget('commands.skillinfo.callback', [])

#
# Skillinfo debugging command
#
def onLoad():
  wolfpack.registercommand('skillinfo', edit)
