#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: Viper                          #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

from wolfpack.consts import *
import wolfpack

def onLoad():
	wolfpack.registerglobal( HOOK_CHAR, EVENT_SKILLUSE, "skills.armslore" )

def onSkillUse( char, skill ):
	# only handle armslore
	if skill != ARMSLORE:
		return 0

	char.socket.clilocmessage( 0x7A27D, "", 0x3b2, 3 )
	char.socket.attachtarget( "skills.armslore.response" )

	return 1

def response( char, args, target ):
	#todo: add weapon and armor ids into lists
	weapontypes = [ 1001, 1002, 1003, 1004, 1005, 1006, 1007 ]
	armors = [ 0x13be, 0x13bf, 0x13c0, 0x13c3, 0x13c4, 0x13c5, 0x13c6, 0x13c7, 0x13cb, \
			0x13cc, 0x13cd, 0x13ce, 0x13d2, 0x13d3, 0x13d4, 0x13d5, 0x13d6, \
			0x13da, 0x13db, 0x13dc, 0x13dd, 0x13e1, 0x13e2, 0x13d2, 0x13eb, \
			0x13ec, 0x13ed, 0x13ee, 0x13ef, 0x13f0, 0x13f1, 0x13f2, 0x1408, \
			0x1409, 0x140a, 0x140b, 0x140c, 0x140d, 0x140e, 0x140f, 0x1410, \
			0x1411, 0x1412, 0x1413, 0x1414, 0x1415, 0x1416, 0x1417, 0x1418, \
			0x1419, 0x141a, 0x144e, 0x144f, 0x1450, 0x1451, 0x1452, 0x1453, \
			0x1454, 0x1455, 0x1456, 0x1457 ] 

	backpack = char.getbackpack()

	if target.item:
		if not target.item.getoutmostchar() == char:

			if not char.canreach( target.char, 4 ):
				char.socket.clilocmessage( 0x7A27F, "", 0x3b2, 3 )
				return 0

			if not char.distanceto ( target.item ) < 5:
				char.socket.clilocmessage( 0x7A27E, "", 0x3b2, 3 )
				return 0

		if not target.item.type in weapontypes:
			if not target.item.id in armors:
				char.socket.clilocmessage( 0x7A280, "", 0x3b2, 3 )
				return 0

		if not char.checkskill( ARMSLORE, 0, 1000 ):
			char.socket.clilocmessage( 0x7A281, "", 0x3b2, 3 )
			return 0

		char.socket.clilocmessage( 0x103319, "", 0x3b2, 3 )
		return 1

	else:
		if not char.canreach( target.char, 4 ):
			char.socket.clilocmessage( 0x7A27F, "", 0x3b2, 3 )
			return 0

		if not char.distanceto ( target.char ) < 5:
			char.socket.clilocmessage( 0x7A27E, "", 0x3b2, 3 )
			return 0

		char.socket.clilocmessage( 0x7A280, "", 0x3b2, 3 )
		return 0


	
