
import wolfpack
import random
import wolfpack.time

def onTimeChange( player ):
	if wolfpack.time.hour() in [ 0, 3, 6, 9, 12, 15, 18, 21 ]:
		if player.socket:
			if player.hunger >= 1 and player.hunger <= 6:
				player.hunger -= 1
				player.socket.sysmessage( "Your stomach growls..." )
			elif player.hunger == 0:
				player.damage( 3, random.randint( 0, 3 ) )
				player.socket.sysmessage( "Your stomach hurts from the lack of food..." )
	return False
