"""
	\
	\
	\command removedupes
	\description removes all (but one) duplicate items from all maps:
	\
	\ a duplicate is an item that is in the same location (x,y,z,map) and has the same ID#
	\ as another.
	\
	\ NOTE: this *may* delete things you would rather keep.  this routine is intended to 
	\       clean up after an 'import or 'decoration command is issued on a map in error.
	\       
	\		this routines does NOT know if an item has had the 'dupe command issued on it
	\       nor does it detect items duped on the account of exploits or other such nonsense.
	\
	\
	\ DOUBLE NOTE: you should consider disabling this command once your shard is "live".  
	\
	\					
"""

import wolfpack

MAP0_WIDTH = 6144
MAP0_HEIGHT = 4096
MAP1_WIDTH = 6144
MAP1_HEIGHT = 4096
MAP2_WIDTH = 2304
MAP2_HEIGHT = 1600
MAP3_WIDTH = 2560
MAP3_HEIGHT = 2048


def onLoad():
	wolfpack.registercommand( "removedupes", commandRemoveDupes )

def commandRemoveDupes(socket, cmd, args):
	socket.sysmessage( "Removing Dupes From Map 0, please wait." )
	removeDupesFromMap( socket, 0 )
	socket.sysmessage( "Removing Dupes From Map 1, please wait." )
	removeDupesFromMap( socket, 1 )
	socket.sysmessage( "Removing Dupes From Map 2, please wait." )
	removeDupesFromMap( socket, 2 )
	socket.sysmessage( "Removing Dupes From Map 3, please wait." )
	removeDupesFromMap( socket, 3 )

	
def removeDupesFromMap( socket, mapid ):
	squareregionsize = 16

	if mapid == 0:
		xiters = MAP0_WIDTH / squareregionsize
		if 0 <> MAP0_WIDTH % squareregionsize:
			xiters = xiters + 1
		yiters = MAP0_HEIGHT / squareregionsize
		if 0 <> MAP0_HEIGHT % squareregionsize:
			yiters = yiters + 1
	elif mapid == 1 :
		xiters = MAP1_WIDTH / squareregionsize
		if 0 <> MAP1_WIDTH % squareregionsize:
			xiters = xiters + 1
		yiters = MAP1_HEIGHT / squareregionsize
		if 0 <> MAP1_HEIGHT % squareregionsize:
			yiters = yiters + 1
	elif mapid == 2 :
		xiters = MAP2_WIDTH / squareregionsize
		if 0 <> MAP2_WIDTH % squareregionsize:
			xiters = xiters + 1
		yiters = MAP2_HEIGHT / squareregionsize
		if 0 <> MAP2_HEIGHT % squareregionsize:
			yiters = yiters + 1
	elif mapid == 3 :
		xiters = MAP3_WIDTH / squareregionsize
		if 0 <> MAP3_WIDTH % squareregionsize:
			xiters = xiters + 1
		yiters = MAP3_HEIGHT / squareregionsize
		if 0 <> MAP3_HEIGHT % squareregionsize:
			yiters = yiters + 1
	else:
		return

	x = 0
	
	while x <= xiters:
		y = 0
		while y <= yiters:
			DoMapRegion( x * squareregionsize, y * squareregionsize, squareregionsize, mapid )
			y = y + 1
		x = x + 1

def DoMapRegion( x_start, y_start, squareregionsize, mapid ):

	x_end = x_start + squareregionsize
	y_end = y_start + squareregionsize

	if mapid == 0:
		if x_start >= MAP0_WIDTH:
			return
		if y_start >= MAP0_HEIGHT:
			return
		if x_end > MAP0_WIDTH:
			x_end = MAP0_WIDTH
		if y_end > MAP0_HEIGHT:
			y_end = MAP0_HEIGHT
	elif mapid == 1:
		if x_start >= MAP1_WIDTH:
			return
		if y_start >= MAP1_HEIGHT:
			return
		if x_end > MAP1_WIDTH:
			x_end = MAP1_WIDTH
		if y_end > MAP1_HEIGHT:
			y_end = MAP1_HEIGHT
	elif mapid == 2:
		if x_start >= MAP2_WIDTH:
			return
		if y_start >= MAP2_HEIGHT:
			return
		if x_end > MAP2_WIDTH:
			x_end = MAP2_WIDTH
		if y_end > MAP2_HEIGHT:
			y_end = MAP2_HEIGHT
	elif mapid == 3:
		if x_start >= MAP3_WIDTH:
			return
		if y_start >= MAP3_HEIGHT:
			return
		if x_end > MAP3_WIDTH:
			x_end = MAP3_WIDTH
		if y_end > MAP3_HEIGHT:
			y_end = MAP3_HEIGHT
	else:
		return

	iter = wolfpack.itemregion(x_start, y_start, x_end, y_end, mapid)
	items = []
	item = iter.first
	while item:
		items.append(item)
		item = iter.next
	
	todelete = []

	item = iter.first
	while item:
		for itemb in items:
			if item == itemb:
				continue
			if item.pos != itemb.pos or item.id != itemb.id:
				continue
			if itemb not in todelete:
				todelete.append(itemb)
				items.remove(itemb)
				if item in items:
					items.remove(item)
		item = iter.next
				
	for item in todelete:
		item.delete()
		item.update()
