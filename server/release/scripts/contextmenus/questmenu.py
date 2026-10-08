#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: MagnusBr                       #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification:                         #
#################################################################
import speech
from quests.npcs import npcquestmain, npcquestreport

def onContextEntry( char, target, tag  ):

	if ( tag == 1 ):
		npcquestmain(target, char)

	if ( tag == 2 ):
		npcquestreport(target, char)

	return 1
