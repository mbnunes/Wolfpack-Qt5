#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: DarkStorm                      #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

from wolfpack.consts import *
from wolfpack.utilities import *
import wolfpack
import wolfpack.time
import skills

TASTEID_DELAY = 1000

def tasteid(char, skill):
	socket = char.socket

	if socket.hastag('skill_delay'):
		if wolfpack.time.currenttime() < socket.gettag('skill_delay'):
			socket.clilocmessage(500118)
			return 1
		else:
			socket.deltag('skill_delay')

	socket.clilocmessage(502807)
	socket.attachtarget("skills.tasteid.response")
	return 1

def response(char, args, target):
	socket = char.socket

	socket.settag('skill_delay', int( wolfpack.time.currenttime() + TASTEID_DELAY ) )

	if target.item:
		if not char.canreach(target.item, 2):
			socket.clilocmessage(500312)
			return

		if not target.item.hasevent( 'food' ):
			socket.clilocmessage(502820)
		else:
			if not char.checkskill(TASTEID, 0, 1000):
				socket.clilocmessage(502823)
			else:
				# TODO: Check for poison in the food.
				socket.clilocmessage(502823)

	elif target.char:
		socket.clilocmessage(502816)

	else:
		socket.clilocmessage(502820)

def onLoad():
	skills.register(TASTEID, tasteid)
