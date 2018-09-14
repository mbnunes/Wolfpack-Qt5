
import wolfpack
import wolfpack.time
from wolfpack import tr
from wolfpack.consts import LAYER_RIGHTHAND, LAYER_LEGS

#
# Remove the nightsight event and bonus
#
def wearout(player):
	if not player.hastag('nightsight'):
		bonus = 0
	else:
		bonus = int(player.gettag('nightsight'))

	found = False
	for i in range(LAYER_RIGHTHAND, LAYER_LEGS):
		if player.itemonlayer(i) and player.itemonlayer(i).hastag('nightsight'):
			found = True
			break
	if not found:
		player.deltag('nightsight')
		player.lightbonus = max(0, player.lightbonus - bonus)
	player.removescript('magic.nightsight')
	player.deltag('nightsight_start')

	if player.socket:
		player.socket.updatelightlevel()

#
# Wear out the nightsight
#
def onLogout(player):
	wearout(player)
	return False

#
# Wear out the nightsight
#
def onLogin(player):
	wearout(player)
	return False

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
			player.socket.sysmessage(tr('Your nightsight is wearing out.'))
		wearout(player)
