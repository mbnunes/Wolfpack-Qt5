
transformation_spells = [106, 107, 113, 116 ]

def onCastSpell(player, spell):
	# Only Transformation Spells are allowed
	if not spell in transformation_spells:
		player.socket.clilocmessage( 1061091 ) # You cannot cast that spell in this form.
		return True

def onRegenHitpoints(char, points):
	bonus = 20
	return bonus
