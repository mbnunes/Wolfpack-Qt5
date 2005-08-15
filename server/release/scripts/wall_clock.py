#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: DarkStorm                      #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

import wolfpack.time
from wolfpack import tr

hourStrings = (
	tr("one o'clock"),
	tr("two o'clock"),
	tr("three o'clock"),
	tr("four o'clock"),
	tr("five o'clock"),
	tr("six o'clock"),
	tr("seven o'clock"),
	tr("eight o'clock"),
	tr("nine o'clock"),
	tr("ten o'clock"),
	tr("eleven o'clock"),
	tr("twelve o'clock"),
)

def onUse( char, item ):
	hour = wolfpack.time.hour()
	minute = wolfpack.time.minute()

	# Prefix, then time itself
	if( minute < 15 ):
		time = tr("It is ")
	elif( minute >= 15 and minute < 30 ):
		time = tr("It is a quarter past ")
	elif( minute >= 30 and minute < 45 ):
		time = tr("It is halft past ")
	else:
		time = tr("It is a quarter till ")
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
			time += tr(" in the afternoon")
		else:
			time += tr(" in the evening")

	char.message( time )
	return True
