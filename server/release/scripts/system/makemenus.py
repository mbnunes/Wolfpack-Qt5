
import wolfpack
from wolfpack.utilities import tobackpack
from wolfpack.gumps import cGump
import math
from wolfpack import console
from wolfpack.consts import *

# Known menus
menus = {}

# Return a menu
def findmenu(id):
  global menus
  if not menus.has_key(id):
    return None
  else:
    return menus[id]

# Constants
whitehtml = '<basefont color="#FFFFFF">%s'
centerhtml = '<basefont color="#FFFFFF"><div align="center">%s</div>'

#
# Response action for a MakeAction Details gump.
#
def MakeActionResponse(player, arguments, response):
  (menu, action) = arguments[0].split(':')
  action = int(action)
  global menus
  if menus.has_key(menu):
    menu = menus[menu]
    if action < len(menu.subactions):
      menu.subactions[action].response(player, response, list(arguments)[1:])

#
# This class encapsulates all the functionality related to 
# an item you can select in one of the MakeMenu categories.
#
class MakeAction:
  #
  # Constructor for an "action" inside of a submenu.
  # This is a baseclass. Override the make() and
  # details() methods.
  #
  def __init__(self, parent, title):
    assert(parent)
    self.parent = parent
    self.title = title
    self.parent.subactions.append(self)
    self.hasdetails = 0

  #
  # Default response function (does nothing).
  #
  def response(self, player, response, arguments):
    pass

  #
  # Executes this action.
  # Arguments are the arguments passed down from the makemenu.
  #
  def make(self, player, arguments):
    # Append this action to the players "last10" history
    history = []
    identifier = '%s:%u' % (self.parent.id, self.parent.subactions.index(self))
    if player.hastag('makehistory'):
      history = str(player.gettag('makehistory')).split(';')
      history.remove(identifier)

    history = [identifier] + history[:9]
    player.settag('makehistory', ';'.join(history))

    player.socket.sysmessage('Executed a makeaction.')

  #
  # Shows a detail page for this action.
  # This is optional. Arguments are the arguments
  # passed down from the makemenu instance.
  #
  def details(self, player, arguments):
    pass

#
# This action creates an item and shows a 
# detail page for it.
#
class MakeItemAction(MakeAction):
  #
  # Creates a MakeItemAction instance.
  #  
  def __init__(self, parent, title, itemid, definition, amount = 1):
    MakeAction.__init__(self, parent, title)
    self.itemid = itemid
    self.definition = definition
    self.amount = amount
    self.description = ''
    self.other = ''
    self.materials = ''
    self.skills = ''
    self.hasdetails = 1

  #
  # Create the item in the players backpack.
  #
  def make(self, player, arguments):
    item = wolfpack.additem(self.definition)
    if not item:
      console.log(LOG_ERROR, "Unknown item definition used in action %u of menu %s.\n" % \
        (self.parent.subactions.index(self), self.parent.id))
    else:
      if self.amount > 0:
        item.amount = self.amount
      if not tobackpack(item, player):
        item.update()
      player.socket.sysmessage('You put the new item into your backpack.')
      MakeAction.make(self, player, arguments)

  #
  # Process a response from the details gump.
  #
  def response(self, player, response, arguments):
    if response.button == 0:
      self.parent.send(player, arguments)
    elif response.button == 1:
      self.make(player, arguments)

  #
  # Shows a detail page for this action.
  #
  def details(self, player, arguments):
    gump = cGump()
    gump.setType(self.parent.gumptype)
    gump.setArgs(['%s:%u' % (self.parent.id, self.parent.subactions.index(self))] + arguments)
    gump.setCallback("system.makemenus.MakeActionResponse")

    gump.addResizeGump(0, 0, 5054, 530, 417)
    gump.addTiledGump(10, 10, 510, 22, 2624)
    gump.addTiledGump(165,	37, 355, 88, 2624)
    gump.addTiledGump(165, 130, 355, 80, 2624)
    gump.addTiledGump(165, 215, 355, 80, 2624)
    gump.addTiledGump(165, 300, 355, 80, 2624)
    gump.addTiledGump(10, 385, 510, 22, 2624)
    gump.addTiledGump(10,  37, 150, 88, 2624)
    gump.addTiledGump(10, 130, 150, 22, 2624)
    gump.addTiledGump(10, 215, 150, 22, 2624)
    gump.addTiledGump(10, 300, 150, 22, 2624)
    gump.addCheckerTrans(10, 10, 510, 397)


    gump.addHtmlGump(10, 12, 510, 20, centerhtml % self.title)
    if self.itemid != 0:
      gump.addTilePic(15, 42, self.itemid)
    gump.addHtmlGump(10, 132, 150, 20, centerhtml % "SKILLS")
    gump.addHtmlGump(10, 217, 150, 20, centerhtml % "MATERIALS")
    gump.addHtmlGump(10, 302, 150, 20, centerhtml % "OTHER")
    gump.addHtmlGump(170, 39, 70, 20, whitehtml % "ITEM")
    gump.addButton(15, 387, 0xFAE, 0xFB0, 0) # Back to the parent menu of this node
    gump.addHtmlGump(50, 389, 80, 18, whitehtml % "BACK")
    gump.addButton(375, 387, 4005, 4007, 1) # Make the item
    gump.addHtmlGump(410, 389, 95, 18, whitehtml % "MAKE NOW")

    # Item Name 
    gump.addHtmlGump(245, 39, 270, 20, whitehtml % self.title)

    # Scrollable Skill List
    gump.addHtmlGump(170, 132, 345, 76, whitehtml % self.skills, 0, self.skills.count('<br>') > 4)

    # Scrollable Material List
    gump.addHtmlGump(170, 217, 345, 76, whitehtml % self.materials, 0, self.materials.count('<br>') > 4)

    # Description
    gump.addHtmlGump(170, 302, 345, 76, whitehtml % self.description, 0, 1)
    
    gump.send(player)

#
# Internal function for sorting a list of 
# objects with a title property.
#
def comparetitle(a, b):
  return cmp(a.title, b.title)

#
# MakeMenu Gump Response Handler
#
def MakeMenuResponse(player, arguments, response):
  assert(len(arguments) == 1)
  menu = arguments[0]
  arguments = list(arguments)[1:]

  global menus
  if not menus.has_key(menu):
    raise RuntimeError, "Unknown makemenu: %s" % menu
  else:
    menus[menu].response(player, response, arguments)

#
# This class encapsulates all functionality for a single
# category in the makemenus.
#
class MakeMenu:
  #
  # Self registering.
  #
  def __init__(self, id, parent, title):
    assert(not ';' in id)
    assert(not ':' in id)
    global menus
    menus[id] = self
    self.id = id
    self.parent = parent    
    self.submenus = []
    self.subactions = []
    self.title = title
    self.notices = ''
    self.gump = None
    self.gumptype = 0
    self.sort = 0

    # Ssee if the parent has the correct type
    if self.parent and not isinstance(parent, MakeMenu):
      raise TypeError, "MakeMenu %s has an invalid parent of type %s." % (self.id, type(parent).__name__)

    if self.parent:
      self.parent.submenus.append(self)

    # See if all submenus have the correct type
    for menu in self.submenus:
      if not isinstance(menu, MakeMenu):
        raise TypeError, "MakeMenu %s has an invalid submenu of type %s." % (self.id, type(menu).__name__)

    # See if all subactions have the correct type
    for action in self.subactions:
      if not isinstance(menu, MakeAction):
        raise TypeError, "MakeMenu %s has an invalid subaction of type %s." % (self.id, type(action).__name__)

  #
  # Display a makehistory for the user. 
  # No pagebuttons, no subactions. Just the last 10
  # actions this user executed.
  #
  def makehistory(self, player, arguments):
    gump = cGump()
    gump.callback = "system.makemenus.MakeMenuResponse"
    gump.setArgs([self.id] + arguments)
    gump.setType(self.gumptype)
    gump.startPage(0)
    gump.addBackground(0x13be, 530, 417)
    gump.addTiledGump(10, 10, 510, 22, 0xA40)
    gump.addTiledGump(10,292, 150, 45, 0xA40)
    gump.addTiledGump(165, 292, 355, 45, 0xA40)
    gump.addTiledGump(10, 342, 510, 65, 0xA40)
    gump.addTiledGump(10, 37, 200, 250, 0xA40)
    gump.addTiledGump(215, 37, 305, 250, 0xA40)
    gump.addCheckerTrans(10, 10, 510, 397)
    
    gump.addHtmlGump(10, 12, 510, 20, centerhtml % self.title)
    gump.addHtmlGump(10, 39, 200, 20, centerhtml % "CATEGORIES")
    gump.addHtmlGump(215, 39, 305, 20, centerhtml % "SELECTIONS")
    gump.addHtmlGump(10, 294, 150, 20, centerhtml % "NOTICES")
    gump.addHtmlGump(170, 292, 345, 41, whitehtml % self.notices)
    
    # EXIT button , return value: 0
    gump.addButton(15, 382, 0xFB1, 0xFB3, 0)
    gump.addHtmlGump(50, 385, 150, 18, whitehtml % "EXIT")
   
    # MAKE LAST button , return value: 2
    gump.addButton(270, 382, 4005, 4007, 2)
    gump.addHtmlGump(305, 385, 150, 18, whitehtml % "MAKE LAST")

    # PREVIOUS MENU button, return value: 3
    if self.parent:
      gump.addButton(15, 362, 0xFAE, 0xFB0, 3)
      gump.addHtmlGump(50, 365, 150, 18, whitehtml % "PREVIOUS MENU")

    # MARK ITEM switch , switch index: 1
    if player.hastag('markitem'):
      gump.addCheckbox(270, 362, 4005, 4007, 1, 1)
    else:
      gump.addCheckbox(270, 362, 4005, 4007, 1, 0)
    gump.addHtmlGump(305, 365, 150, 18, whitehtml % "MARK ITEM")

    # Calculate how many pages are required
    pages = math.ceil(len(self.submenus) / 9.0)
    actions = 0

    for i in range(0, pages):
      gump.startPage(i+1)
      
      # Fill the page with submenus
      for j in range(0, 9):
        if actions + j < len(self.submenus):
          yoffset = 80 + 20 * j
          gump.addButton(15, yoffset, 4005, 4007, 0x80000000 | (actions + j))
          gump.addHtmlGump(50, yoffset + 3, 150, 18, whitehtml % self.submenus[actions + j].title)
      actions += 9

      # We know that we only have 10 makeactions if
      # we show the history. So we don't need a huge iteration 
      # here.
      if i == 0 and player.hastag('makehistory'):
        history = str(player.gettag('makehistory')).split(';')[:10]
        yoffset = 60
        j = 0
        for item in history:
          (menu, action) = item.split(':')
          action = int(action)
          global menus
          if menus.has_key(menu):
            menu = menus[menu]
            if action < len(menu.subactions):
              gump.addButton(220, yoffset, 4005, 4007, 0x10000000 | j)
              if menu.subactions[action].hasdetails:
                gump.addButton(480, yoffset, 4011, 4012, 0x08000000 | j)
              gump.addHtmlGump(255, yoffset + 3, 220, 18, whitehtml % menu.subactions[action].title)
              yoffset += 20
          j += 1 # Always increase to keep in sync              
    
    gump.send(player)

  #
  # Helper function for making the last action
  # the player executed. Returns true if the
  # action is actually executed.
  #
  def makelast(self, player, arguments):
    if player.hastag('makehistory'):
      history = str(player.gettag('makehistory')).split(';')
      if len(history) > 0:
        (menu, action) = history[0].split(":")
        global menus
        if menus.has_key(menu):
          menu = menus[menu]
          action = int(action)
          if action < len(menu.subactions):
            menu.subactions[action].make(player, arguments)
            return 1
        else:
          raise TypeError, menu

    return 0

  #
  # Handle a response to this makemenu.
  #
  def response(self, player, response, arguments):
    # Change the markitem tag
    if not 1 in response.switches:
      player.deltag('markitem')
    else:
      player.settag('markitem', 1)
   
    # Show a gump with the last 10 items the player made.
    if response.button == 1:
      self.makehistory(player, arguments)

    # Directly execute the last action the user made.
    elif response.button == 2:
      if not self.makelast(player, arguments):
        player.socket.sysmessage("You didn't make anything yet.")
        self.send(player, arguments)

    # Show the parent menu.
    elif response.button == 3:
      if self.parent:
        self.parent.send(player)

    # Submenu
    elif response.button & 0x80000000:
      submenu = response.button & ~ 0x80000000
      if submenu < len(self.submenus):
        self.submenus[submenu].send(player, arguments)
    
    # Directly make subaction
    elif response.button & 0x40000000:
      subaction = response.button & ~ 0x40000000
      if subaction < len(self.subactions):
        self.subactions[subaction].make(player, arguments)

    # Enter subaction detail page
    elif response.button & 0x20000000:
      subaction = response.button & ~ 0x20000000
      if subaction < len(self.subactions):
        self.subactions[subaction].details(player, arguments)

    # MakeHistory: Make Item
    elif response.button & 0x10000000:
      subaction = response.button & ~ 0x10000000
      if player.hastag('makehistory'):
        history = str(player.gettag('makehistory')).split(';')
        if subaction < len(history):
          (menu, action) = history[subaction].split(':')
          action = int(action)
          global menus
          if menus.has_key(menu) and action < len(menus[menu].subactions):
            menus[menu].subactions[action].make(player, arguments)

    # MakeHistory: Show Detail Page
    elif response.button & 0x08000000:
      subaction = response.button & ~ 0x08000000
      if player.hastag('makehistory'):
        history = str(player.gettag('makehistory')).split(';')
        if subaction < len(history):
          (menu, action) = history[subaction].split(':')
          action = int(action)
          global menus
          if menus.has_key(menu) and action < len(menus[menu].subactions):
            menus[menu].subactions[action].details(player, arguments)     

  #
  # Generate the gump out of the properties of this
  # menu.
  #
  def generate(self):
    if self.gump:
      return

    # Create the main gump.
    gump = cGump()
    gump.callback = "system.makemenus.MakeMenuResponse"
    gump.setType(self.gumptype)
    gump.startPage(0)
    gump.addBackground(0x13be, 530, 417)
    gump.addTiledGump(10, 10, 510, 22, 0xA40)
    gump.addTiledGump(10,292, 150, 45, 0xA40)
    gump.addTiledGump(165, 292, 355, 45, 0xA40)
    gump.addTiledGump(10, 342, 510, 65, 0xA40)
    gump.addTiledGump(10, 37, 200, 250, 0xA40)
    gump.addTiledGump(215, 37, 305, 250, 0xA40)
    gump.addCheckerTrans(10, 10, 510, 397)
    
    gump.addHtmlGump(10, 12, 510, 20, centerhtml % self.title)
    gump.addHtmlGump(10, 39, 200, 20, centerhtml % "CATEGORIES")
    gump.addHtmlGump(215, 39, 305, 20, centerhtml % "SELECTIONS")
    gump.addHtmlGump(10, 294, 150, 20, centerhtml % "NOTICES")
    gump.addHtmlGump(170, 292, 345, 41, whitehtml % self.notices)
    
    # EXIT button , return value: 0
    gump.addButton(15, 382, 0xFB1, 0xFB3, 0)
    gump.addHtmlGump(50, 385, 150, 18, whitehtml % "EXIT")
    
    # LAST TEN button , return value: 1
    gump.addButton(15, 60, 4011, 4012, 1)
    gump.addHtmlGump(50, 63, 150, 18, whitehtml % "LAST TEN")
    
    # MAKE LAST button , return value: 2
    gump.addButton(270, 382, 4005, 4007, 2)
    gump.addHtmlGump(305, 385, 150, 18, whitehtml % "MAKE LAST")

    # PREVIOUS MENU button, return value: 3
    if self.parent:
      gump.addButton(15, 362, 0xFAE, 0xFB0, 3)
      gump.addHtmlGump(50, 365, 150, 18, whitehtml % "PREVIOUS MENU")

    # Sort them first if neccesary
    if self.sort:
      self.submenus.sort(comparetitle)
      self.subactions.sort(comparetitle)

    # Calculate how many pages are required
    menupages = math.ceil(len(self.submenus) / 9.0)
    actionpages = math.ceil(len(self.subactions) / 10.0)
    pages = int(max(menupages, actionpages))

    actions = 0
    menus = 0

    for i in range(0, pages):
      gump.startPage(i+1)
      
      # Fill the page with submenus
      for j in range(0, 9):
        if actions + j < len(self.submenus):
          yoffset = 80 + 20 * j
          gump.addButton(15, yoffset, 4005, 4007, 0x80000000 | (actions + j))
          gump.addHtmlGump(50, yoffset + 3, 150, 18, whitehtml % self.submenus[actions + j].title)
      actions += 9

      # Fill the page with subactions
      for j in range(0, 10):
        if menus + j < len(self.subactions):
          yoffset = 60 + 20 * j
          gump.addButton(220, yoffset, 4005, 4007, 0x40000000 | (menus + j))
          if self.subactions[menus + j].hasdetails:
            gump.addButton(480, yoffset, 4011, 4012, 0x20000000 | (menus + j))
          gump.addHtmlGump(255, yoffset+3, 220, 18, whitehtml % self.subactions[menus + j].title)
      menus += 9

      # Add a back button
      if i > 0:
        gump.addPageButton(15, 342, 0xFAE, 0xFB0, i)
        gump.addHtmlGump(50, 345, 150, 18, whitehtml % "PREVIOUS PAGE")

      # Add a next button
      if i+1 < pages:
        gump.addPageButton(270, 342, 4005, 4007, i + 2)
        gump.addHtmlGump(305, 345, 150, 18, whitehtml % "NEXT PAGE")

    self.gump = gump

  #
  # Send this menu to a character. If the gump
  # has not yet been generated. Autogenerate it.
  # Args are additional arguments you want to have
  # passed on between gump calls.
  #
  def send(self, player, args = []):
    if not self.gump:
      self.generate()

    gump = cGump.copy(self.gump)
    # MARK ITEM switch , switch index: 1
    if player.hastag('markitem'):
      gump.addCheckbox(270, 362, 4005, 4007, 1, 1)
    else:
      gump.addCheckbox(270, 362, 4005, 4007, 1, 0)
    gump.addHtmlGump(305, 365, 150, 18, whitehtml % "MARK ITEM")
    gump.setArgs([self.id] + args)
    gump.send(player)

