
import wolfpack

def onLogin( player ):
	socket = player.socket
	socket.sysmessage( "Welcome to Wolfpack %s" % ( wolfpack.serverversion() )  )
	socket.sysmessage( "Please refer to http://www.wpdev.org for bug reporting." )
	player.hidden = 0
	player.update()
	return True

def onLogout( player ):
	socket = player.socket
	player.hidden = 1
	player.removefromview()
	player.update()
	return True
