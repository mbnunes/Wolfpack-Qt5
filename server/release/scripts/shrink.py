#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: Abassador Kosh                 #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

# Shrink
import wolfpack
from wolfpack.consts import *

def onUse( char, item ):
	if not item.hastag( "npc_id" ):
		item.delete()
		return 1

	# Needs to be on ourself
	if item.getoutmostchar() != char:
		char.socket.clilocmessage( 500312 ) # You can't reach...
		return 1

	npcid = item.gettag("npc_id")
	npc = wolfpack.addnpc(npcid, char.pos)
	
	if npc:
		if item.hastag('npc_type') and char.gettag('npc_type') == 'mount':
			char.mount(npc)
		else:
			npc.owner = char
			npc.tamed = True
			npc.wandertype = 0
			npc.direction = char.direction
			npc.update()
						
			npc.sound(SND_IDLE)
			npc.action(ANIM_CASTDIRECTED)
		
	item.delete()	
	return True

