#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: Correa                         #
#   )).-" {{ ;"`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

#
# This script handles UOGateway shard status requests
#


import wolfpack
from wolfpack.consts import *

#
# The packet we expect to receive from a status requesting client.
#
statusRequest = [
    # Header (1 byte)
    0x80, 

    # Username (30 byte)
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 

    # Password (30 byte)
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 
    
    # Unknown
    0x00 ]

#
# Handle an incoming 0x80 packet.
#
def handleStatus(socket, packet):
    statusPacket = 1
    for i in range( 0, packet.size ):
        if (packet.getbyte(i) != statusRequest[i]):
            statusPacket = 0
            break

    if statusPacket:
        status = 'Wolfpack, Clients=%i, Items=%i, Chars=%i, Version=%s' % (wolfpack.sockets.count(), wolfpack.itemcount(), wolfpack.charcount(), wolfpack.serverversion())

        packet = wolfpack.packet(87, len(status) + 1)
        packet.setascii(1, status)
        packet.send(socket)
        socket.log(LOG_MESSAGE, "Sent status information.\n")

    return statusPacket

def onLoad():
    wolfpack.registerpackethook(0x80, handleStatus)
