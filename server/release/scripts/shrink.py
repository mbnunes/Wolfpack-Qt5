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
	pos = char.pos
	npcmount = wolfpack.addnpc( npcid, pos )
	char.mount( npcmount )
	item.delete()

	return 1