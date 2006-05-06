#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: DarkStorm                      #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

import wolfpack
from wolfpack.consts import *
import random
import skills
import wolfpack.time

EVALINTDELAY = 1000
MINSKILL = 0
MAXSKILL = 1200

def evaluatingintel( char, skill ):
	#Only Handle Evalint
	if skill != EVALUATINGINTEL:
		return False

	if char.socket.hastag( 'skill_delay' ):
		if wolfpack.time.currenttime() < char.socket.gettag( 'skill_delay' ):
			char.socket.clilocmessage( 500118, "", 0x3b2, 3 ) 
			return True
		else:
			char.socket.deltag( 'skill_delay' )

	char.socket.clilocmessage( 500906, "", 0x3b2, 3 ) # What do you wish to evaluate?
	char.socket.attachtarget( "skills.evaluatingintel.response" )
	return True

def response( char, args, target ):
	if not char.canreach( target, 8 ):
		return False

	if skills.skilltable[ EVALUATINGINTEL ][ skills.UNHIDE ] and char.hidden:
		char.reveal()

	if target.char:
		if target.char == char:
			char.socket.clilocmessage( 500910, "", 0x3b2, 3, target.char ) # Hmm, that person looks really silly.

		# Town Criers: 500907 He looks smart enough to remember the news.  Ask him about it.

		elif target.char.ai == "Human_Vendor" and target.char.invulnerable:
			char.socket.clilocmessage( 500909, "", 0x3b2, 3, target.char ) # That person could probably calculate the cost of what you buy from them.
		else:
			marginOfError = max(0, (20 - (char.skill[EVALUATINGINTEL] / 50)))
			char.socket.sysmessage(str(marginOfError))

			intel = target.char.intelligence + random.randint(-marginOfError, marginOfError)
			mana = ((target.char.mana * 100) / max(target.char.maxmana, 1)) + random.randint(-marginOfError, marginOfError)

			intMod = intel / 10
			mnMod = mana / 10

			if intMod > 10:
				intMod = 10
			elif intMod < 0:
				intMod = 0

			if mnMod > 10:
				mnMod = 10
			elif mnMod < 0:
				mnMod = 0

			body = 0
			if target.char.bodytype == 4: # is human
				if target.char.gender:
					body = 11
			else:
				body = 22

			if char.checkskill( EVALUATINGINTEL, MINSKILL, MAXSKILL ):
				char.socket.clilocmessage( 1038169 + intMod + body, "", 0x3b2, 3, target.char ) # He/She/It looks [slighly less intelligent than a rock.]  [Of Average intellect] [etc...]

				if char.skill[EVALUATINGINTEL] >= 760:
					char.socket.clilocmessage( 1038202 + mnMod, "", 0x3b2, 3, target.char ) # That being is at [10,20,...] percent mental strength.
			else:
				char.socket.clilocmessage( 1038166 + (body / 11), "", 0x3b2, 3, target.char ) # You cannot judge his/her/its mental abilities.
	else:
		char.socket.clilocmessage( 500908, "", 0x3b2, 3, target.item ) # It looks smarter than a rock, but dumber than a piece of wood.

	char.socket.settag( 'skill_delay', int( wolfpack.time.currenttime() + EVALINTDELAY ) )
	return True

def onLoad():
	skills.register(EVALUATINGINTEL, evaluatingintel)
