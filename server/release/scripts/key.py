
# TODO: implementation of renaming and unlocking/locking etc.

import wolfpack
import wolfpack.gumps

def onLoad():
  pass

def onUse(char, key):
  char.message('A shiny key!')
  return 1

def onShowTooltip(char, item, tooltip):
  tooltip.add(0xF9060 + item.id, '')

  # Add the lock id for gms
  if char.gm:
    lock = 'None'

    if item.hastag('lock'):
      lock = str(item.gettag('lock'))

    tooltip.add(1050045, " \t" + "Lock: " + lock + "\t ")

  tooltip.send(char)

  # We still should apply the default handling
  return 1

