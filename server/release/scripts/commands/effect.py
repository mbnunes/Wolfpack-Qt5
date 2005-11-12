#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts
#  ((    _/{  "-;  | Created by: Dreoth
#   )).-' {{ ;'`   | Revised by: Naddel
#  ( (  ;._ \\ ctr | Last Modification: targeteffect command
#===============================================================#
# .effect/.targeteffect Command                                 #
#===============================================================#

import wolfpack
from wolfpack.utilities import hex2dec

def effect(socket, command, arguments):
	arguments = arguments.split(',')
	player = socket.player
	playeffect(player, arguments, None)

def targeteffect(socket, command, arguments):
	arguments = arguments.split(',')
	socket.attachtarget("commands.effect.target", arguments)

def target(socket, arguments, target):
	playeffect(socket, arguments, target)

def playeffect(char, arguments, target):
	# Default values
	id = 0
	speed = 20
	duration = 10
	hue = 0
	rendermode = 0
	
	socket = char.socket

	if len(arguments) == 1 and arguments[0] != '':
		(id) = arguments[0]
	elif len(arguments) == 2:
		(id, speed) = arguments
	elif len(arguments) == 3:
		(id, speed, duration) = arguments
	elif len(arguments) == 4:
		(id, speed, duration, hue) = arguments
	elif len(arguments) == 5:
		(id, speed, duration, hue, rendermode) = arguments
	else:
		socket.sysmessage('Usage: effect <effect-id>[, speed, duration, hue, rendermode]')
		return False
	try:
		effect = hex2dec(id)
		speed = int(speed)
		duration = int(duration)
		hue = int(hue)
		rendermode = int(rendermode)
		if not target:
			socket.player.effect(effect, speed, duration, hue, rendermode)
		else:
			if target.char:
				target.char.effect(effect, speed, duration, hue, rendermode)
			elif target.item:
				target.item.effect(effect, speed, duration, hue, rendermode)
			else:
				wolfpack.effect(effect, target.pos, duration, speed)
	except:
		socket.sysmessage('Usage: effect <effect-id>[, speed, duration, hue, rendermode]')

def onLoad():
	wolfpack.registercommand( "effect", effect )
	wolfpack.registercommand( "targeteffect", targeteffect )

"""
	\command effect
	\description Lets your character perform an effect visible to everyone, unless you're invisible.
	\usage - <code>effect id</code>
	- <code>effect id, speed, duration, hue, rendermode</code>

	Id is the effect you want your character to perform.
	Speed is how fast the effect is played (default 20).
	Duration is how long the effect is played (default 10).
	Hue is the color of the effect (default 0).
	Rendermode is something special. Values are unknown (default 0).
"""

"""
	\command targeteffect
	\description Lets you choose a target to perform an effect visible to everyone.
	\usage - <code>effect id</code>
	- <code>effect id, speed, duration, hue, rendermode</code>

	Id is the effect being shown.
	Speed is how fast the effect is played (default 20).
	Duration is how long the effect is played (default 10).
	Hue is the color of the effect (default 0).
	Rendermode is something special. Values are unknown (default 0).
"""
