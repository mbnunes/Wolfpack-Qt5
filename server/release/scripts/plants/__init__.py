
import wolfpack

#
# Helper Function
#
def sendCodexOfWisdom(socket, topic, display = True):
	packet = wolfpack.packet(0xbf, 11)
	packet.setshort(1, 11)
	packet.setshort(3, 0x17) # SubCommand
	packet.setbyte(5, 1) # Unknown
	packet.setint(6, int(topic)) # The Topic ID
	packet.setbyte(10, int(display)) # Should the topic be displayed?
	packet.send(socket)
