#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: DarkStorm                      #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

from wolfpack import console
from wolfpack.consts import *
import math
import wolfpack
from system.makemenus import CraftItemAction, MakeMenu, findmenu
from wolfpack.utilities import hex2dec
from combat.properties import itemcheck, fromitem
import random

# Metals used by Blacksmithing
# The first four values are requied by the makemenu system.
# The last value is the resname to use for newly crafted items
METALS = [
    ['Iron',        BLACKSMITHING, 0, ['iron_ingot'], 0x0, 'iron'],
    ['Dull Copper', BLACKSMITHING, 650, ['dullcopper_ingot'], 0x973, 'dullcopper'],
    ['Shadow Iron', BLACKSMITHING, 700, ['shadowiron_ingot'], 0x966, 'shadowiron'],
    ['Copper',      BLACKSMITHING, 750, ['copper_ingot'], 0x96d, 'copper'],
    ['Bronze',      BLACKSMITHING, 800, ['bronze_ingot'], 0x972, 'bronze'],
    ['Gold',        BLACKSMITHING, 850, ['gold_ingot'], 0x8a5, 'gold'],
    ['Agapite',     BLACKSMITHING, 900, ['agapite_ingot'], 0x979, 'agapite'],
    ['Verite',      BLACKSMITHING, 950, ['verite_ingot'], 0x89f, 'verite'],
    ['Valorite',    BLACKSMITHING, 990, ['valorite_ingot'], 0x8ab, 'valorite'],
]

#
# A list of scales used by the blacksmithing menu.
#
SCALES = [
  ['Red Scales', 0, 0, ['red_scales'], 0x66D, 'red_scales'],
  ['Yellow Scales', 0, 0, ['yellow_scales'], 0x8A8, 'yellow_scales'],
  ['Black Scales', 0, 0, ['black_scales'], 0x455, 'black_scales'],
  ['Green Scales', 0, 0, ['green_scales'], 0x851, 'green_scales'],
  ['White Scales', 0, 0, ['white_scales'], 0x8FD, 'white_scales'],
  ['Blue Scales', 0, 0, ['blue_scales'], 0x8B0, 'blue_scales'],
]

# This table defines all special properties for items crafted 
# using a given resource. It's based on the resname tag.
ARMOR_PROPERTIES = {
  # Durability Bonus (% of current val), Lower Requirements (% of current val)
  'dullcopper': [1.5],
  'shadowiron': [2.0],
  'gold': [1.0],
  'valorite': [1.5],
}

WEAPON_PROPERTIES = {
  # Durability Bonus (% of current val), Lower Requirements (% of current val)
  'dullcopper': [2.0],
  'shadowiron': [1.5],
  'gold': [1.0],
}

#
# Check for anvil and forge
#
def checkanvilandforge(char):
  # Check dynamic items.
  # We should later check for statics too
  forge = 0
  anvil = 0
  items = wolfpack.items(char.pos.x, char.pos.y, char.pos.map, 5)
  for item in items:
    if item.id == 0xFAF or item.id == 0xFB0:
      anvil = 1
    elif item.id == 0xFB1 or (item.id >= 0x197A and item.id <= 0x19A9):
      forge = 1

    if anvil and forge:
      return 1

  return 0

#
# Check if the character is using the right tool
#
def checktool(char, item, wearout = 0):
  # Has to be in our posession
  if item.getoutmostchar() != char:
    char.socket.clilocmessage(500364)
    return 0

  # We do not allow "invulnerable" tools.
  if not item.hastag('remaining_uses'):
    char.socket.clilocmessage(1044038)
    item.delete()
    return 0
  
  # See if we have another tool equipped
  equipped = char.itemonlayer(LAYER_RIGHTHAND)
  if equipped and equipped != item:
    char.socket.clilocmessage(1048146)
    return 0

  if wearout:
    uses = int(item.gettag('remaining_uses'))
    if uses <= 1:
      char.socket.clilocmessage(1044038)
      item.delete()
      return 0
    else:
      item.settag('remaining_uses', uses - 1)
  
  return 1

#
# Bring up the blacksmithing menu
#
def onUse(char, item):
  if not checktool(char, item):
    return 1

  if not checkanvilandforge(char):
    char.socket.clilocmessage(1044267)
    return 1

  menu = findmenu('BLACKSMITHING')
  if menu:
    menu.send(char, [item.serial])
  return 1

#
# Smith an item.
# Used for scales + ingots
#
class SmithItemAction(CraftItemAction):
  def __init__(self, parent, title, itemid, definition):
    CraftItemAction.__init__(self, parent, title, itemid, definition)
    self.markable = 1 # All blacksmith items are markable

  #
  # Check if we did an exceptional job.
  #
  def getexceptionalchance(self, player, arguments):
    # Only works if this item requires blacksmithing
    if not self.skills.has_key(BLACKSMITHING):
      return 0

    minskill = self.skills[BLACKSMITHING][0]
    maxskill = self.skills[BLACKSMITHING][1]
    chance = ( (player.skill[BLACKSMITHING] - minskill) / (maxskill - minskill) ) / 10.0

    return chance

  #
  # Apply resname and color to the item.
  #
  def applyproperties(self, player, arguments, item, exceptional):
    # See if special ingots were used in the creation of 
    # this item. All items crafted by blacksmiths gain the
    # color!
    if self.submaterial1 > 0:
      material = self.parent.getsubmaterial1used(player, arguments)
      material = self.parent.submaterials1[material]
      item.color = material[4]
      item.settag('resname', material[5])

      # Armor properties
      if itemcheck(item, ITEM_ARMOR):
        if ARMOR_PROPERTIES.has_key(material[5]):
          info = ARMOR_PROPERTIES[material[5]]
          if info[0] > 0:
            item.maxhealth = int(math.ceil(item.maxhealth * info[0]))
            item.health = item.maxhealth
            value = int(info[0] * 100)
            if item.hastag('aos_boni_durability'):
              item.settag('aos_boni_durability', int(item.gettag('aos_boni_durability')) - 100 + value)
            else:
              item.settag('aos_boni_durability', value)

      # Weapon properties
      elif itemcheck(item, ITEM_WEAPON):
        if WEAPON_PROPERTIES.has_key(material[5]):
          info = WEAPON_PROPERTIES[material[5]]
          if info[0] > 0:
            item.maxhealth = math.ceil(item.maxhealth * info[0])
            item.health = item.maxhealth
            value = int(fromitem(item, ) + (info[0] - 1.0) * 100)
            if item.hastag('aos_boni_durability'):
              item.settag('aos_boni_durability', int(item.gettag('aos_boni_durability')) + value)
            else:
              item.settag('aos_boni_durability', value)

    # Apply properties of secondary material
    if self.submaterial2 > 0:
      material = self.parent.getsubmaterial2used(player, arguments)
      material = self.parent.submaterials2[material]
      item.color = material[4]
      item.settag('resname2', material[5])

    # Distribute another 6 points randomly between the resistances this armor already has
    if exceptional:
      if itemcheck(item, ITEM_ARMOR) or itemcheck(item, ITEM_SHIELD):
        # Copy all the values to tags
        boni = [0, 0, 0, 0, 0]

        for i in range(0, 6):
          boni[random.randint(0,4)] += 1

        item.settag('res_physical', fromitem(item, RESISTANCE_PHYSICAL) + boni[0])
        item.settag('res_fire', fromitem(item, RESISTANCE_FIRE) + boni[1])
        item.settag('res_cold', fromitem(item, RESISTANCE_COLD) + boni[2])
        item.settag('res_energy', fromitem(item, RESISTANCE_ENERGY) + boni[3])
        item.settag('res_poison', fromitem(item, RESISTANCE_POISON) + boni[4])
      elif itemcheck(item, ITEM_WEAPON):
        # Increase the damage bonus by 20%
        bonus = fromitem(item, DAMAGEBONUS)
        bonus += 20
        item.settag('aos_boni_damage', bonus)

    # Reduce the uses remain count
    checktool(player, wolfpack.finditem(arguments[0]), 1)

  #
  # First check if we are near an anvil and forge.
  # Then play a blacksmithing sound.
  #
  def make(self, player, arguments):
    # Look for forge and anvil
    if not checkanvilandforge(player):
      player.socket.clilocmessage(1044267)
      return 0

    if not checktool(player, wolfpack.finditem(arguments[0])):
      return 0    

    return CraftItemAction.make(self, player, arguments)

  #
  # Play a simple soundeffect
  #
  def playcrafteffect(self, player, arguments):
    player.soundeffect(0x2a)

#
# A blacksmith menu. The most notable difference is the 
# button for selecting another ore.
#
class BlacksmithingMenu(MakeMenu):
  def __init__(self, id, parent, title):
    MakeMenu.__init__(self, id, parent, title)
    self.sort = 1
    self.allowmark = 1
    self.allowrepair = 1
    self.allowenhance = 1
    self.allowsmelt = 1
    self.submaterials1 = METALS
    self.submaterials2 = SCALES
    self.submaterial2missing = 1060884
    self.submaterial1missing = 1044037
    self.submaterial1noskill = 1044268

  #
  # Get the material used by the character from the tags
  #
  def getsubmaterial1used(self, player, arguments):
    if not player.hastag('blacksmithing_ore'):
      return 0
    else:
      material = int(player.gettag('blacksmithing_ore'))
      if material < len(self.submaterials1):
        return material
      else:
        return 0

  #
  # Get the material used by the character from the tags
  #
  def getsubmaterial2used(self, player, arguments):
    if not player.hastag('blacksmithing_scales'):
      return 0
    else:
      material = int(player.gettag('blacksmithing_scales'))
      if material < len(self.submaterials2):
        return material
      else:
        return 0

  #
  # Save the material preferred by the user in a tag
  #
  def setsubmaterial2used(self, player, arguments):
    player.settag('blacksmithing_scales', material)

  #
  # Save the material preferred by the user in a tag
  #
  def setsubmaterial1used(self, player, arguments, material):
    player.settag('blacksmithing_ore', material)

  #
  # Save the material preferred by the user in a tag
  #
  def setsubmaterial2used(self, player, arguments, material):
    player.settag('blacksmithing_scales', material)

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
  menu = BlacksmithingMenu(id, parent, name)

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
    elif child.name == 'smith':
      if not child.hasattribute('definition') or not child.hasattribute('name'):
        console.log(LOG_ERROR, "Smith action without definition or name in menu %s.\n" % menu.id)
      else:
        itemdef = child.getattribute('definition')
        name = child.getattribute('name')
        try:
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
          action = SmithItemAction(menu, name, int(itemid), itemdef)
        except:
          console.log(LOG_ERROR, "Smith action with invalid item id in menu %s.\n" % menu.id)
        
        # Process subitems        
        for j in range(0, child.childcount):
          subchild = child.getchild(j)
         
          # How much of the primary resource should be consumed
          if subchild.name == 'ingots':
            action.submaterial1 = hex2dec(subchild.getattribute('amount', '0'))            

          # How much of the secondary resource should be consumed
          elif subchild.name == 'scales':
            action.submaterial2 = hex2dec(subchild.getattribute('amount', '0'))

          # Normal Material
          elif subchild.name == 'material':
            if not subchild.hasattribute('id'):
              console.log(LOG_ERROR, "Material element without id list in menu %s.\n" % menu.id)
              break
            else:              
              try:
                ids = subchild.getattribute('id').split(';')
                for i in range(0, len(ids)):
                  ids[i] = hex2dec(ids[i])
              except:
                console.log(LOG_ERROR, "Material element with invalid id list in menu %s.\n" % menu.id)
                break

              try:
                amount = hex2dec(subchild.getattribute('amount', '1'))
              except:
                console.log(LOG_ERROR, "Material element with invalid id list in menu %s.\n" % menu.id)
                break
              action.materials.append([ids, amount])

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
            action.skills[skill] = [minimum, maximum]

#
# Load the blacksmithing menu.
#
def onLoad():
  loadMenu('BLACKSMITHING')
