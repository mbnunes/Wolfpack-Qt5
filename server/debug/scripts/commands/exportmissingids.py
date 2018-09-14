#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: Incanus                        #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#
# .exportmissingids Command					#
#===============================================================#

"""
	\command exportmissingids
	\description Export a list of item ids that have no item definition.
	\notes Tiledata.mul is searched for item ids.
	
	Be careful, not all found items do exist in art.mul.

"""

from wolfpack import tiledata, registercommand, getdefinition
from wolfpack.consts import WPDT_ITEM

import os

def searchidscmd( socket, command, arguments ):
	out = open( 'missingids.xml', "wb" )

	if os.name == 'posix':
		nl = "\n"
	else:
		nl = "\r\n"

	out.write( '<definitions>%s' % ( nl ) )
	for id in range(0x0, 0x4000):
		tile = tiledata( id )

		# unused ids have only 5 entries in tiledata
		if len( tile ) > 5:
			itemdef =  hex(id).split("x")[1]
			if not getdefinition(WPDT_ITEM, itemdef):
				out.write( '\t<!-- %s -->%s' % ( tile['name'], nl ) )
				out.write( '\t<item id="%s">%s' % ( itemdef, nl ) )
				out.write( '\t\t<id>%s</id>%s' % ( hex(id), nl ) )
				out.write( '\t</item>%s' % ( nl ) )
				out.write( nl )

	out.write( '</definitions>%s' % ( nl ) )
	out.close()

def onLoad():
	registercommand( "exportmissingids", searchidscmd )
	return
