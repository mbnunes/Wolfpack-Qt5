#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: Antigravity                    #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#
# Power Scroll Implementation (ServUO Compatible)              #
#===============================================================#

import random
import wolfpack
import wolfpack.gumps
import wolfpack.console
from wolfpack.consts import *
from wolfpack import tr

# Skill lists valid for Power Scrolls
POWERSCROLL_SKILLS = [
	SWORDSMANSHIP, FENCING, MACEFIGHTING, ARCHERY, WRESTLING,
	PARRYING, TACTICS, ANATOMY, HEALING, MAGERY,
	MEDITATION, EVALUATINGINTEL, MAGICRESISTANCE,
	TAMING, ANIMALLORE, VETERINARY, MUSICIANSHIP,
	PROVOCATION, PEACEMAKING, ENTICEMENT,
	BLACKSMITHING, TAILORING,
	CHIVALRY, FOCUS, NECROMANCY, STEALING,
	STEALTH, SPIRITSPEAK, BUSHIDO, NINJITSU,
	SPELLWEAVING
]

SKILL_NAMES_MAP = {
	SWORDSMANSHIP: "Swordsmanship",
	FENCING: "Fencing",
	MACEFIGHTING: "Mace Fighting",
	ARCHERY: "Archery",
	WRESTLING: "Wrestling",
	PARRYING: "Parrying",
	TACTICS: "Tactics",
	ANATOMY: "Anatomy",
	HEALING: "Healing",
	MAGERY: "Magery",
	MEDITATION: "Meditation",
	EVALUATINGINTEL: "Evaluating Intelligence",
	MAGICRESISTANCE: "Resisting Spells",
	TAMING: "Animal Taming",
	ANIMALLORE: "Animal Lore",
	VETERINARY: "Veterinary",
	MUSICIANSHIP: "Musicianship",
	PROVOCATION: "Provocation",
	PEACEMAKING: "Peacemaking",
	ENTICEMENT: "Discordance",
	BLACKSMITHING: "Blacksmithing",
	TAILORING: "Tailoring",
	CHIVALRY: "Chivalry",
	FOCUS: "Focus",
	NECROMANCY: "Necromancy",
	STEALING: "Stealing",
	STEALTH: "Stealth",
	SPIRITSPEAK: "Spirit Speak",
	BUSHIDO: "Bushido",
	NINJITSU: "Ninjitsu",
	SPELLWEAVING: "Spellweaving",
}

def get_skill_name(skill_id):
	if skill_id in SKILL_NAMES_MAP:
		return SKILL_NAMES_MAP[skill_id]
	if 0 <= skill_id < len(SKILLNAMES):
		return SKILLNAMES[skill_id].capitalize()
	return "Unknown Skill"

def create_powerscroll(skill_id=None, value=None):
	"""
	Factory function to create a Power Scroll.
	value: 1050, 1100, 1150, 1200 (in tenths, 105.0 to 120.0)
	"""
	if skill_id is None:
		skill_id = random.choice(POWERSCROLL_SKILLS)
	if value is None:
		roll = random.random()
		if roll < 0.05:
			value = 1200
		elif roll < 0.40:
			value = 1150
		else:
			value = 1100

	item = wolfpack.newitem("powerscroll")
	if item:
		item.settag("skill", int(skill_id))
		item.settag("value", int(value))
		item.hue = 0x481
		item.resendtooltip()
	return item

def onCreate(item, definition):
	if not item.hastag("skill"):
		item.settag("skill", int(random.choice(POWERSCROLL_SKILLS)))
	if not item.hastag("value"):
		item.settag("value", 1100)
	item.hue = 0x481

def onUse(char, item):
	doUse(char, item, True)
	return True

def doUse(char, item, firstStage):
	if item.getoutmostchar() != char:
		char.socket.clilocmessage(1042001) # That must be in your pack for you to use it.
		return True

	try:
		skill_id = int(item.gettag("skill"))
		value = int(item.gettag("value"))
	except Exception:
		char.socket.sysmessage("Invalid power scroll properties.")
		return False

	current_cap = char.skillcap[skill_id]

	if current_cap >= value:
		char.socket.clilocmessage(1049511) # Your skill is too high for this power scroll.
		return True

	if firstStage:
		InternalGump(char, item, skill_id, value)
	else:
		char.socket.clilocmessage(1049512) # You feel a surge of magic as the scroll enhances your powers!
		char.skillcap[skill_id] = value
		char.pos.soundeffect(0x243)
		char.effect(0x375A, 35, 90)
		item.delete()

	return True

def InternalGump(char, item, skill_id, value):
	gump = wolfpack.gumps.cGump()
	gump.setCallback(gumpCallback)
	gump.setArgs([item.serial])

	gump.addResizeGump(25, 10, 5054, 420, 200)
	gump.addTiledGump(33, 20, 401, 181, 2624)
	gump.addCheckerTrans(33, 20, 401, 181)

	# Explanatory text
	gump.addXmfHtmlGump(40, 48, 387, 100, 1049469, 1, 1)

	# Do you wish to use this scroll?
	gump.addXmfHtmlGump(125, 148, 200, 20, 1049478, 0, 0, 0xFFFFFF)

	# Yes / No buttons
	gump.addButton(100, 172, 4005, 4007, 1)
	gump.addXmfHtmlGump(135, 172, 120, 20, 1046362, 0, 0, 0xFFFFFF) # Yes

	gump.addButton(275, 172, 4005, 4007, 0)
	gump.addXmfHtmlGump(310, 172, 120, 20, 1046363, 0, 0, 0xFFFFFF) # No

	# Title at top
	skill_name = get_skill_name(skill_id)
	val_float = value / 10.0

	if value == 1050:
		title_type = "Wonderous"
	elif value == 1100:
		title_type = "Exalted"
	elif value == 1150:
		title_type = "Mythical"
	elif value == 1200:
		title_type = "Legendary"
	else:
		title_type = "Special"

	title = tr("<basefont color=#FFFFFF>%s Scroll of %s (%.1f):</basefont>" % (title_type, skill_name, val_float))
	gump.addHtmlGump(40, 20, 360, 20, title, 0, 0)

	gump.send(char.socket)

def gumpCallback(char, args, response):
	if response.button == 0:
		return False
	item = wolfpack.finditem(args[0])
	if item:
		doUse(char, item, False)
	return True

def onShowTooltip(char, item, tooltip):
	if not item.hastag('skill') or not item.hastag('value'):
		return False

	tooltip.reset()
	try:
		skill_id = int(item.gettag('skill'))
		value = int(item.gettag('value'))
	except Exception:
		return False

	skill_name = get_skill_name(skill_id)
	val_float = value / 10.0

	if value == 1050:
		scroll_type = "a wonderous scroll"
	elif value == 1100:
		scroll_type = "an exalted scroll"
	elif value == 1150:
		scroll_type = "a mythical scroll"
	elif value == 1200:
		scroll_type = "a legendary scroll"
	else:
		scroll_type = "a scroll"

	tooltip.add(1042971, "%s of %s (%.1f Skill)" % (scroll_type, skill_name, val_float))
	return True
