
import wolfpack

#
# This script is attached to all rideable animals
#
def onShowPaperdoll(char, player):
	if player.socket and player.socket.hastag('block_mount'):
		expire = int(player.socket.gettag('block_mount'))
		if expire < wolfpack.currenttime():
			player.socket.deltag('block_mount')
		else:
			player.socket.clilocmessage(1040024) # Too dazed...
			return True
	return False
