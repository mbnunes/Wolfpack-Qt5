import wolfpack.settings
import wolfpack.time
#from wolfpack.utilities import isyoung

#def onCheckVictim(npc, victim, dist):
#	if not npc.owner or not npc.summoned:
#		if CheckYoungProtection(victim):
#			return False

def CheckYoungProtection(char):
	if not isyoung(char):
		return False
	if char.region and char.region.cave:
		return False

	if char.socket.hastag( 'lastyoungmessage' ):
		if wolfpack.time.currenttime() < char.socket.gettag( 'lastyoungmessage' ):
			return False
	char.socket.settag( 'lastyoungmessage', int( wolfpack.time.currenttime() + 60000 ) )
	char.socket.clilocmessage( 1019067 ) # A monster looks at you menacingly but does not attack.  You would be under attack now if not for your status as a new citizen of Britannia.
	return True
