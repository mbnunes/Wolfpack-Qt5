
import wolfpack.utilities
import math
import random

def dispel(char, args, source, dispelargs):
	expire(char, dispelargs)

def expire(char, args):
	if char.socket:
		char.socket.clilocmessage( 1061687 ) # You can breath normally again.
	char.removescript('magic.strangle')
	return True

def dodamage(char, args):
	if char.dead:
		return

	(spiritLevel, MinBaseDamage, MaxBaseDamage, HitDelay, MaxCount, Count) = args

	Count -= 1

	if HitDelay > 1:
		if MaxCount < 5:
			HitDelay -= 1
		else:
			delay = int(math.ceil( (1.0 + (5 * Count)) / MaxCount ) )

			if delay <= 5:
				HitDelay = delay
			else:
				HitDelay = 5
	if Count == 0:
		return expire(char, args)
	else:
		NextHit = HitDelay * 1000

		damage = MinBaseDamage + (random.random() * (MaxBaseDamage - MinBaseDamage))

		damage *= (3 - ((char.stamina / char.maxstamina) * 2))

		if damage < 1:
			damage = 1

		if not char.player:
			damage *= 1.75

		wolfpack.utilities.energydamage(char, char, int(damage), 0, 0, 0, 100)
		char.addtimer(NextHit, dodamage, [spiritLevel, MinBaseDamage, MaxBaseDamage, HitDelay, MaxCount, Count], True, False, 'STRANGLE', dispel )
