
from wolfpack.consts import *
import system.poison
impot random

def onDoDamage(npc, damagetype, amount, victim):
	if damagetype != DAMAGE_PHYSICAL:
		return amount

	if npc == victim:
		return amount # Safety measure so we never poison ourselves

	hit_poison_level = npc.getintproperty('hit_poison_level', 0)
	hit_poison_chance = npc.getintproperty('hit_poison_chance', 50)

	if hit_poison_level < 0 or hit_poison_level > 4:
		return amount
	
	if hit_poison_chance <= 0:
		return amount
		
	# Check the poisoning chance
	if hit_poison_chance > random.randint(0, 99):
		system.poison(victim, hit_poison_level)

	return amount
