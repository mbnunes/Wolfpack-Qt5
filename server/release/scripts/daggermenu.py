import wolfpack
from wolfpack.gumps import cGump

def onContextEntry( char, item, tag  ):
    if( tag == 4 ):
	char.message( "'info " + str( item.serial )  )
        
    return 1
