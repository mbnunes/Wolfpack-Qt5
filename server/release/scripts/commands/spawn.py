
import wolfpack
from wolfpack import tr
from wolfpack.gumps import WarningGump

"""
	\command spawn
	\description Adds a spawngem with the given parameters at the target location.
	\usage - <code>spawn npc range mintime maxtime</code>
	Npc is the definition id of the npc to spawn.
	Range is the wander range for the spawned npc.
	Mintime is the minimum interval in minutes between spawns.
	Maxtime is the maximum interval in minutes between spawns.
"""

def addSpawn(player, ok, args):
	if not ok:
		return
	
	(pos, npc, radius, mintime, maxtime) = args
	
	player.socket.sysmessage(tr('Adding %s spawn at %s.') % (npc, pos))
	
	item = wolfpack.additem('spawngem')
	
	# Set tag info
	item.settag('spawntype', 1)
	item.settag('spawndef', npc)
	item.settag('area', radius)
	item.settag('mininterval', mintime)	
	item.settag('maxinterval', maxtime)	
	item.moveto(pos)
	item.name = tr('a %s spawn') % npc
	item.update()

def callback(player, arguments, target):
	pos = None
	
	(npc, radius, mintime, maxtime) = arguments
	
	if target.item and not target.item.container:
		pos = target.item.pos
	elif target.char:
		pos = target.char.pos
	else:
		pos = target.pos
		
	args = [pos, npc, radius, mintime, maxtime]
		
	# Check Distance. If more than 30 tiles away, warn the user
	if player.distanceto(pos) > 30:
		gump = WarningGump( 1060635, 30720, tr("The position (%s) you targetted is more than 30 tiles away. Are you sure to add a spawn there?") % pos, 0xFFFFFF, 420, 250, addSpawn, args )
		gump.send( player.socket )	
	else:
		addSpawn(player, True, args)

#
# Usage: .spawn <creature> <radius> <mintime> <maxtime>
#
def spawn(socket, command, arguments):
	arguments = arguments.split(' ')
	
	if len(arguments) != 4:
		socket.sysmessage(tr('Usage: spawn npc radius mintime maxtime'))
		return
		
	(npc, radius, mintime, maxtime) = arguments
	try:
		radius = int(radius)
		mintime = int(mintime)
		maxtime = int(maxtime)
	except:
		socket.sysmessage(tr('Usage: npc spawn radius mintime maxtime'))
		return
		
	socket.sysmessage(tr('Where do you want to spawn npc %s.') % npc)
	socket.attachtarget("commands.spawn.callback", [npc, radius, mintime, maxtime])

def onLoad():
	wolfpack.registercommand('spawn', spawn)
