#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: Viper                          #
#   )).-" {{ ;"`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

from wolfpack.consts import *
from wolfpack.time import *
import wolfpack
from math import floor
import random
import skills

SPSPEAK_DELAY = 5000

def spiritspeak(char, skill):
	socket = char.socket

	if socket.hastag('spiritspeaking'):
		socket.clilocmessage(500118)
		return 1

	if socket.hastag('skill_delay'):
		cur_time = servertime()
		if cur_time < socket.gettag( 'skill_delay' ):
			socket.clilocmessage(500118)
			return 1
		else:
			socket.deltag('skill_delay')

	if char.health >= char.maxhitpoints:
		char.socket.clilocmessage(1061288)
		return 1

	char.say("Anh Mi Sah Ko")
	char.soundeffect(0x24a)
	char.action(ANIM_CASTAREA)
	char.addtimer(1000, "skills.spiritspeak.effect", [skill])
	socket.settag('spiritspeaking', 1)
	return 1

def effect(char, args):
	# Delete the spiritspeaking flag and set the
	# skill delay for this client.
	socket = char.socket
	socket.deltag('spiritspeaking')
	socket.settag('skill_delay', servertime() + SPSPEAK_DELAY)

	# Check for skill usage success
	if not char.checkskill(SPIRITSPEAK, 0, 1000):
		char.socket.clilocmessage(502443)
		return

	# Find corpses around the user of this skill
	# 3 tiles range.
	pos = char.pos
	items = wolfpack.items(pos.x, pos.y, pos.map, 3)
	corpses = []
	for item in items:
		if item.id == 0x2006 and not item.hastag('drained'):
			corpses.append(item)

	# There was an undrained corpse near the player using
	# the skill.
	if len(corpses) > 0:
		# Select a random corpse.
		corpse = random.choice(corpses)

		# Change the color of the corpse and set the "drained" flag for it.
		corpse.color = 0x835
		corpse.settag('drained', 1)
		corpse.update()

		char.socket.clilocmessage(1061287)
	else:
		if char.mana < 10:
			char.socket.clilocmessage(1061285)
			return

		char.mana -= 10
		char.updatemana()

		char.socket.clilocmessage(1061286)

	# Show a nice effect.
	char.effect(0x375a, 1, 15)

	# The amount of damage healed is based on the spirit speaking value.
	minval = 1 + floor(char.skill[SPIRITSPEAK] * 0.025)
	maxval = minval + 4
	char.health = min(char.maxhitpoints, char.health + random.randint(minval, maxval))
	char.updatehealth()

def onLoad():
	skills.register(SPIRITSPEAK, spiritspeak)
