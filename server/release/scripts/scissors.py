#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: DarkStorm / Nacor              #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

# Scissors
import wolfpack
import wolfpack.utilities

ids_rawleather = [ 0x1078, 0x1079 ]
ids_clothes_2 = [ 0x153f, 0x1540, 0x1543, 0x1544 ]
ids_clothes_4 = [ 0x1541, 0x1542 ]
ids_clothes_6 = [ 0x152e, 0x152f, 0x153b, 0x153c ]
ids_clothes_8 = [ 0x1517, 0x1518, 0x1537, 0x1538, 0x1539, 0x153a, 0x1efd, 0x1f7b, 0x1f7c, 0x1efe ]
ids_clothes_10 = [ 0x1516, 0x1531, 0x153d, 0x153e, 0x1717, 0x1f01, 0x1f02 ]
ids_clothes_11 = [ 0x1713, 0x1715, 0x1719 ]
ids_clothes_12 = [ 0x1714, 0x171a, 0x171b, 0x1eff, 0x1f00, 0x1fa1, 0x1fa2 ]
ids_clothes_13 = [ 0x1716 ]
ids_clothes_14 = [ 0x1515, 0x1530, 0x1ffd, 0x1ffe ]
ids_clothes_15 = [ 0x1718, 0x171c, 0x172e ]
ids_clothes_16 = [ 0x1f03, 0x1f04 ]
ids_clothes_24 = [ 0x1f9f, 0x1fa0 ]
ids_bolts = [ 0x0f95, 0xf96, 0x0f97, 0xf98, 0x0f99, 0x0f9a, 0x0f9b, 0x0f9c ]
ids_clothes = [ 0x1765, 0x1766, 0x1767, 0x1768 ]


def onUse( char, item ):
	# Needs to be on ourself
	if item.getoutmostchar() != char:
		char.socket.clilocmessage( 0x7A258 ) # You can't reach...
		return 1

	char.socket.clilocmessage( 0x7AAA2 ) # What do you want to use this scissors on?
	char.socket.attachtarget( "scissors.response", [ item.serial ] )
	return 1


def response( char, args, target ):
	# 2440: Scissors can not be used on that to produce anything
	# 2437: The item you wish to cut must be in your backpack	

	item = wolfpack.finditem( args[0] )

	if not item: 
		char.socket.clilocmessage( 0x7A258 ) # You can't reach...
		return
	
	if not ( item.getoutmostchar() == char):
		char.socket.clilocmessage( 0x7AAA5 ) # You can't reach...
		return 

	# Check target (only item targets valid)
	if not target.item:
		char.socket.clilocmessage( 0x7AAA8, "", 0x3b2, 3, char ) # Scissors can not be used on that to produce anything
		return
    
	if not target.item.getoutmostchar() == char:
		char.socket.clilocmessage( 0x7A4FC )
		return 

	if target.item.id in ids_rawleather:
		char.soundeffect( 0x248 )
		item_new = wolfpack.additem( "1067" )
		item_new.amount = target.item.amount
		target.item.delete()
		if not wolfpack.utilities.tocontainer( item_new, char.getbackpack() ):
			item_new.update()


	elif target.item.id in ids_clothes_2:
		char.soundeffect( 0x248 )
		item_new = wolfpack.additem( "ee9" )
		item_new.amount = 2
		target.item.delete()
		if not wolfpack.utilities.tocontainer( item_new, char.getbackpack() ):
			item_new.update()


	elif target.item.id in ids_clothes_4:
		char.soundeffect( 0x248 )
		item_new = wolfpack.additem( "ee9" )
		item_new.amount = 4
		target.item.delete()
		if not wolfpack.utilities.tocontainer( item_new, char.getbackpack() ):
			item_new.update()


	elif target.item.id in ids_clothes_6:
		char.soundeffect( 0x248 )
		item_new = wolfpack.additem( "ee9" )
		item_new.amount = 6
		target.item.delete()
		if not wolfpack.utilities.tocontainer( item_new, char.getbackpack() ):
			item_new.update()


	elif target.item.id in ids_clothes_8:
		char.soundeffect( 0x248 )
		item_new = wolfpack.additem( "ee9" )
		item_new.amount = 8
		target.item.delete()
		if not wolfpack.utilities.tocontainer( item_new, char.getbackpack() ):
			item_new.update()


	elif target.item.id in ids_clothes_10:
		char.soundeffect( 0x248 )
		item_new = wolfpack.additem( "ee9" )
		item_new.amount = 10
		target.item.delete()
		if not wolfpack.utilities.tocontainer( item_new, char.getbackpack() ):
			item_new.update()


	elif target.item.id in ids_clothes_11:
		char.soundeffect( 0x248 )
		item_new = wolfpack.additem( "ee9" )
		item_new.amount = 11
		target.item.delete()
		if not wolfpack.utilities.tocontainer( item_new, char.getbackpack() ):
			item_new.update()


	elif target.item.id in ids_clothes_12:
		char.soundeffect( 0x248 )
		item_new = wolfpack.additem( "ee9" )
		item_new.amount = 12
		target.item.delete()
		if not wolfpack.utilities.tocontainer( item_new, char.getbackpack() ):
			item_new.update()


	elif target.item.id in ids_clothes_13:
		char.soundeffect( 0x248 )
		item_new = wolfpack.additem( "ee9" )
		item_new.amount = 13
		target.item.delete()
		if not wolfpack.utilities.tocontainer( item_new, char.getbackpack() ):
			item_new.update()


	elif target.item.id in ids_clothes_14:
		char.soundeffect( 0x248 )
		item_new = wolfpack.additem( "ee9" )
		item_new.amount = 14
		target.item.delete()
		if not wolfpack.utilities.tocontainer( item_new, char.getbackpack() ):
			item_new.update()


	elif target.item.id in ids_clothes_15:
		char.soundeffect( 0x248 )
		item_new = wolfpack.additem( "ee9" )
		item_new.amount = 15
		target.item.delete()
		if not wolfpack.utilities.tocontainer( item_new, char.getbackpack() ):
			item_new.update()


	elif target.item.id in ids_clothes_16:
		char.soundeffect( 0x248 )
		item_new = wolfpack.additem( "ee9" )
		item_new.amount = 16
		target.item.delete()
		if not wolfpack.utilities.tocontainer( item_new, char.getbackpack() ):
			item_new.update()


	elif target.item.id in ids_clothes_24:
		char.soundeffect( 0x248 )
		item_new = wolfpack.additem( "ee9" )
		item_new.amount = 24
		target.item.delete()
		if not wolfpack.utilities.tocontainer( item_new, char.getbackpack() ):
			item_new.update()


	elif target.item.id in ids_bolts:
		char.soundeffect( 0x248 )
		item_new = wolfpack.additem( "1765" )
		item_new.amount = ( target.item.amount * 5 )
		target.item.delete()
		if not wolfpack.utilities.tocontainer( item_new, char.getbackpack() ):
			item_new.update()

	elif target.item.id in ids_clothes:
		char.soundeffect( 0x248 )
		if ( target.item.amount > 1 ):
			target.item.amount = target.item.amount -1
			item.update()
		else:
			target.item.delete()

		item_new = wolfpack.additem( "ee9" )
		item_new.amount = 10
		if not wolfpack.utilities.tocontainer( item_new, char.getbackpack() ):
			item_new.update()
	else:
		char.socket.clilocmessage( 0x7AAA8, "", 0x3b2, 3, char ) # Scissors can not be used on that to produce anything
		return
