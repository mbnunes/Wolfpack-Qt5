
import wolfpack
from wolfpack import tr

def onLogin( player ):
	socket = player.socket
	socket.sysmessage( tr("Welcome to %s") % ( wolfpack.serverversion() )  )
	socket.sysmessage( tr("Report Bugs: http://bugs.wpdev.org/") )
	player.hidden = False
	player.update()
	socket.resendplayer()

def onLogout( player ):
	socket = player.socket
	player.removefromview()
	player.hidden = 1
	player.update()

def onDamage(char, type, amount, source):
	socket = char.socket
	
	if socket and amount > 25 and socket.hastag('bandage_slipped'):
		socket.settag('bandage_slipped', int(socket.gettag('bandage_slipped')) + 1)
		socket.clilocmessage(500961)
	
	return amount
