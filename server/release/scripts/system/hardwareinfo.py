
import wolfpack

#
# Handle an incoming 0xD9 packet.
#
def handleInfo(socket, packet):
  return 1

#
# Register the hook.
#
def onLoad():
  wolfpack.registerpackethook(0xD9, handleInfo)
