
#
# Resurrect players
#
def onCollide(player, item):
	if not player.dead or not player.player or not player.socket:
		return

	player.resurrect()
	return 1
