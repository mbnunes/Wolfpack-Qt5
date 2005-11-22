#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts
#  ((    _/{  "-;  | Created by: MagnusBr
#   )).-' {{ ;'`   | Revised by:
#  ( (  ;._ \\ ctr | Last Modification: Created
#===============================================================#
# Quest System - Events for Quest System
#===============================================================#

import wolfpack
from quests.functions import checknpcforquest

#######################################################################################
##############   Death Events for Kill NPC Module   ###################################
#######################################################################################

def onDeath(char, source, corpse):

	if source and source.ischar() and source.socket:
	
		checknpcforquest(source, char)