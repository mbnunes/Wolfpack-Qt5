#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: khpae									 #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

from wolfpack import console
from wolfpack.consts import *
import math
import wolfpack
from system.makemenus import CraftItemAction, MakeMenu, findmenu
from wolfpack.utilities import hex2dec, tobackpack
from wolfpack.properties import itemcheck, fromitem
import random

#
# Display fletching craftmenu.
#
def onUse(player, item):
  if not checktool(player, item):
    return True

  menu = findmenu('BOWCRAFT')
  if menu:
    menu.send(player, [item.serial])
  return True

#
# Check if the character is using the right tool
#
def checktool(char, item, wearout = 0):
  if not item:
    return False

  # Has to be in our posession
  if item.getoutmostchar() != char:
    char.socket.clilocmessage(500364)
    return False

  # We do not allow "invulnerable" tools.
  if not item.hastag('remaining_uses'):
    char.socket.clilocmessage(1044038)
    item.delete()
    return False

  if wearout:
    uses = int(item.gettag('remaining_uses'))
    if uses <= 1:
      char.socket.clilocmessage(1044038)
      item.delete()
      return False
    else:
      item.settag('remaining_uses', uses - 1)

  return True

#
# Craft an item.
#
class FletchItemAction(CraftItemAction):
  def __init__(self, parent, title, itemid, definition):
    CraftItemAction.__init__(self, parent, title, itemid, definition)
    self.markable = 1
    self.stackable = 0

  #
  # Check if we did an exceptional job.
  #
  def getexceptionalchance(self, player, arguments):
    if not self.skills.has_key(BOWCRAFT):
      return False

    minskill = self.skills[BOWCRAFT][0]
    maxskill = self.skills[BOWCRAFT][1]
    penalty = self.skills[BOWCRAFT][2]
    if not penalty:
      penalty = 0

    minskill += penalty
    maxskill += penalty
    chance = ( player.skill[BOWCRAFT] - (minskill *0.65) ) / 10
    #chance = ( (player.skill[BOWCRAFT] - minskill) / (maxskill - minskill) ) / 10.0

    if chance > 100:
      chance = 100
    elif chance < 0:
      chance = 0

    # chance range 0.00 - 1.00
    chance = chance * .01
    return chance
  
  #
  # Don't consume any material if the item is marked as stackable.
  #  
  def consumematerial(self, player, arguments, half = 0):
  	if self.stackable:
  		return True
  	else:
  		return CraftItemAction.consumematerial(player, arguments, half)

  #
  # For bowcraft we simply consume the maximum material available.
  #
  def applyproperties(self, player, arguments, item, exceptional):
    # Use all available resources if the item we make is
    # flagged as "stackable".
    if self.stackable:
      backpack = player.getbackpack()
      count = -1
      for (materials, amount, name) in self.materials:
        items = backpack.countitems(materials)
        if count == -1:
          count = items / amount
        else:
          count = min(count, items / amount)
      for (materials, amount, name) in self.materials:
        backpack.removeitems( materials, count )
      if count != -1:
        item.amount += count
      else:
        item.amount = 1 + count
      item.update()


    # Distribute another 6 points randomly between the resistances an armor alread
    # has. There are no tailored weapons.
    if exceptional:
      pass

    # Reduce the uses remain count
    checktool(player, wolfpack.finditem(arguments[0]), 1)

  #
  # Check for the used tool.
  #
  def make(self, player, arguments, nodelay=0):
    assert(len(arguments) > 0, 'Arguments has to contain a tool reference.')

    if not checktool(player, wolfpack.finditem(arguments[0])):
      return False

    return CraftItemAction.make(self, player, arguments, nodelay)

  #
  # Play a simple soundeffect
  #
  def playcrafteffect(self, player, arguments):
    player.soundeffect(0x55)

#
# The bowcraft menu.
#
class BowcraftMenu(MakeMenu):
  def __init__(self, id, parent, title):
    MakeMenu.__init__(self, id, parent, title)
    self.allowmark = 1
    self.allowrepair = 1
    self.gumptype = 0x4f1ba412 # This should be unique

  #
  # Repair an item
  #
  def repair(self, player, arguments, target):
    if not checktool(player, wolfpack.finditem(arguments[0])):
      return

    if not target.item:
      player.socket.clilocmessage(500426)
      return

    if target.item.container != player.getbackpack():
      player.socket.clilocmessage(1044275)
      return

    item = target.item
    weapon = itemcheck(item, ITEM_WEAPON)

    if weapon:
      # Item in full repair
      if item.maxhealth <= 0 or item.health >= item.maxhealth:
        player.socket.clilocmessage(500423)
        return

      skill = player.skill[BOWCRAFT]
      if skill >= 900:
        weaken = 1
      elif skill >= 700:
        weaken = 2
      else:
        weaken = 3

      action = self.findcraftitem(item.baseid)

      # We can't craft it, so we can't repair it.
      if not action:
        player.socket.clilocmessage(1044277)
        return

      # We will either destroy or repair it from here on
      # So we can play the craft effect.
      player.soundeffect(0x55)

      if item.maxhealth <= weaken:
        player.socket.clilocmessage(500424)
        item.delete()
      elif player.checkskill(TAILORING, 0, 1000):
        player.socket.clilocmessage(1044279)
        item.maxhealth -= weaken
        item.health = item.maxhealth
        item.resendtooltip()
      else:
        player.socket.clilocmessage(1044280)
        item.maxhealth -= weaken
        item.health = max(0, item.health - weaken)
        item.resendtooltip()

      # Warn the user if we'll break the item next time
      if item.maxhealth <= weaken:
        player.socket.clilocmessage(1044278)

      return

    player.socket.clilocmessage(1044277)

#
# Load a menu with a given id and
# append it to the parents submenus.
#
def loadMenu(id, parent = None):
  definition = wolfpack.getdefinition(WPDT_MENU, id)
  if not definition:
    if parent:
      console.log(LOG_ERROR, "Unknown submenu %s in menu %s.\n" % (id, parent.id))
    else:
      console.log(LOG_ERROR, "Unknown menu: %s.\n" % id)
    return

  name = definition.getattribute('name', '')
  menu = BowcraftMenu(id, parent, name)

  # See if we have any submenus
  for i in range(0, definition.childcount):
    child = definition.getchild(i)
    # Submenu
    if child.name == 'menu':
      if not child.hasattribute('id'):
        console.log(LOG_ERROR, "Submenu with missing id attribute in menu %s.\n" % menu.id)
      else:
        loadMenu(child.getattribute('id'), menu)

    # Craft an item
    elif child.name == 'fletch':
      if not child.hasattribute('definition') or not child.hasattribute('name'):
        console.log(LOG_ERROR, "Fletch action without definition or name in menu %s.\n" % menu.id)
      else:
        itemdef = child.getattribute('definition')
        name = child.getattribute('name')

        # See if we can find an item id if it's not given
        if not child.hasattribute('itemid'):
          item = wolfpack.getdefinition(WPDT_ITEM, itemdef)
          itemid = 0
          if item:
            itemchild = item.findchild('id')
            if itemchild:
              itemid = itemchild.value
        else:
          itemid = hex2dec(child.getattribute('itemid', '0'))
        action = FletchItemAction(menu, name, int(itemid), itemdef)

        # Process subitems
        for j in range(0, child.childcount):
          subchild = child.getchild(j)

          # Normal Material
          if subchild.name == 'material':
            if not subchild.hasattribute('id'):
              console.log(LOG_ERROR, "Material element without id list in menu %s.\n" % menu.id)
              break
            else:
              ids = subchild.getattribute('id').split(';')
              try:
                amount = hex2dec(subchild.getattribute('amount', '1'))
                materialname = subchild.getattribute('name', 'Unknown')
              except:
                console.log(LOG_ERROR, "Material element with invalid id list in menu %s.\n" % menu.id)
                break
              action.materials.append([ids, amount, materialname])

          # Consume all available materials scaled by the
          # amount of each submaterial
          elif subchild.name == 'stackable':
            action.stackable = 1

          # This item will never be exceptional
          elif subchild.name == 'nomark':
            action.markable = 0

          # Skill requirement
          elif subchild.name in skillnamesids:
            skill = skillnamesids[subchild.name]
            try:
              minimum = hex2dec(subchild.getattribute('min', '0'))
            except:
              console.log(LOG_ERROR, "%s element with invalid min value in menu %s.\n" % (subchild.name, menu.id))

            try:
              maximum = hex2dec(subchild.getattribute('max', '1200'))
            except:
              console.log(LOG_ERROR, "%s element with invalid max value in menu %s.\n" % (subchild.name, menu.id))

            try:
              penalty = hex2dec(subchild.getattribute('penalty', '0'))
            except:
              console.log(LOG_ERROR, "%s element with invalid penalty value in menu %s.\n" % (subchild.name, menu.id))

            action.skills[skill] = [minimum, maximum, penalty]

  # Sort the menu. This is important for the makehistory to make.
  menu.sort()

#
# Load the blacksmithing menu.
#
def onLoad():
  loadMenu('BOWCRAFT')
