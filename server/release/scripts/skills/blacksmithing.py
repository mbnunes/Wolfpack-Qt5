#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: khpae									 #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

import wolfpack
from system.makemenus import CraftItemAction, MakeMenu, findmenu

#
# Bring up the blacksmithing menu
#
def onUse(char, item):
  pass

def testmenu(socket, command, arguments):
  menu = findmenu('BLACKSMITHING')
  if menu:
    menu.send(socket.player)

def onLoad():
  menu = MakeMenu('BLACKSMITHING', None, 'Blacksmithing')
  action = CraftItemAction(menu, 'Test Item', 0, '')
  action.materials.append([['efa'], 5])
  wolfpack.registercommand('testmenu', testmenu)
