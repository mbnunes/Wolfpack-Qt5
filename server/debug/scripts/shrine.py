
#
# Resurrect players
#
def onCollide(player, item):
	if not player.dead or not player.player or not player.socket:
		return False

	player.resurrect( item )
	return True
