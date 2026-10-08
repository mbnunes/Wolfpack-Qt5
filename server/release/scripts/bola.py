
import wolfpack
from wolfpack import tr

def mounted( char ):
	if char.itemonlayer( 25 ):
		return True
	return False

def onUse( char, item ):
	if not item.getoutmostitem() == char.getbackpack():
		char.socket.clilocmessage( 1040019 ) # The bola must be in your pack to use it.
	#elif char.itemonlayer( 1 ) or char.itemonlayer( 2 ):
	#	char.socket.clilocmessage( 1040015 ) # Your hands must be free to use this
	elif mounted( char ):
		char.socket.clilocmessage( 1040016 ) # You cannot use this while riding a mount
	else:
		one = char.itemonlayer( 1 )
		two = char.itemonlayer( 2 )

		if one:
			char.getbackpack().additem( one )
		if two:
			char.getbackpack().additem( two )

		char.message( 1049632, "" ) # * You begin to swing the bola...*
		chars = wolfpack.chars( char.pos.x, char.pos.y, char.pos.map, 16 )
		for player in chars:
			if player != char and player.cansee(char):
				player.message( 1049633, str(char.name) ) # ~1_NAME~ begins to menacingly swing a bola...
		char.socket.attachtarget( "bola.target", [item.serial] )
	return True

def target( char, args, target ):
	bola = wolfpack.finditem( args[0] )
	if not bola:
		return False

	if target.char:
		if not mounted( target.char ):
			char.socket.clilocmessage( 1049628 ) # You have no reason to throw a bola at that.
		else:
			char.reveal()
			bola.delete()
			char.turnto( target.char )
			char.action( 11 )
			char.movingeffect( 0x26ac, target.char, 1, 0, 1, 0, 1 )
			char.addtimer( 500, delay, [target.char] )
	else:
		char.socket.clilocmessage( 1049629 ) # You cannot throw a bola at that.
	return True

def delay( char, args ):
	target = wolfpack.findchar( args[0] )
	if not target:
		return False
	new_bola = wolfpack.additem( "26ac" )
	new_bola.moveto( target.pos )
	new_bola.update()

	target.damage( 1, 1, char )
	target.unmount()
	if target.socket:
		target.socket.clilocmessage( 1040023 ) # You have been knocked off of your mount!
	return True
