
import wolfpack
from wolfpack.consts import *
from wolfpack import console
from system.makemenus import MakeItemAction, MakeMenu, MakeAction, findmenu

#
# Target response for adding a npc
#
def addnpc(player, arguments, target):
  if target.item and target.item.container:
    player.socket.sysmessage("You can't add the npc there.")
    return

  npc = wolfpack.addnpc(str(arguments[0]), target.pos)
  npc.update()

#
# Target response for adding an item
#
def additem(player, arguments, target):
  item = wolfpack.additem(str(arguments[0]))
  if target.item:
    if target.item.type == 1:
      target.item.additem(item, 1, 1, 0)
    elif target.item.container:
      target.item.container.additem(item, 1, 1, 0)
    else:
      item.moveto(target.item.pos)
  elif target.char:
    item.moveto(target.char.pos)
  else:
    item.moveto(target.pos)

  item.update()

#
# Add an item or character or
# show the addmenu.
#
def add(socket, command, arguments):
  if len(arguments) > 0:
    if wolfpack.getdefinition(WPDT_ITEM, arguments):
      socket.sysmessage("Where do you want to place the item '%s'?" % arguments)
      socket.attachtarget("commands.add.additem", [arguments])
    elif wolfpack.getdefinition(WPDT_NPC, arguments):
      socket.sysmessage("Where do you want to spawn the npc '%s'?" % arguments)
      socket.attachtarget("commands.add.addnpc", [arguments])
    else:
      socket.sysmessage('No Item or NPC definition by that name found.')
    return

  menu = findmenu('ADDMENU')
  if menu:
    menu.send(socket.player)
  else:
    socket.sysmessage('No ADDMENU menu found.')

#
# This action creates a npc.
#
class AddNpcAction(MakeAction):
  def __init__(self, parent, title, definition):
    MakeAction.__init__(self, parent, title)
    self.definition = definition
    self.description = ''
    self.other = ''
    self.materials = ''
    self.skills = ''

  def make(self, player, arguments):
    player.socket.sysmessage("Where do you want to spawn the npc '%s'?" % self.definition)
    player.socket.attachtarget("commands.add.addnpc", [self.definition])
    MakeAction.make(self, player, arguments)

#
# This action creates an item
#
class AddItemAction(MakeItemAction):
  def __init__(self, parent, title, itemid, definition):
    MakeItemAction.__init__(self, parent, title, itemid, definition)

  def make(self, player, arguments):
    player.socket.sysmessage("Where do you want to place the item '%s'?" % self.definition)
    player.socket.attachtarget("commands.add.additem", [self.definition])
    MakeAction.make(self, player, arguments)

#
# Generate a menu structure out of the
# category tags of our item definitions.
#
def generateAddMenu():
  items = wolfpack.getdefinitions(WPDT_ITEM)

  addmenu= MakeMenu('ADDMENU', None, 'Add Menu')
  submenus = {}

  for item in items:
    if not item.hasattribute('id'):
      continue

    child = item.findchild('category')
    if not child:
      continue

    categories = ['Items'] + child.text.split('\\')
    description = categories[len(categories)-1] # Name of the action
    categories = categories[:len(categories)-1]

    # Iterate trough the categories and see if they're all there
    category = ''
    if len(categories) > 0 and not submenus.has_key('\\'.join(categories) + '\\'):
      for subcategory in categories:
        if not submenus.has_key(category + subcategory + '\\'):
          # Category is our parent category
          parent = None
          if len(category) == 0:
            parent = addmenu
          elif submenus.has_key(category):
            parent = submenus[category]

          category += subcategory + '\\'
          menu = MakeMenu('ADDMENU_' + category, parent, subcategory)
          submenus[category] = menu
        else:
          category += subcategory + '\\'

    child = item.findchild('id')
    if child:
      try:
        id = int(child.value)
      except:
        id = 0
    else:
      id = 0
    definition = item.getattribute('id')

    # Parse the position of this makemenu entry
    if len(categories) == 0:
      additem = AddItemAction(addmenu, description, id, definition)
    else:
      additem = AddItemAction(submenus['\\'.join(categories) + '\\'], description, id, definition)
    additem.description = 'Definition: ' + definition

  for (id, menu) in submenus.items():
    menu.sort()

  npcs = wolfpack.getdefinitions(WPDT_NPC)
  submenus = {}

  for npc in npcs:
    if not npc.hasattribute('id'):
      continue

    child = npc.findchild('category')
    if not child:
      continue

    categories = ['NPCs'] + child.text.split('\\')
    description = categories[len(categories)-1] # Name of the action
    categories = categories[:len(categories)-1]

    # Iterate trough the categories and see if they're all there
    category = ''
    if len(categories) > 0 and not submenus.has_key('\\'.join(categories) + '\\'):
      for subcategory in categories:
        if not submenus.has_key(category + subcategory + '\\'):
          # Category is our parent category
          parent = None
          if len(category) == 0:
            parent = addmenu
          elif submenus.has_key(category):
            parent = submenus[category]

          category += subcategory + '\\'
          menu = MakeMenu('ADDMENU_' + category, parent, subcategory)
          submenus[category] = menu
        else:
          category += subcategory + '\\'

    definition = npc.getattribute('id')

    # Parse the position of this makemenu entry
    if len(categories) == 0:
      addnpc = AddNpcAction(addmenu, description, definition)
    else:
      addnpc = AddNpcAction(submenus['\\'.join(categories) + '\\'], description, definition)
    addnpc.description = 'Definition: ' + definition
  
  for (id, menu) in submenus.items():
    menu.sort()

  addmenu.sort()

def onLoad():
  generateAddMenu()
  wolfpack.registercommand('add', add)

"""
	\command add
	\description Add a npc or item and if no definition was specified, open a menu.
	\usage - <code>add npc-id</code>	
	- <code>add item-id</code>
	- <code>add</code>
	If neither a npc nor an item id is passed to the add command, a menu with all
	categorized item and npc definitions is shown.
"""
