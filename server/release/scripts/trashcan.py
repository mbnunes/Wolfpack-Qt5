
import wolfpack
import wolfpack.time
import wolfpack.utilities

# Two minutes
delete_time = 120000

def onDropOnItem( target, item ):
	if target.baseid == 'trashcan':
		target.settag( 'trash_timer', int( wolfpack.time.currenttime() + delete_time ) )
		target.addtimer( delete_time, timerPass, [] )
	return False

def timerPass( trashcan, args ):
	if not trashcan or not trashcan.baseid != "trashcan":
		return False

	if trashcan.hastag( 'trash_timer' ) and trashcan.gettag( 'trash_timer' ) <= wolfpack.time.currenttime():
		trashlist = wolfpack.utilities.itemsincontainer( trashcan.serial )
		if len( trashlist ) == 0:
			return False
		i = 0
		for serial in trashlist:
			trash = wolfpack.finditem( serial )
			trash.delete()
			i += 1
		trashcan.deltag( 'trash_timer' )
		return True
	return False

def onUse( char, item ):
	if item.type != 1 or item.baseid != 'trashcan':
		return False
	item.settag( 'trash_timer', int( wolfpack.time.currenttime() + delete_time ) )
	wolfpack.addtimer( delete_time, timerPass, [ item.serial ] )
	return False
