
import wolfpack
from wolfpack.utilities import *
from wolfpack.consts import *
from combat.properties import itemcheck, fromitem
from combat.utilities import weaponskill

#
# Show certain modifiers stored in tags.
#
def modifiers(object, tooltip):
  modifiers = {
    "req_str": 1061170,
    "boni_dex": 1060409,
    "boni_int": 1060432,
    "boni_str": 1060485,
    "remaining_uses": 1060584,
  }

  for (tag, cliloc) in modifiers.items():
    if object.hastag(tag):
      tooltip.add(cliloc, str(object.gettag(tag)))

PREFIXES = {
    'dullcopper': 1018332,
    'shadowiron': 1018333,
    'copper': 1018334,
    'bronze': 1018335,
    'gold': 1018336,
    'agapite': 1018337,
    'verite': 1018338,
    'valorite': 1018339,
}

#
# Equipment has a lot of additional effects.
# These are shown to the user in form of tooltips.
#
def onShowTooltip(viewer, object, tooltip):
  # Reinsert the name if we need an ore prefix
  if object.hastag('resname'):    
    resname = str(object.gettag('resname'))
    if PREFIXES.has_key(resname):
      tooltip.reset() # Remove all previous lines
      prefix = PREFIXES[resname]
      if len(object.name) == 0:
        itemname = '#' + str(1020000 + object.id)
      else:
        itemname = object.name

      if object.amount > 1:
        # amount\torename\tname
        tooltip.add(1050045, "%u\t#%u\t%s" % (object.amount, prefix, itemname))
      else:
        # orename\tname
        if object.hastag('exceptional'):
          tooltip.add(1053100, "#%u\t%s" % (prefix, itemname))
        else:
          tooltip.add(1053099, "#%u\t%s" % (prefix, itemname))

  # Exceptional item?
  if object.hastag('exceptional'):
    tooltip.add(1060636, '')
    
    # 1050043: Crafted by ~param~
    serial = int(object.gettag('exceptional'))
    crafter = wolfpack.findchar(serial)
    if crafter:
      tooltip.add(1050043, crafter.name)

  armor = itemcheck(object, ITEM_ARMOR)
  weapon = itemcheck(object, ITEM_WEAPON)  
  shield = itemcheck(object, ITEM_SHIELD)

  # Only Armors and Weapons have durability
  if weapon or armor or shield:
    tooltip.add(1060639, "%u\t%u" % (object.health, object.maxhealth))
  
  # Weapon specific properties
  if weapon:
    # One or twohanded weapon
    if object.twohanded:      
      tooltip.add(1061171, '')
    else:
      tooltip.add(1061824, '')

    # Used weaponskill
    skill = weaponskill(viewer, object)
    if skill == SWORDSMANSHIP:
      tooltip.add(1061172, '')
    elif skill == MACEFIGHTING:
      tooltip.add(1061173, '')
    elif skill == FENCING:
      tooltip.add(1061174, '')
    elif skill == ARCHERY:
      tooltip.add(1061175, '')

    # Special weapon range
    if object.hastag('range'):
      weaponrange = int(object.gettag('range'))
      if weaponrange > 1:
        tooltip.add(1061169, str(weaponrange))

    # Max-Mindamage
    mindamage = fromitem(object, MINDAMAGE)
    maxdamage = fromitem(object, MAXDAMAGE)
    tooltip.add(1061168, "%u\t%u" % (mindamage, maxdamage))    

    # Speed
    speed = fromitem(object, SPEED)
    tooltip.add(1061167, str(speed))

    # Physical Damage Distribution    
    fire = fromitem(object, DAMAGE_FIRE)
    cold = fromitem(object, DAMAGE_COLD)
    poison = fromitem(object, DAMAGE_POISON)
    energy = fromitem(object, DAMAGE_ENERGY)
    physical = 100 - (fire + cold + poison + energy)

    if physical:
      tooltip.add(1060403, str(physical))

    if fire:
      tooltip.add(1060405, str(fire))

    if cold:
      tooltip.add(1060404, str(cold))

    if poison:
      tooltip.add(1060406, str(poison))

    if energy:
      tooltip.add(1060407, str(energy))

  fire = fromitem(object, RESISTANCE_FIRE)
  cold = fromitem(object, RESISTANCE_COLD)
  poison = fromitem(object, RESISTANCE_POISON)
  energy = fromitem(object, RESISTANCE_ENERGY)
  physical = fromitem(object, RESISTANCE_PHYSICAL)

  if physical:
    tooltip.add(1060448, str(physical))

  if fire:
    tooltip.add(1060447, str(fire))

  if cold:
    tooltip.add(1060445, str(cold))

  if poison:
    tooltip.add(1060449, str(poison))

  if energy:
    tooltip.add(1060446, str(energy))

  modifiers(object, tooltip)

#
# Check for certain equipment requirements
#
def onWearItem(player, wearer, item, layer):
  if item.hastag('req_str') and wearer.strength < int(item.gettag('req_str')):
    if player != wearer:
      player.socket.sysmessage('This person can\'t wear that item, seems not strong enough.')
    else:
      player.socket.clilocmessage(500213)
    return 1
      
  if item.hastag('req_dex') and wearer.dexterity < int(item.gettag('req_dex')):
    if player != wearer:
      player.socket.sysmessage('This person can\'t wear that item, seems not agile enough.')
    else:
      player.socket.clilocmessage(502077)
    return 1

  if item.hastag('req_int') and wearer.intelligence < int(item.gettag('req_int')):
    if player != wearer:
      player.socket.sysmessage('This person can\'t wear that item, seems not smart enough.')
    else:
      player.socket.sysmessage('You are not ingellgent enough to equip this item.')
    return 1
      
  return 0

#
# Grant certain stat or skill boni.
#
def onEquip(char, item, layer):
  # Bonus Strength
  if item.hastag('boni_str'):
    char.strength = char.strength + int(item.gettag('boni_str'))

  # Bonus Dex
  if item.hastag('boni_dex'):
    char.dexterity = char.dexterity + int(item.gettag('boni_dex'))

  # Bonus Int
  if item.hastag('boni_int'):
    char.intelligence = char.intelligence + int(item.gettag('boni_int'))

  # Update Stats
  char.updatestats()

#
# Remove certain stat or skill boni.
#
def onUnequip(char, item, layer):
  # Bonus Str
  if item.hastag('boni_str'):
    char.strength = char.strength - int(item.gettag('boni_str'))

  # Bonus Dex
  if item.hastag('boni_dex'):
    char.dexterity = char.dexterity - int(item.gettag('boni_dex'))

  # Bonus Int
  if item.hastag('boni_int'):
    char.intelligence = char.intelligence - int(item.gettag('boni_int'))

  # Update Stats
  char.updatestats()
