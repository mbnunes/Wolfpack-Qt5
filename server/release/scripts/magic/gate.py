
import wolfpack
from wolfpack import utilities

def onCollide(player, item):
	if not item.hastag('target'):
		target = [item.pos.x, item.pos.y, item.pos.z, item.pos.map]
	else:
		target = str(item.gettag('target')).split(',')

	try:
		target = map(int, target)
	except:
		target = [item.pos.x, item.pos.y, item.pos.z, item.pos.map]

	# Den Spieler bewegen
	pos = player.pos
	pos.x = target[0]
	pos.y = target[1]
	pos.z = target[2]
	pos.map = target[3]

	if not pos.validspawnspot():
		if player.socket:
			player.socket.clilocmessage(501942)
		return

	# Move his pets if he has any
	if player.player:
		for follower in player.followers:
			if follower.wandertype == 4 and follower.distanceto(player) < 5:
				follower.removefromview()
				follower.moveto(location)
				follower.update(0)

	player.removefromview()
	player.moveto(pos)
	player.update()
	if player.socket:
		player.socket.resendworld()

	# An der alten und neuen position soundeffekt und effekt spielen
	item.soundeffect(0x1fe)
	
	utilities.smokepuff(player, player.pos)
	utilities.smokepuff(player, item.pos)
	return 1
