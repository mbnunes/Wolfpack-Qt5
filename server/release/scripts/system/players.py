
import wolfpack

def onLogin( player ):
	socket = player.socket
	socket.sysmessage( "Welcome to %s" % ( wolfpack.serverversion() )  )
	socket.sysmessage( "Report Bugs: http://bugs.wpdev.org/" )
	player.hidden = 0
	player.update()
	socket.resendplayer()
	return True

def onLogout( player ):
	socket = player.socket
	player.hidden = 1
	player.removefromview()
	player.update()
	return True
