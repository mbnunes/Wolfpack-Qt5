
import wolfpack
import wolfpack.time
import wolfpack.utilities

# Two minutes
delete_time = 120000

def onDropOnItem( target, item ):
	if target.baseid == 'trashcan':
		target.settag( 'trash_timer', int( wolfpack.time.currenttime() + delete_time ) )
		wolfpack.addtimer( delete_time, "trashcan.timerPass", [ target ] )
		return False
	else:
		return False
	return False

def timerPass( timer, args ):
	trashcan = args[0]
	if trashcan.baseid != "trashcan":
		return False

	if trashcan.hastag( 'trash_timer' ) and trashcan.gettag( 'trash_timer' ) <= wolfpack.time.currenttime():
		trashlist = wolfpack.utilities.itemsincontainer( trashcan.serial )
		if len( trashlist ) == 0:
			trashcan.say( "*hungry*" )
			return False
		i = 0
		for serial in trashlist:
			trash = wolfpack.finditem( serial )
			trash.delete()
			i += 1
		trashcan.say( "Deleted %i items!" % i )
		trashcan.deltag( 'trash_timer' )
		return True
	return False

def onUse( char, item ):
	if item.type != 1 or item.baseid != 'trashcan':
		return False
	item.settag( 'trash_timer', int( wolfpack.time.currenttime() + delete_time ) )
	wolfpack.addtimer( delete_time, "trashcan.timerPass", [ item ] )
	return False
