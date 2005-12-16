import wolfpack

def onUse( char, item ):
	if Format( char ):
		(yLat, yMins, xLong, xMins, xEast, xSouth) = Format(char)
		east = "E"
		if not xEast:
			east = "W"
		south = "S"
		if not xSouth:
			south = "N"
		location = "%s° %s'%s, %s° %s'%s" % ( yLat, yMins, south, xLong, xMins, east )
		char.message( location )
	return True


def ComputeMapDetails( map, x, y ):
	xWidth = 5120
	yHeight = 4096

	if map == 1 or map == 0:
		if x >= 0 and y >= 0 and x < 5120 and y < 4096:
			xCenter = 1323
			yCenter = 1624
		elif x >= 5120 and y >= 2304 and x < 6144 and y < 4096:
			xCenter = 5936
			yCenter = 3112
		else:
			return False

	elif x >= 0 and y >= 0 and x < map.Width and y < map.Height:
		xCenter = 1323
		yCenter = 1624
	else:
		return False

	return (xCenter, yCenter, xWidth, yHeight)


def Format( char ):
	map = char.pos.map

	if not wolfpack.hasmap(map):
		return False

	x = char.pos.x
	y = char.pos.y

	if not ComputeMapDetails( map, x, y ):
		return False
	else:
		(xCenter, yCenter, xWidth, yHeight) = ComputeMapDetails(map, x, y)

	absLong = float(((x - xCenter) * 360) / xWidth)
	absLat  = float(((y - yCenter) * 360) / yHeight)

	if absLong > 180.0:
		absLong = -180.0 + (absLong % 180.0)

	if absLat > 180.0:
		absLat = -180.0 + (absLat % 180.0)

	east = (absLong >= 0)
	south = (absLat >= 0)

	if absLong < 0.0:
		absLong = -absLong

	if absLat < 0.0:
		absLat = -absLat

	xLong = int(absLong)
	yLat  = int(absLat)

	#char.socket.sysmessage(str(absLong))

	xMins = int((absLong % 1.0) * 60)
	yMins = int((absLat  % 1.0) * 60)
	#char.socket.sysmessage(str(xMins))

	return (yLat, yMins, xLong, xMins, east, south)
