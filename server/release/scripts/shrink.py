#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: Abassador Kosh                 #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

# Shrink
import wolfpack

def onUse( char, item ):
	if not item.hastag( "npc_id" ):
		char.socket.clilocmessage( 0, 500, 312 ) # You can't reach...
		item.delete()
		return 1

	# Needs to be on ourself
	if item.getoutmostchar() != char:
		char.socket.clilocmessage( 0, 500, 312 ) # You can't reach...
		return 1

	npcid = item.gettag( "npc_id" )
	npctype = item.gettag( "npc_type" )
	pos = char.pos
	npc = wolfpack.addnpc( npcid, pos )
	if npctype == 'mount':
		char.mount(npc)
		item.delete()
	elif npctype == 'follow':
		npc.tamed = 1
		npc.npcwander = 0
		npc.follow(char)
		item.delete()
	else:
		item.delete()
	return 1
