
import wolfpack.time

def onRegenMana(char, time):
	#new = wolfpack.time.currenttime()
	char.socket.sysmessage(str(time))
	points = 3
	time_new = int((time + ( 0.1 * points )) )
	char.socket.sysmessage(str(time_new))
	#char.socket.sysmessage(str(new))
	#char.socket.sysmessage(str(char.regenmana))
	#char.socket.sysmessage(str(int(1.0 / ( 0.1 * char.regenmana))))
	#time -= points
	#char.socket.sysmessage("neu " + str(time))
	return time

def dispel(char, args, source, dispelargs):
	char.removescript('magic.lichform')

def expire(char, args):
	char.hitpoints -= 1
	char.updatehealth()
	char.addtimer( 2500, expire, [], True, False, 'LICHFORM', dispel )
