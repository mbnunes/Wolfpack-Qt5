
"""
	\command tile
	\description Fills a rectangle with items.
	\usage - <code>tile z ids</code>
	Z is the height where the items should be created.
	Ids is a list of definition ids separated by commas. 
	Every item will be created using an id randomly selected 
	from that list.
"""

import wolfpack
import random

#
# Gather another corner if neccesary and start tiling
#
def tileResponse(player, arguments, target):
  if len(arguments) < 3:
    player.socket.sysmessage('Please select the second corner.')
    player.socket.attachtarget("commands.tile.tileResponse", list(arguments) + [target.pos])
    return
  
  x1 = min(arguments[2].x, target.pos.x)
  x2 = max(arguments[2].x, target.pos.x)
  y1 = min(arguments[2].y, target.pos.y)
  y2 = max(arguments[2].y, target.pos.y)
  z = arguments[0]
  ids = arguments[1]

  player.socket.sysmessage('Creating items from %u,%u to %u,%u at z=%d.' % (x1, y1, x2, y2, z))
  pos = player.pos
  pos.z = z

  for x in range(x1, x2 + 1):
    for y in range(y1, y2 + 1):
      pos.x = x
      pos.y = y
      item = wolfpack.additem(random.choice(ids))
      item.moveto(pos)
      item.decay = 0
      item.update()

#
# Gather arguments and validate id list.
#
def commandTile(socket, command, arguments):
  if arguments.count(' ') != 1:
    socket.sysmessage('Usage: .tile z id{,id}')
    return

  (z, ids) = arguments.split(' ')
  
  ids = ids.split(',')

  try:
    z = int(z)
  except:
    socket.sysmessage('Invalid z value.')
    return

  socket.sysmessage('Please select the first corner.')
  socket.attachtarget('commands.tile.tileResponse', [z, ids])

#
# Register the command
#
def onLoad():
  wolfpack.registercommand("tile", commandTile)
