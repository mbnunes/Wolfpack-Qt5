
import wolfpack
from wolfpack import console
from system.makemenus import MakeAction, MakeMenu, findmenu
from magic import castSpell

class PolymorphAction(MakeAction):
	def __init__(self, parent, title, bodyid):
		MakeAction.__init__(self, parent, title)
		self.bodyid = bodyid

	def make(self, player, arguments, nodelay=0):
		MakeAction.make(self, player, arguments, nodelay)
		castSpell(player, 56, 0, [self.bodyid])

class PolymorphMenu(MakeMenu):
  def __init__(self, id, parent, title):
    MakeMenu.__init__(self, id, parent, title)
    self.gumptype = 0xf4132a42 # This should be unique
    self.name_makelast = "Select Last"

def showmenu(player):
  menu = findmenu("POLYMORPH")
  menu.send(player)

# Populate the Polymorph menu
menu = PolymorphMenu("POLYMORPH", None, "POLYMORPH")

animals = PolymorphMenu("POLYMORPH_ANIMALS", menu, "Animals")
PolymorphAction(animals, "Chicken", 0xD0)
PolymorphAction(animals, "Dog", 0xD9)
PolymorphAction(animals, "Wolf", 0xE1)
PolymorphAction(animals, "Panther", 0xD6)
PolymorphAction(animals, "Gorilla", 0x1D)
PolymorphAction(animals, "Black Bear", 0xD3)
PolymorphAction(animals, "Grizzly Bear", 0xD4)
PolymorphAction(animals, "Polar Bear", 0xD5)
PolymorphAction(animals, "Human Male", 0x190)
animals.sort()

monster = PolymorphMenu("POLYMORPH_MONSTER", menu, "Monster")
PolymorphAction(monster, "Slime", 0x33)
PolymorphAction(monster, "Orc", 0x11)
PolymorphAction(monster, "Lizard Man", 0x21)
PolymorphAction(monster, "Gargoyle", 0x04)
PolymorphAction(monster, "Ogre", 0x01)
PolymorphAction(monster, "Troll", 0x36)
PolymorphAction(monster, "Ettin", 0x02)
PolymorphAction(monster, "Daemon", 0x09)
PolymorphAction(monster, "Human Female", 0x191)
monster.sort()

menu.sort()
