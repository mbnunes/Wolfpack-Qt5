
import wolfpack
from wolfpack.consts import *

#
# Send a status string to the specified client.
#
def sendStatus(socket, status):
    packet = wolfpack.packet(0xEF, 4 + len(status))
    packet.setshort(1, 4 + len(status))
    packet.setascii(3, status)
    packet.send(socket)
    socket.log(LOG_MESSAGE, "Sent %u bytes of status information.\n" % packet.size)

#
# Handle an incoming 0xEF packet.
#
def handleStatus(socket, packet):
  if not packet.size == 3:
    socket.log(LOG_ERROR, "Invalid size for the 0xEF status packet: %u\n" % packet.size)
    socket.disconnect()
    return
  
  status = 'Wolfpack 12.9.4'
  sendStatus(socket, status)
  return 1

#
# Register our status handler for the 0xEF packet.
#
def onLoad():
  wolfpack.registerpackethook(0xEF, handleStatus)
