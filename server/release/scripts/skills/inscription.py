#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: khpae                          #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

from wolfpack.consts import *
from wolfpack.utilities import *
import wolfpack
from system.craftmenu import *
import skills
from wolfpack import console
from math import floor,ceil

# reagents itemdef:cliloc_id
reagents = { 'f7a':0, 'f7b':1, 'f84':2, 'f85':3, 'f86':4, 'f88':5, 'f8c':6, 'f8d':7 }

def checktool(char, item, wearout=0):
	if not item:
		return False
	if item.getoutmostchar() != char:
		char.socket.clilocmessage(500364)
		return False
	# use this for a Siege-style shard
	#if not item.hastag('remaining_uses'):
	#	char.socket.clilocmessage(1044038)
	#	item.delete()
	#	return False
	#if wearout:
	#	uses = int(item.gettag('remaining_uses'))
	#	if uses < 1:
	#		char.socket.clilocmessage(1044038)
	#		item.delete()
	#		return False
	#	else:
	#		item.settag('remaining_uses', uses - 1)
	return True

# skill is used via the blue button on skill gump - copy a book
def inscription(char, skill):
	if skill != INSCRIPTION:
		return False
	char.socket.sysmessage("copying a book is not implemented yet")
	return True

# skill is used via a scribe's pen
def onUse(char, item):
	#if not checktool(char, item):
	#	return True
	# send makemenu
	menu = findmenu('INSCRIPTION')
	if menu:
		menu.send(char, [item.serial])
	return True

class InscriptionMenu(CraftMenu):
	def __init__(self, id, parent, titleid=0, title=''):
		CraftMenu.__init__(self, id, parent, titleid, title)
		self.allowmark = 1
		self.allowrepair = 0
		self.gumptype = 0xce123456

def loadMenu( id, parent = None ):
	definition = wolfpack.getdefinition(WPDT_MENU, id)
	if not definition:
		if parent:
			console.log(LOG_ERROR, "Unknown submenu %s in menu %s.\n" % (id, parent.id))
		else:
			console.log(LOG_ERROR, "Unknown menu: %s.\n" % id)
		return
	# use 'clilocid' title for OSI style
	# or use 'title' for custom craft menu
	title = ''
	titleid = 0
	if definition.hasattribute('title'):
		title = definition.getattribute('title')
	elif definition.hasattribute('clilocid'):
		titleid = int(definition.getattribute('clilocid', '0'))
	menu = InscriptionMenu(id, parent, titleid, title)
	for i in range(0, definition.childcount):
		child = definition.getchild(i)
		if child.name == 'menu':
			loadMenu(child.getattribute('id'), menu)
		elif child.name == 'action':
			actionclilocid = 0
			actionname = ''
			if child.hasattribute('clilocid'):
				actionclilocid = int(child.getattribute('clilocid'))
			elif child.hasattribute('title'):
				actionname = child.getattribute('title')
			else:
				console.log(LOG_ERROR, "Action without title or cliloc id in menu %s.\n" % id)
				pass
			itemdef = child.getattribute('itemid')
			item2 = wolfpack.getdefinition(WPDT_ITEM, itemdef)
			if item2:
				itemchild = item2.findchild('id')
				if itemchild:
					itemid = int(itemchild.value)
			action = InsItemAction(menu, itemid, itemdef, actionclilocid, actionname)
			for j in range(0, child.childcount):
				subchild = child.getchild(j)
				if subchild.name == 'mana':
					action.mana = int(subchild.getattribute('amount', '0'))
				elif subchild.name == 'spell':
					action.spell = int(subchild.getattribute('id', '0'))
				elif subchild.name == 'scroll':
					ids = subchild.getattribute('id').split(';')
					amount = int(subchild.getattribute('amount', '1'))
					action.materials.append([ids, amount, 'scroll'])
				elif subchild.name == 'reagents':
					ids = subchild.getattribute('id').split(';')
					amount = int(subchild.getattribute('amount', '1'))
					name = subchild.getattribute('name')
					action.materials.append([ids, amount, name])
				elif subchild.name in skillnamesids:
					skill = skillnamesids[subchild.name]
					minimum = int(subchild.getattribute('min', '0'))
					maximum = int(subchild.getattribute('max', '1200'))
					action.skills[skill] = [minimum, maximum]

#	menu.sort()

class InsItemAction(CraftItemAction):
	def __init__(self, parent, itemid, definition, titleid=0, title=''):
		CraftItemAction.__init__(self, parent, itemid, definition, titleid, title)
		self.markable = 1
		self.mana = 0
		self.spell = 0

	# FIXME : exceptional / success chance
	def getexceptionalchance(self, player, args):
		return False

	def getsuccesschance(self, player, args):
		return False

	def checkmaterial(self, player, args, silent=0):
		backpack = player.getbackpack()
		for material in self.materials:
			(baseids, amount) = material[:2]
			count = backpack.countitems(baseids)
			if count < amount:
				# empty scroll
				if 'ef3' in baseids or 'e34' in baseids:
					msg_id = 1044378 - 1044361
				elif reagents.haskey(baseids[0]):
					msg_id = reagents[baseids[0]]
				player.socket.clilocmessage(1044361 + msg_id)
				# recall rune
				#elif '1f14' in baseids or '1f15' in baseids or '1f16' in baseids or '1f17' in baseids:
				return False
		return True

	def make(self, player, args, nodelay=0):
		if not checktool(player, wolfpack.finditem(args[0]), 0):
			return False
		# spell scrolls
		# FIXME : spell no. is not correct
		if self.titleid > 1027980 and self.titleid < 1028045:
			if self.titleid == 1027981:
				self.spell = 6
			elif self.titleid > 1027988:
				self.spell = self.titleid - 1027981
			else:
				self.spell = self.titleid - 1027982
			if not hasSpell(player, self.spell):
				self.noticeid = 0
				self.noticestr = "You don't have that spell in your spellbook."
				self.parent.send(player, args)
				player.socket.sysmessage(self.noticestr)
				return False
			if player.mana < self.mana:
				player.socket.clilocmessage(1044380)
				self.noticeid = 1044380
				self.noticestr = ''
				self.parent.send(player, args)
				return False
			if player.socket.hastag('craftmenu_crafting'):
				player.socket.clilocmessage(500119)
				self.noticeid = 500119
				self.noticestr = ''
				self.parent.send(player, args)
				return False
			if not self.checkmaterial(player, args):
				self.parent.send(player, args)
				return False
			player.mana = player.mana - self.mana
			self.consumematerial(player, args, 0)
			success = self.checkskills( player, args, 1)
			if success:
				item = wolfpack.additem(self.definition)
				if not item:
					console.log(LOG_ERROR, "Unknown item definition used in action %u of menu %s.\n" % (self.parent.subactions.index(self), self.parent.id))
				else:
					item.decay = 1
					item.magic = 1
					if not tobackpack(item, player):
						item.update()
			self.parent.send(player, args)
			return success
		# runebook / bulk order deed
		else:
			self.noticeid = 0
			self.noticestr = 'Not implemented yet'
			self.parent.send(player, args)
			return False

def onLoad():
	loadMenu( 'INSCRIPTION' )
	skills.register( INSCRIPTION, inscription )
