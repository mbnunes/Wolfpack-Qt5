#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: khpae                          #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

from wolfpack import tr
from wolfpack.consts import *
import math
import wolfpack
from system.makemenus import CraftItemAction, MakeMenu, findmenu
import random
import magic
import skills
from magic.utilities import *

# reagent names
reagents = {
	REAGENT_BLACKPEARL: tr("Black Pearl"),
	REAGENT_BLOODMOSS: tr("Blood Moss"),
	REAGENT_GARLIC: tr("Garlic"),
	REAGENT_GINSENG: tr("Ginseng"),
	REAGENT_MANDRAKE: tr("Mandrake Root"),
	REAGENT_NIGHTSHADE: tr("Nightshade"),
	REAGENT_SULFURASH: tr("Sulfurous Ash"),
	REAGENT_SPIDERSILK: tr("Spider's Silk"),
	
	# Rarer Reagents
	REAGENT_BATWING: tr("Batwing"),
	REAGENT_BLACKMOOR: tr("Blackmoor"),
	REAGENT_BLOODSPAWN: tr("Bloodspawn"),
	REAGENT_DAEMONBLOOD: tr("Daemon Blood"),
	REAGENT_BONE: tr("Bone"),
	REAGENT_BRIMSTONE: tr("Brimstone"),
	REAGENT_DAEMONBONE: tr("Daemon Bone"),
	REAGENT_FERTILEDIRT: tr("Fertile Dirt"),
	REAGENT_DRAGONSBLOOD: tr("Dragon's Blood"),
	REAGENT_EXECUTIONERSCAP: tr("Executioner's Cap"),
	REAGENT_EYEOFNEWT: tr("Eye of Newt"),
	REAGENT_OBSIDIAN: tr("Obsidian"),
	REAGENT_PIGIRON: tr("Pigiron"),
	REAGENT_PUMICE: tr("Pumice"),
	REAGENT_NOXCRYSTAL: tr("Nox Crystal"),
	REAGENT_GRAVEDUST: tr("Grave Dust"),
	REAGENT_DEADWOOD: tr("Deadwood"),
	REAGENT_WYRMSHEART: tr("Wyrm's Heart"),
}

# min / max skills for each circle spell scroll / runebook / BOD
req_skills = [[0, 250], [0, 392], [35, 535], [178, 678], [321, 821], [464, 964], [607, 1107], [750, 1250], [650, 1250]]
scrolls = {} # Registry for scrolls

# Create scroll registry
for i in range(0, 64):
	# first circle has special treatment
	if i < 8: 	
		if i < 6:
			scrolls[i] = [0x1f2e + i, '%x' % (0x1f2e + i)]
		elif i == 6:
			scrolls[i] = [0x1f2d, '1f2d']
		elif i == 7:
			scrolls[i] = [0x1f34, '1f34']
	else:
		scrolls[i] = [0x1f2d + i, '%x' % (0x1f2d + i)]

# skill is used via a scribe's pen
def onUse(char, item):
	menu = findmenu('INSCRIPTION')
	if menu:
		menu.send(char, [item.serial])
	return True

def isempty(book):
	pages = 64
	if book.hastag('pages'):
		pages = int(book.gettag('pages'))
	for page in range(1, pages+1):
		if book.hastag('page%u' % page):
			return False
	return True

#
# Skill is used via blue button
#
def inscription(player, skill):
	player.socket.clilocmessage( 1046295 ) # Target the book you wish to copy.
	player.socket.attachtarget('skills.inscription.copy', []) # , '', 'skills.inscription.timeout', 60000) # times out after 1 minute
	return True

def copy(char, args, target):
	if not target.item:
		return False
	if not char.canreach(target.item, 3):
		return False
	if not target.item.hasscript('book'):
		char.socket.clilocmessage( 1046296 ) # That is not a book
		return False
	if isempty(target.item):
		char.socket.clilocmessage( 501611 ) # Can't copy an empty book.
		return False

	char.socket.clilocmessage( 501612 ) # Select a book to copy this to.
	char.socket.attachtarget('skills.inscription.target_copy', [target.item]) #, '', 'skills.inscription.timeout', 60000) # times out after 1 minute
	return True

def timeout(char):
	char.socket.clilocmessage( 501619 ) # You have waited too long to make your inscribe selection, your inscription attempt has timed out.

def target_copy(char, args, target):
	if not target.item:
		return False
	if not char.canreach(target.item, 3):
		return False
	tocopy = args[0]
	if not target.item.hasscript('book'):
		char.socket.clilocmessage( 1046296 ) # That is not a book
		return False
	if tocopy == target.item:
		char.socket.clilocmessage( 501616 ) # Cannot copy a book onto itself.
		return False
	if target.item.hastag('protected'):
		char.socket.clilocmessage( 501614 ) # Cannot write into that book.
		return False
	if char.checkskill(INSCRIPTION, 0, 500):
		docopy(tocopy, target.item)
		char.socket.clilocmessage( 501618 ) # You make a copy of the book.
		char.soundeffect( 0x249 )
	else:
		char.socket.clilocmessage( 501617 ) # You fail to make a copy of the book.
		return False
	return True

def docopy(original, copy):
	if original.hastag('author'):
		copy.settag('author', unicode(original.gettag('author')))
	copy.name = original.name

	pages = 64
	if original.hastag('pages'):
		pages = int(original.gettag('pages'))

	for page in range(1, pages+1):
		if original.hastag('page%u' % page):
			copy.settag('page%u' % page, unicode(original.gettag('page%u' % page)))
	copy.resendtooltip()
	return True

#
# ScribeAction for SCROLLS ONLY
#
class ScribeItemAction(CraftItemAction):
	def __init__(self, parent, title, itemid, definition):
		CraftItemAction.__init__(self, parent, title, itemid, definition)
		self.markable = False # Scrolls aren't markable
		self.mana = 0
		self.spellid = 0

	#
	# This action requires mana.
	#
	def checkmaterial(self, player, arguments, silent = 0):
		result = CraftItemAction.checkmaterial(self, player, arguments, silent)
		
		if result:
			# Check for mana
			if self.mana > 0 and player.mana < self.mana:
				player.socket.clilocmessage(1044380) # You don't have enough mana to inscribe that spell.
				return False
			
			# Check for availability of spell
			if self.spellid > 0 and not hasSpell(player, self.spellid - 1, False):
				return False
		
		return result

	#
	# Consume material
	#
	def consumematerial(self, player, arguments, half = 0):
		result = CraftItemAction.consumematerial(self, player, arguments, half)
		
		if result and self.mana > 0:			
			if half:
				needed = int(math.ceil(self.mana / 2))
			else:
				needed = self.mana

			if player.mana >= needed:
				player.mana -= needed
				player.updatemana()
			else:
				return False

		return result

	#
	# Check if we did an exceptional job.
	#
	def getexceptionalchance(self, player, arguments):
		return 0 # No chance of getting exceptional scrolls

	#
	# Play a simple soundeffect
	#
	def playcrafteffect(self, player, arguments):
		player.soundeffect(0x249)

	#
	# We failed to create the item.
	# LostMaterials indicates whether materials where lost.
	#
	def fail(self, player, arguments, lostmaterials=0):
		if lostmaterials:
			player.socket.clilocmessage(501630)
		else:
			player.socket.clilocmessage(501630)

	#
	# Successful crafting of item.
	#
	def success(self, player, arguments, item, exceptional=0, marked=0):
		player.socket.clilocmessage(501629)

#
# The Inscription Menu
#
class InscriptionMenu(MakeMenu):
	def __init__(self, id, parent, title):
		MakeMenu.__init__(self, id, parent, title)
		self.allowmark = True	
		self.gumptype = 0xce123456 # This should be unique
		self.requiretool = True

#
# Action for Runebooks only
#
class CraftRunebook(CraftItemAction):
	def __init__(self, parent):
		CraftItemAction.__init__(self, parent, tr('Runebook'), 0x22c5, '22c5')
		self.markable = True
		self.runes = 8
		
		# Add the other requirements
		self.materials.append([['1f60'], 1, tr('Gate Travel Scrolls')])
		self.materials.append([['1f4c'], 1, tr('Recall Scrolls')])
		self.skills[INSCRIPTION] = [450, 1250, 250]

	#
	# Checks for the unmarked recall runes
	#
	def checkmaterial(self, player, arguments, silent = 0):
		result = CraftItemAction.checkmaterial(self, player, arguments, silent)
		
		# Check for runes
		backpack = player.getbackpack()
		if result and self.runes > 0:
			runes = 0
			for item in backpack.content:
				if item.baseid in ['1f14', '1f15', '1f16', '1f17'] and not item.hastag('location') and item.gettag('marked') == 0:
					runes += 1
					if runes >= self.runes:
						return True
						
			player.socket.clilocmessage(1044253)
			return False
		
		return result
		
	#
	# Consume the unmarked recall runes
	#
	def consumematerial(self, player, arguments, half = 0):
		result = CraftItemAction.consumematerial(self, player, arguments, half)
		
		if result and self.runes > 0:
			if half:
				needed = int(math.ceil(self.runes / 2))
			else:
				needed = self.runes
			
			backpack = player.getbackpack()
			for item in backpack.content:
				if item.baseid in ['1f14', '1f15', '1f16', '1f17'] and not item.hastag('location') and item.gettag('marked') == 0:
					item.delete()
					needed -= 1
					if needed < 1:
						break
			
			if needed > 0:
				player.socket.clilocmessage(1044253)
				return False

		return result

	#
	# Check if we did an exceptional job.
	#
	def getexceptionalchance(self, player, arguments):
		# Only works if this item requires blacksmithing
		if not self.skills.has_key(INSCRIPTION):
			return 0.0

		minskill = self.skills[INSCRIPTION][0]
		maxskill = self.skills[INSCRIPTION][1]
		chance = max(0.0, (player.skill[INSCRIPTION] - minskill) / float(maxskill - minskill) - 0.6)
		return chance

	#
	# Add our unmarked runes to the materials list
	#
	def getmaterialshtml(self, player, arguments):
		materialshtml = CraftItemAction.getmaterialshtml(self, player, arguments)
		
		if self.runes > 0:
			materialshtml += "%s: %u<br>" % (tr('Unmarked Runes'), self.runes)

		return materialshtml

	#
	# Play a simple soundeffect
	#
	def playcrafteffect(self, player, arguments):
		player.soundeffect(0x249)

def addOther(toplevel):
	other = InscriptionMenu('INSCRIPTION_OTHER', toplevel, tr('Other'))
	
	runebook = CraftRunebook(other) # Add The Runebook
	other.sort() # Sort the menu
	
	return other # Makes it a hell of a lot easier to override this

def generateMenu():
	# Generate the Menu
	toplevel = InscriptionMenu('INSCRIPTION', None, tr('Inscription'))
	
	circle_names = [tr('First Circle'), tr('Second Circle'), tr('Third Circle'), tr('Fourth Circle'), tr('Fifth Circle'), tr('Sixth Circle'), tr('Seventh Circle'), tr('Eight Circle')]

	# Generate one Menu per Circle
	for i in range(0, 8):
		circle = InscriptionMenu('INSCRIPTION_CIRCLE_%u' % i, toplevel, circle_names[i])
		
		# Add the Spells for this Circle
		for j in range(0, 8):
			spellid = i * 8 + j
			spell = magic.spells[spellid + 1]
			
			global scrolls
			action = ScribeItemAction(circle, (1044381 + scrolls[spellid][0] - 0x1f2d), scrolls[spellid][0], scrolls[spellid][1])
			action.index = j # We don't want to sort the circle
			action.mana = spell.mana
			action.materials.append([["ef3", "e34"], 1, tr('Blank Scrolls')])
			action.skills[INSCRIPTION] = [req_skills[spell.circle - 1][0], req_skills[spell.circle - 1][1], 250]
			
			global reagents
			for (baseid, amount) in spell.reagents.items():
				if baseid in reagents:
					action.materials.append([[baseid], amount, reagents[baseid]])

	addOther(toplevel)

def onLoad():
	generateMenu()
	skills.register(INSCRIPTION, inscription)
