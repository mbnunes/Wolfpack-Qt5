
import wolfpack
import wolfpack.time

def onTimeChange( player ):
	if player.socket:
		if player.hunger >= 1 and player.hunger <= 6:
			player.hunger -= 1
			player.socket.sysmessage( "Your stomach growls..." )
		elif player.hunger == 0:
			player.hitpoints -= 1
			player.update()
			player.socket.sysmessage( "Your stomach hurts from the lack of food..." )
		return True
	else:
		return False
