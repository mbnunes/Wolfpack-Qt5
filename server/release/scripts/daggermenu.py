from wolfpack.gumps import cGump

scrLength = 800
scrHeight = 600
GumpLength = 178
GumpHeight = 108
centerX = scrLength/2
centerY = scrHeight/2
LeftGumpX = centerX - gumpLength/2
TopGumpY = centerY - gumpHeight/2

def onContextEntry( char, item, tag  ):
    if( 	
    if( tag == 4 ):
	char.message( "'info " + str( item.serial )  )
    if( tag == 3 ):
	sureGump( char, item, "daggermenu.sureCallback" )
    return 1


def sureCallback( char, args, target ):
    if( target.button == 2 ): char.message( "'remove " + str( args[1].serial ) )

def sureGump( char, item, callback ):

   okGump = cGump( 1, 1, 0 )
   okGump.startPage( 1 )
   okGump.addGump( LeftGumpX, TopGumpY, 2070 )
   okGump.addText( LeftGumpX+GumpLength/4, TopGumpY+GumpHeight/3, "Are you sure ?", 12 )
   # cancel button
   okGump.addButton( LeftGumpX+GumpLength/5.4, TopGumpY+GumpHeight/1.5, 0x817, 0x818, 1 )
   # ok button
   okGump.addButton( LeftGumpX+GumpLength/1.8, TopGumpY+GumpHeight/1.5 , 0x81A, 0x81B, 2 )
   # set callback function and its arguments
   okGump.setCallback( callback )
   # send it
   okGump.setArgs( [ char, item ] )
   okGump.send( char )	

   return 1

