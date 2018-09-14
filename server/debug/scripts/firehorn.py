
import wolfpack
import wolfpack.time
from wolfpack.consts import *
from wolfpack.utilities import mayAreaHarm
import random

AMOUNT = 4 # The amount of sulfurous ash which is consumed
RANGE = 3
FIREHORN_DELAY = 6000
BREAK_CHANCE = 0.01 # 1% chance the firehorn breaks


def onUse(char, item):
	if checkuse(char, item):
		char.socket.clilocmessage( 1049620 ) # Select an area to incinerate.
		char.socket.attachtarget('firehorn.target', [item.serial])
	return True

def target(char, args, target):
	firehorn = wolfpack.finditem(args[0])
	if not firehorn:
		return False
	if not char.canreach(target, RANGE):
		return False
	if target.item:
		loc = None
		if target.item.container:
			loc = target.item.getoutmostitem().pos
		else:
			loc = target.item.pos
	else:
		loc = target.pos
	usehorn(char, firehorn, loc)

def checksuccess(char, chance):
	if chance < 0.0:
		return False # Too difficult
	elif chance >= 1.0:
		return True # No challenge
	else:
		return chance >= random.random()

def usehorn(char, firehorn, loc):
	if not checkuse(char, firehorn):
		return False

	char.socket.settag('firehorn_delay', wolfpack.time.currenttime() + FIREHORN_DELAY)

	music = char.skill[MUSICIANSHIP]

	sucChance = 500 + ( music - 775 ) * 2
	dSucChance = sucChance / 1000.0

	if not checksuccess(char, dSucChance):
		char.socket.clilocmessage(1049618) # The horn emits a pathetic squeak.
		char.soundeffect(0x18a)
		return False

	char.useresource(AMOUNT, 0xf8c)

	char.soundeffect(0x15f)
	char.movingeffect(0x36d4, loc, 1, 0, 5)

	eable = wolfpack.chars(loc.x, loc.y, loc.map, RANGE)

	targets = []
	for mobile in eable:
		if mayAreaHarm(char, mobile, excludeself = True, includeinnocents = True):
			targets.append(mobile)
			playerVsPlayer = False
			if mobile.player:
				playerVsPlayer = True

	if len(targets) > 0:
		prov = char.skill[PROVOCATION] / 10
		entic = char.skill[ENTICEMENT] / 10
		peace = char.skill[PEACEMAKING] / 10

		minDamage = 0
		maxDamage = 0

		musicScaled = music + max( 0, music - 900 ) * 2
		provScaled = prov + max( 0, prov - 900 ) * 2
		enticScaled = entic + max( 0, entic - 900 ) * 2
		peaceScaled = peace + max( 0, peace - 900 ) * 2

		weightAvg = ( musicScaled + provScaled * 3 + enticScaled * 3 + peaceScaled ) / 80

		avgDamage = 0
		if playerVsPlayer:
			avgDamage = weightAvg / 3
		else:
			avgDamage = weightAvg / 2

		minDamage = ( avgDamage * 9 ) / 10
		maxDamage = ( avgDamage * 10 ) / 9

		damage = random.randint(minDamage, maxDamage)

		if len(targets) > 1:
			damage = (damage * 2) / len(targets)

		for i in targets:
			if i.checkskill(MAGICRESISTANCE, 0, 1200):
				damage *= 0.5
				if i.socket:
					i.socket.clilocmessage( 501783 ) # You feel yourself resisting magical energy.
				if not i.attacktarget:
					i.fight(char)
				i.damage(1, damage, char)
				i.effect(0x3709, 10, 30)
				i.say("tes")

	if random.random() < BREAK_CHANCE:
		char.socket.clilocmessage( 1049619 ) # The fire horn crumbles in your hands.
		firehorn.delete()

def checkuse(char, item):
	if not char.canreach(item, 2):
		char.message(1019045) # I can't reach that.
		return False

	if char.socket.hastag( 'firehorn_delay' ):
		if wolfpack.time.currenttime() < char.socket.gettag( 'firehorn_delay' ):
			char.socket.clilocmessage(1049615) # You must take a moment to catch your breath.
			return False
		else:
			char.socket.deltag( 'firehorn_delay' )

	if not char.countresource(0xf8c) >= AMOUNT:
		char.socket.clilocmessage(1049617) # You do not have enough sulfurous ash.
		return False

	return True
