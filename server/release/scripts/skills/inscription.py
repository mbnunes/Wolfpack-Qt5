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

# reagents itemdef:noreags_msg_id ( reagents name cliloc id = 1044353 + id )
reagents = { 'f7a':0, 'f7b':1, 'f84':2, 'f85':3, 'f86':4, 'f88':5, 'f8c':6, 'f8d':7 }
# mana[circle - 1] required to make a spell croll
req_manas = [4, 6, 9, 11, 14, 20, 40, 50]
# min / max skills for each circle spell scroll / runebook / BOD
req_skills = [[0, 250], [0, 392], [35, 535], [178, 678], [321, 821], [464, 964], [607, 1107], [750, 1250], [450, 1250], [650, 1250]]
# spell circle menu ids
circle_ids = [1015163, 1015171, 1015177, 1015185, 1015193, 1015202, 1015210, 1015219]
# spell scroll cliloc ids : 1027981 - 1028044
# spell scroll itemid
#	if reactive armor : itemid = 1f2d
#	elif spell_num < weaken : itemid = 1f2f + spell_num
#	else : itemid = 1f2d + spell_num
# empty scroll cliloc id : 1044377
# empty recall rune cliloc id : 104447
# reagents required to make a spell scroll (reactive armor first)
spell_regs = [\
		['f84', 'f8d', 'f8c'],\
		['f7b', 'f88'],\
		['f84', 'f85', 'f86'],\
		['f88', 'f85'],\
		['f84', 'f85', 'f8d'],\
		['f8c'],\
		['f8d', 'f8c'],\
		['f84', 'f88'],\
		['f7b', 'f86'],\
		['f88', 'f86'],\
		['f84', 'f85'],\
		['f88', 'f8d'],\
		['f84', 'f8d', 'f8c'],\
		['f7b', 'f8c'],\
		['f84', 'f85', 'f8c'],\
		['f86', 'f88'],\
		['f86', 'f84'],\
		['f7a'],\
		['f7b', 'f84', 'f8c'],\
		['f88'],\
		['f7b', 'f86'],\
		['f7b', 'f86'],\
		['f7b', 'f8c'],\
		['f7b', 'f84'],\
		['f84', 'f85', 'f86'],\
		['f84', 'f85', 'f86', 'f8c'],\
		['f84', 'f88', 'f8c'],\
		['f7a', 'f8d', 'f8c'],\
		['f84', 'f85', 'f86', 'f8d'],\
		['f86', 'f8c'],\
		['f7a', 'f86', 'f8d'],\
		['f7a', 'f7b', 'f86'],\
		['f7a', 'f86', 'f88'],\
		['f7a', 'f84', 'f8d', 'f8c'],\
		['f7b', 'f84', 'f88'],\
		['f84', 'f86', 'f8d'],\
		['f7a', 'f86', 'f88', 'f8c'],\
		['f84', 'f86', 'f8d'],\
		['f7a', 'f88', 'f8d'],\
		['f7b', 'f86', 'f8d'],\
		['f84', 'f86', 'f8c'],\
		['f7a', 'f88'],\
		['f7b', 'f86'],\
		['f7b', 'f88'],\
		['f7a', 'f7b', 'f86'],\
		['f84', 'f86', 'f88', 'f8c'],\
		['f7a', 'f85', 'f8d'],\
		['f7b', 'f8c'],\
		['f7a', 'f7b', 'f86', 'f8c'],\
		['f7a', 'f86', 'f8d', 'f8c'],\
		['f8d', 'f8c'],\
		['f7a', 'f86', 'f8c'],\
		['f7a', 'f7b', 'f86', 'f8d'],\
		['f7a', 'f84', 'f86', 'f8c'],\
		['f7b', 'f86', 'f8d', 'f8c'],\
		['f7b', 'f86', 'f8d'],\
		['f7b', 'f85', 'f86', 'f8c'],\
		['f7a', 'f7b', 'f86', 'f88'],\
		['f7b', 'f84', 'f85'],\
		['f7b', 'f86', 'f8d'],\
		['f7b', 'f86', 'f8d', 'f8c'],\
		['f7b', 'f86', 'f8d'],\
		['f7b', 'f86', 'f8d', 'f8c'],\
		['f7b', 'f86', 'f8d']]

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

def generateMenu( id, parent = None ):
	titleid = 1015162
	menu0 = InscriptionMenu(id, parent, titleid)
	# add spell scrolls
	# loop circles
	for i in range(0, 8):
		menu = InscriptionMenu(id + str(i), menu0, circle_ids[i])
		# loop spells
		for j in range(0, 8):
			spell_num = i * 8 + j
			# if reactive armor spell
			if spell_num == 6:
				spell_num = 0
			elif spell_num < 6:
				spell_num += 1
			# spell < weaken
			itemdef = str(hex(int(0x1f2d) + spell_num)).replace('0x', '')
			item2 = wolfpack.getdefinition(WPDT_ITEM, itemdef)
			if item2:
				itemchild = item2.findchild('id')
				if itemchild:
					itemid = int(itemchild.value)
			actionid = 1027981 + spell_num
			action = InsItemAction(menu, itemid, itemdef, actionid)
			# required mana
			action.mana = req_manas[i]
			# empty scroll
			action.materials.append([['ef3','e34'], 1, 1044377])
			# required reagents
			regs = spell_regs[spell_num]
			for k in range(0, len(regs)):
				action.materials.append([[regs[k]], 1, reagents[regs[k]] + 1044353])
			# skill
			action.skills[INSCRIPTION] = req_skills[i]

	# Others
	menu = InscriptionMenu(id + '8', menu0, 1044294)
	# runebook
	itemdef = '22c5'
	item2 = wolfpack.getdefinition(WPDT_ITEM, itemdef)
	itemchild = item2.findchild('id')
	itemid = int(itemchild.value)
	actionid = 1041267
	action = InsItemAction(menu, itemid, itemdef, actionid)
	# recall scroll
	action.materials.append([['1f4c'], 1, 1044445])
	# gate travel scroll
	action.materials.append([['1f60'], 1, 1044446])
	# 8 runes
	action.materials.append([['1f14', '1f15', '1f16', '1f17'], 8, 1044447])
	# bulk order book : TODO : not yet defined in xml
	#actionid = 1028793
	#action = InsItemAction(menu, itemid, itemdef, actionid)
	#action.materials.append([['ef3','e34'], 10, 1044377])

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
				elif reagents.has_key(baseids[0]):
					msg_id = reagents[baseids[0]]
				# recall rune
				#elif '1f14' in baseids or '1f15' in baseids or '1f16' in baseids or '1f17' in baseids:
				else:
					msg_id = -108
				player.socket.clilocmessage(1044361 + msg_id)
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
			self.consumematerial(player, args, 0)
			success = self.checkskills( player, args, 1)
			if success:
				player.mana = player.mana - self.mana
				item = wolfpack.additem(self.definition)
				if not item:
					console.log(LOG_ERROR, "Unknown item definition used in action %u of menu %s.\n" % (self.parent.subactions.index(self), self.parent.id))
				else:
					item.decay = 1
					item.movable = 1
					if not tobackpack(item, player):
						item.update()
			self.parent.send(player, args)
			return success
		# runebook / bulk order book
		else:
			# runebook
			if self.titleid == 1041267:
				if not self.checkmaterial(player, args):
					self.noticeid = 1044253
					self.noticestr = ''
					self.parent.send(player, args)
					return False
				self.consumematerial(player, args, 0)
				success = self.checkskills( player, args, 1)
				if success:
					item = wolfpack.additem(self.definition)
					if not item:
						console.log(LOG_ERROR, "Unknown item definition used in action %u of menu %s.\n" % (self.parent.subactions.index(self), self.parent.id))
					else:
						item.decay = 1
						item.movable = 1
						if not tobackpack(item, player):
							item.update()
				self.parent.send(player, args)
				return success
			else:

				self.noticeid = 0
				self.noticestr = 'Not implemented yet'
				player.socket.sysmessage(self.noticestr)
				self.parent.send(player, args)
				return False

def onLoad():
	generateMenu( 'INSCRIPTION' )
	skills.register( INSCRIPTION, inscription )
