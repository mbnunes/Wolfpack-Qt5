
import wolfpack
from wolfpack import tr
from potions.utilities import *
from potions.consts import *

#
# Shrink a char in range
#
def potion( player, arguments, target ):
	potion = wolfpack.finditem(arguments[0])
	
	if not potion or not player.canreach( potion, -1 ):
		player.socket.sysmessage( tr( 'The shrink potion has to be in your backpack to use it.' ) )
		return False

	if not canUsePotion( char, potion ):
		return False

	if not target.char:
		player.socket.sysmessage(tr('You can only shrink pets owned by you.'))
		return False

	if not player.canreach(target.char, 1):
		player.socket.clilocmessage(500312)
		return False

	if target.char.player:
		player.socket.sysmessage(tr('You cannot shrink other players.'))
		return False

	if target.char.owner != player:
		player.socket.sysmessage(tr("You don't own that creature."))
		return False

	if target.char.id in [0x190, 0x191, 0x192, 0x193, 0x3db]:
		player.socket.sysmessage(tr('You can only shrink animals and monsters!'))
		return False

	bodyinfo = wolfpack.bodyinfo(target.char.id)

	if bodyinfo['figurine'] <= 0 or bodyinfo['figurine'] >= 0x4000:
		player.socket.sysmessage(tr('You cannot shrink that.'))
		return False

	target.char.sound(SND_IDLE)

	# Create a new figurine and make it newbie
	figurine = wolfpack.additem('%x' % bodyinfo['figurine'])
	figurine.newbie = True
	figurine.addscript('figurine')
	figurine.settag('pet', target.char.serial)
	figurine.color = target.char.skin
	figurine.name = target.char.name
	figurine.update()
	player.getbackpack().additem(figurine, True, False) # Random pos, no auto stacking
	figurine.update()

	target.char.removefromview()
	target.char.owner = None
	target.char.stablemaster = figurine.serial
	target.char.addscript('figurine') # This is a figurined NPC

	consumePotion(player, potion)
	return True
