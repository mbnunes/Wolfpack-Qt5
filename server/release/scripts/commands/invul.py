
import wolfpack


"""
	\command invul
	\description Toggle or change your invulnerability flag.
	\usage - <code>invul</code>
	- <code>invul on</code>
	- <code>invul true</code>
	- <code>invul 1</code>
	- <code>invul off</code>
	- <code>invul false</code>
	- <code>invul 0</code>	
	If no argument is given, the flag is toggled.
"""

def invul(socket, command, arguments):
	arguments = arguments.lower()
	if len(arguments) and arguments == '1' or arguments == 'on' or arguments == 'true':
		socket.player.invulnerable = 1
	elif len(arguments) and arguments == '0' or arguments == 'off' or arguments == 'false':
		socket.player.invulnerable = 0
	else:
		socket.player.invulnerable = not socket.player.invulnerable
	socket.sysmessage("'invul' is now '%u'" % socket.player.invulnerable)	
	
def onLoad():
	wolfpack.registercommand('invul', invul)
