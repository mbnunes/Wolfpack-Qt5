
import wolfpack
import wolfpack.time

#
# Remove the nightsight event and bonus
#
def wearout(player):
	if not player.hastag('nightsight'):
		bonus = 0
	else:
		bonus = int(player.gettag('nightsight'))

	player.lightbonus = max(0, player.lightbonus - bonus)

	player.removescript('magic.nightsight')
	player.deltag('nightsight')
	player.deltag('nightsight_start')

	if player.socket:
		player.socket.updatelightlevel()

#
# Wear out the nightsight
#
def onLogout(player):
	wearout(player)

#
# Wear out the nightsight
#
def onLogin(player):
	wearout(player)

#
# See if the nightsight expired
#
def onTimeChange(player):
	if not player.hastag('nightsight_start'):
		nightsight_start = 0
	else:
		nightsight_start = int(player.gettag('nightsight_start'))

	# Nightsight lasts 1440 ingame minutes
	if nightsight_start + 1440 < wolfpack.time.minutes():
		if player.socket:
			player.socket.sysmessage('Your nightsight is wearing out.')
		wearout(player)
