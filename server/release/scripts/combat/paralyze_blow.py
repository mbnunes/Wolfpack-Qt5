#
# "Paralyzing Blow - A successful Paralyzing Blow will leave the target stunned,
# unable to move, attack, or cast spells, for a few seconds. 
# Duration is 3 seconds (player), 6 seconds (NPC). The duration is not broken by combat damage the way the paralyze spell is."
#
# if this doesn't exist, the char will be unfrozen after a hit
def onDamage(char, type, amount, source):
	char.frozen = True
	return amount
