#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: DarkStorm                      #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

import wolfpack.time

hourStrings = (
	"one o'clock",
	"two o'clock",
	"three o'clock",
	"four o'clock",
	"five o'clock",
	"six o'clock",
	"seven o'clock",
	"eight o'clock",
	"nine o'clock",
	"ten o'clock",
	"eleven o'clock",
	"twelve o'clock",
)

def onUse( char, item ):
	hour = wolfpack.time.hour()
	minute = wolfpack.time.minute()

	# Prefix, then time itself
	if( minute < 15 ):
		time = "It is "
	elif( minute >= 15 and minute < 30 ):
		time = "It is a quarter past "
	elif( minute >= 30 and minute < 45 ):
		time = "It is halft past "
	else:
		time = "It is a quarter till "
		hour += 1
		if( hour > 24 ):
			hour = 12

	# Normalize time
	if( hour == 0 ):
		hour = 12 # 12 pm
	elif( hour > 12 ):
		hour -= 12

	# Stringify hour
	time += hourStrings[ hour-1 ]
	
	if( wolfpack.time.hour() == 0 or wolfpack.time.hour() > 12 ):
		if( hour >= 1 and hour < 6 ):
			time += " in the afternoon"
		else:
			time += " in the evening"		

	char.message( time )
