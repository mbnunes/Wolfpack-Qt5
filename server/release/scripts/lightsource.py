#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: DarkStorm                      #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#
# Handles lightsource related stuff                             #
#===============================================================#

# List of "burning" lightsources
burning = [ 0x9fd, 0xa02, 0xa07, 0xa0c, 0xa0f, 0xa12, 0xa15, 0xa1a, 0xa22, 0xb1a, 0xb1d, 0xb20, 0xb22, 0xb24, 0xb26 ]

ids = {
	# Wall Sconche
	0x9FB : 0x9FD,
	0x9FD : 0x9FB,

	0xA00 : 0xA02,
	0xA02 : 0xA00,

	# Wall Torch
	0xA05 : 0xA07,
	0xA07 : 0xA05,

	0xA0A : 0xA0C,
	0xA0C : 0xA0A,

	# Lantern
	0xA18 : 0xA15,
	0xA15 : 0xA18,
	
	# Hanging Lantern
	0xA1D : 0xA1A,
	0xA1A : 0xA1D,

	# Lantern
	0xA25 : 0xA22,
	0xA22 : 0xA25,
	
	# Candle
	0xA28 : 0xA0F,
	0xA0F : 0xA28,

	# Candle (stand)
	0xA26 : 0xB1A,
	0xB1A : 0xA26,

	# Candlabra
	0xA27 : 0xB1D,
	0xB1D : 0xA27,

	# Candlabra
	0xA29 : 0xB26,
	0xB26 : 0xA29,

	# Lamp Post
	0xB21 : 0xB20,
	0xB20 : 0xB21,

	# Lamp Post
	0xB22 : 0xB23,
	0xB23 : 0xB22,

	# Lamp Post
	0xB24 : 0xB25,
	0xB25 : 0xB24,

	}

def onUse( char, item ):
	# Relatively stupid function
	if ids.has_key( item.id ):
		# Change the id
		newid = ids[ item.id ]

		if item.container and item.container.isitem() and newid in burning:
			char.socket.showspeech( item, "You can't light this in a container." )
			return 1

		item.id = newid
		item.update()
		char.soundeffect( 0x226 )

	# This is no Light Source
	else:
		return 0

	return 1

def onDropOnItem( container, item ):
	dropper = item.container

	# Turn off the lightsource
	if item.id in burning and ids.has_key( item.id ):
		item.id = ids[ item.id ]
		item.update()
		dropper.soundeffect( 0x226, 0 )

	return 0

def onDropOnChar( char, item ):

	dropper = item.container

	# Turn off the lightsource
	if item.id in burning and ids.has_key( item.id ):
		item.id = ids[ item.id ]
		item.update()
		dropper.soundeffect( 0x226, 0 )

	return 0
