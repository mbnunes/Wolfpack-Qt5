
# TODO: implementation of renaming and unlocking/locking etc.

from wolfpack.consts import *
import wolfpack
import wolfpack.gumps
import random

def onLoad():
  pass

def lock_response(char, args, target):
  if len(args) != 1:
    return    

  key = wolfpack.finditem(args[0])
  if not key or not char.canreach(key,5):
    char.socket.clilocmessage(501661)
    return

  # Check for an item target.
  if not target.item or not char.canreach(target.item,5):
    char.socket.clilocmessage(501666)
    return

  # a) Targetted a lockable item
  if target.item == key:
    char.socket.sysmessage('rename key')
    # Gump...

  elif 'lock' in target.item.events:
    if target.item.hastag('locked') and int(target.item.gettag('locked')) == 1:
      target.item.deltag('locked')
    else:
      target.item.settag('locked','1')
    char.soundeffect(0x241)

  else:
    char.socket.clilocmessage(501666)

  return

def copy_response(char, args, target):
  if len(args) != 1:
    return    

  key = wolfpack.finditem(args[0])
  if not key or not char.canreach(key,5):
    char.socket.clilocmessage(501661)
    return

  # Check if the targetted item is a key
  if not target.item or not 'key' in target.item.events:
    char.socket.clilocmessage(501679)
    return
 
  # Also a blank key?
  if not target.item.hastag('lock'):
    char.socket.clilocmessage(501675)
    return

  # Check if the player can reach the item
  if not char.canreach(target.item,5):
    char.socket.clilocmessage(501661)
    return

  # Tinkering check (15%-30%, 25% chance of loosing the key on failure)
  if char.checkskill(TINKERING, 150, 300):
    key.settag('lock',target.item.gettag('lock'))
    char.socket.clilocmessage(501676)    
  else:
    char.socket.clilocmessage(501677)

    # 25% chance of destroying the blank key
    if random.randint(1,4) == 1:
      char.socket.clilocmessage(501678)
      key.remove()

def onUse(char, key):
  # Does this key open a lock?
  if not key.hastag('lock'):
    # The key is blank so we wan't to make a copy of it
    char.socket.clilocmessage(501663)
    char.socket.attachtarget('key.copy_response',[key.serial])
  else:
    char.socket.clilocmessage(501662)
    char.socket.attachtarget('key.lock_response',[key.serial])

  return 1

def onShowTooltip(char, item, tooltip):
  tooltip.add(0xF9060 + item.id, '')

  # Add the lock id for gms
  if char.gm:
    if item.hastag('lock'):
      lock = str(item.gettag('lock'))
      tooltip.add(1050045, " \t" + "Lock: " + lock + "\t ")
    else:
      tooltip.add(1050045, " \tThis key is blank\t ")

  tooltip.send(char)

  # We still should apply the default handling
  return 1

