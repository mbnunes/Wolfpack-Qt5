#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: Nacor                          #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

# Cooking
import wolfpack
import wolfpack.utilities

ids_heat =[ 0xde3 ]
ids_meat =[ 0x9b7, 0x9b8, 0x9f2, 0x1608 ]
ids_fish =[ 0x97b ]
ids_pastries =[ 0x160c, 0x9e9, 0x1041, 0x1040 ]

ids = {
	# raw bird : cooked bird
	0x9b9 : 0x9b7,
	0x9ba : 0x9b8,
	
	# raw fish : cooked fish
	0x97a : 0x97b,
	
	# raw rib : cooked rib
	0x9f1 : 0x9f2,
	
	# dough : bread
	0x103d : 0x103b,
	
	# cookie mix : cookie
	0x103f : 0x160c,
	
	# cakie mix : cake
	0xa1e : 0x9e9,
	
	# raw chickenleg : cooked chickenleg
	0x1607 : 0x1608,
	
	# raw lamleg : cooked lamleg
	0x1609 : 0x160a,
	
	# unbaked pi : baked pie
	0x1042 : 0x1041,
	
	# uncooked pizza : pizza
	0x1083 : 0x1040
	
	}


def onUse( char, item ):
	# Needs to be on ourself
	if item.getoutmostchar() != char:
		char.socket.clilocmessage( 0, 500, 312 ) # You can't reach...
		return 1
	
	if ids.has_key( item.id ):
		char.socket.clilocmessage( 0, 500, 242 )
		char.socket.attachtarget( "cooking.response", [ item.serial ] )
		return 1

def response( char, args, target ):
	direction = char.directionto( target.pos )
	if not char.direction == direction:
		char.direction = direction
		char.update()
	item = wolfpack.finditem( args[0] )
	
	if ( ( char.pos.x-target.pos.x )**2 + ( char.pos.y-target.pos.y )**2 > 4):
		char.socket.clilocmessage( 0, 500, 295 )
		return 1
		
	if abs( char.pos.z - target.pos.z ) > 5:
		char.socket.clilocmessage( 0, 500, 295 )
		return 1
	
	# Check target (only item targets valid)
	if not target.item:
		char.socket.sysmessage( "You can't cook on this." )
		return 1

	if target.item.id in ids_heat:
		# Change the id
		newid = ids[ item.id ]
		item.id = newid
	
	if item.newid in ids_meat:
		item.type2 = 8
		item.update()
	
	elif item.newid in ids_fish:
		item.type2 = 9
		item.update()
		
	elif item.newid in ids_pastries:
		item.type2 = 10
		item.update()
	
	else:
		char.socket.sysmessage( "You can't cook on this." )
		return
	