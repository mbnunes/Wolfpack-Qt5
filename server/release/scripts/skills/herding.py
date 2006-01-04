# Herding
import wolfpack
from wolfpack.consts import GRAY, HERDING, SND_ATTACK

def onUse( char, item ):
	# Needs to be on ourself
	if item.getoutmostchar() != char:
		char.socket.clilocmessage( 500312, '', GRAY ) # You cannot reach that.
		return False
	char.socket.clilocmessage( 502464 ) # Target the animal you wish to herd.
	char.socket.attachtarget( "skills.herding.response", [] )
	return True

def response( char, args, target ):
	if target.char:
		if not char.canreach(target.char, 10):
			return False
		# animal
		if target.char.bodytype == 3:
			char.socket.clilocmessage( 502475 ) # Click where you wish the animal to go.
			char.socket.attachtarget( "skills.herding.gothere", [ target.char.serial ] )
		else:
			char.socket.clilocmessage( 502468 ) # That is not a herdable animal.
			return False
	else:
		char.socket.clilocmessage( 502472 ) # You don't seem to be able to persuade that to move.
		return False
	return True

def gothere(char, args, target):
	pet = wolfpack.findchar(args[0])
	if not pet:
		return False
	if not char.canreach(target.pos, 10):
		return False
	if not char.checkskill(HERDING, 0, 1000):
		char.socket.clilocmessage( 502472 ) # You don't seem to be able to persuade that to move.
		return False

	char.socket.clilocmessage( 502479 ) # The animal walks where it was instructed to.
	pet.goto(target.pos)
	pet.sound(SND_ATTACK)
	return
