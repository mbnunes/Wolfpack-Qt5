#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: DarkStorm                      #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#
# .where Command                                                #
#===============================================================#

"""
	\command where
	\description Shows your current position.
"""

import wolfpack

def where( socket, command, arguments ):
	char = socket.player
	region = char.region
	pos = char.pos
	name = None

	while region:
		if region.name and len( region.name ) > 0:
			name = region.name
			break

		region = region.parent

	multi = char.multi

	if multi and len(multi.name) > 0:
		if not name:
			name = multi.name
		else:
			name += ' (%s)' % multi.name

	if pos.map == 0:
		map = 'Felucca'
	elif pos.map == 1:
		map = 'Trammel'
	elif pos.map == 2:
		map = 'Ilshenar'
	elif pos.map == 3:
		map = 'Malas'
	else:
		map = 'Map %d' % pos.map

	if name:
		char.message( "You are in %s at %d,%d,%d on %s" % ( name, pos.x, pos.y, pos.z, map ) )
	else:
		char.message( "You are at %d,%d,%d on %s" % ( pos.x, pos.y, pos.z, map ) )

def onLoad():
	wolfpack.registercommand( "where", where )
