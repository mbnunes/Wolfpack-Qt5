
import wolfpack

#
# Handle an incoming 0x80 packet.
#
def handleInfo(socket, packet):
  return 1

def onLoad():
  wolfpack.registerpackethook(0xD9, handleInfo)
