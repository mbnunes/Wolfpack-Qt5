import wolfpack
from wolfpack import console, tr
from wolfpack.consts import SND_IDLE, ANIM_CASTDIRECTED

#
# Unstable the NPC, re-tame it and
# make it visible.
#
def onUse(player, item):
	if not item.getoutmostchar() == player:
		player.socket.sysmessage('The figurine needs to be in your posession to use it.')
		return 1

	if not item.hastag('pet'):
		item.delete()
		return 1

	try:
		pet = wolfpack.findchar(int(item.gettag('pet')))

		if not pet:
			item.deltag('pet')
			item.delete()
			return 1
	except:
		item.deltag('pet')
		item.delete()
		return 1

	# Remove pet reference and then delete
	item.deltag('pet')
	item.delete()

	pet.removescript('figurine')
	pet.owner = player
	pet.stablemaster = 0
	pet.wandertype = 0 # Stay where you are...
	pet.direction = player.direction
	pet.moveto(player.pos)
	pet.update()
	pet.sound(SND_IDLE)
	pet.action(ANIM_CASTDIRECTED)
	return 1

#
# This event is also applied to figurined npcs so
# the figurine can be removed along with the npc.
# Or remove the npc if the figurine is deleted.
# WARNING: Avoid circular reference.
#
def onDelete(object):
	if object.isitem():
		item = object
		if item.hastag('pet'):
			try:
				pet = wolfpack.findchar(int(item.gettag('pet')))
				if pet and pet.stablemaster != -1:
					item.free = 1
					pet.delete()
			except:
				pass
		return

	char = object
	if not char.npc:
		return

	figurine = wolfpack.finditem(char.stablemaster)
	if figurine:
		char.free = 1
		figurine.delete()

#
# This command creates a figurine out of the
# targetted npc.
#
def shrink(socket, command, arguments):
	socket.sysmessage( 'What do you want to shrink?' )
	socket.attachtarget( 'figurine.shrinktarget', [] )
	#pass
	
def shrinktarget( char, args, target ):
	if not target.char:
		char.socket.sysmessage(tr('You can only shrink characters.'))
		return
	if target.char.player:
		char.socket.sysmessage(tr('You cannot shrink other players.'))
		return
		
	bodyinfo = wolfpack.bodyinfo(target.char.id)
		
	if bodyinfo['figurine'] <= 0 or bodyinfo['figurine'] >= 0x4000:
		char.socket.sysmessage(tr('You cannot shrink that.'))
		return
			
	target.char.sound(SND_IDLE)

	# Create a new figurine and make it newbie
	figurine = wolfpack.additem('%x' % bodyinfo['figurine'])
	figurine.newbie = True
	figurine.addscript('figurine')
	figurine.settag('pet', target.char.serial)
	figurine.color = target.char.skin
	figurine.name = target.char.name
	figurine.update()
	figurine.moveto(target.char.pos)
	#char.getbackpack().additem(figurine, True, False) # Random pos, no auto stacking
	figurine.update()

	target.char.removefromview()
	target.char.owner = None
	target.char.stablemaster = figurine.serial
	target.char.addscript('figurine') # This is a figurined NPC


#
# Register the shrink command.
#
def onLoad():
	wolfpack.registercommand('shrink', shrink)
	
"""
	\command shrink
	\description With this command you can shrink a NPC.
	\usage - <code>shrink</code>
	 A figurine will be created at the point you shrinked it.
	\notes This command cannot be used on players. 
"""
