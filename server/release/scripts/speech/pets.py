#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: DarkStorm                      #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#
# Speech Script for Pets                                        #
#===============================================================#

from wolfpack.speech import addKeyword,setCatchAll

def onLoad():
	setCatchAll( 'speech_pets', 0 ) # We don't want to catch all speech but only the pet commands

	# Register some keywords
	addKeyword( 'speech_pets', 0x155 ) # "*come"
	#addKeyword( 'speech_pets', 0x156 ) # "*drop"
	#addKeyword( 'speech_pets', 0x157 ) # "*fetch"
	#addKeyword( 'speech_pets', 0x158 ) # "*get"
	#addKeyword( 'speech_pets', 0x159 ) # "*bring"
	addKeyword( 'speech_pets', 0x15A ) # "*follow"
	addKeyword( 'speech_pets', 0x15B ) # "*friend"
	addKeyword( 'speech_pets', 0x15C ) # "*guard"
	addKeyword( 'speech_pets', 0x15D ) # "*kill"
	addKeyword( 'speech_pets', 0x15E ) # "*attack"
	#addKeyword( 'speech_pets', 0x15F ) # "*patrol"
	#addKeyword( 'speech_pets', 0x160 ) # "*report"
	addKeyword( 'speech_pets', 0x161 ) # "*stop"
	#addKeyword( 'speech_pets', 0x162 ) # "*hire"
	addKeyword( 'speech_pets', 0x163 ) # "*follow me"
	addKeyword( 'speech_pets', 0x164 ) # "all come"
	addKeyword( 'speech_pets', 0x165 ) # "all follow"
	addKeyword( 'speech_pets', 0x166 ) # "all guard"
	addKeyword( 'speech_pets', 0x167 ) # "all stop"
	addKeyword( 'speech_pets', 0x168 ) # "all kill"
	addKeyword( 'speech_pets', 0x169 ) # "all attack"
	#addKeyword( 'speech_pets', 0x16A ) # "all report"
	addKeyword( 'speech_pets', 0x16B ) # "all guard me"
	addKeyword( 'speech_pets', 0x16C ) # "all follow me"
	addKeyword( 'speech_pets', 0x16D ) # "*release"
	addKeyword( 'speech_pets', 0x16E ) # "*transfer"
	addKeyword( 'speech_pets', 0x16F ) # "*stay"
	addKeyword( 'speech_pets', 0x170 ) # "all stay"

def onSpeech( pet, char, text, keywords ):
	# Check for keywords
	checkname = 1

	# for all* commands we dont need to check for the name
	for keyword in keywords:
		if ( keyword >= 0x164 and keyword <= 0x16C ) or keyword == 0x170:
			checkname = 0

	# Check if we are mean't by this command
	if checkname:
		if not text.lower().startswith( pet.name.lower() + " " ):
			return 0
	
	# Check which command was issued:
	for keyword in keywords:

		# "come"
		if keyword == 0x155
			pet_come( pet, char )
			break

	return 1

def pet_come( pet, char ):
	char.message( pet.name + " wants to come to you" )
