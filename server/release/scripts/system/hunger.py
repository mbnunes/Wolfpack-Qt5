
import wolfpack
import random
import wolfpack.time

def onTimeChange( player ):
	hungerrate = int(wolfpack.settings.getstring("Game Speed", "Hunger Rate", "2"))
	hungerdamage = int(wolfpack.settings.getstring("General", "Hunger Damage", "0"))

	if wolfpack.time.hour() in range( hungerrate, 23, hungerrate ):
		if player.socket:
			if player.hunger >= 1 and player.hunger <= 6:
				player.hunger -= 1
				player.socket.sysmessage( "Your stomach growls..." )
			elif player.hunger == 0:
				player.damage( 3, random.randint( 0, hungerdamage ) )
				player.socket.sysmessage( "Your stomach hurts from the lack of food..." )
	return False
