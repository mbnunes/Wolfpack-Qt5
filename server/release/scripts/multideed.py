import wolfpack
from wolfpack.consts import *

def onUse( char, item ):
   char.socket.attachmultitarget( "multideed.response", 0x13ec )
   return 1

def response( char, args, target ):
   foundation( char, target, 7, 7 )
   return 1

def foundation( char, target, width, height ):
   multi = wolfpack.multi( CUSTOMHOUSE )
   multi.moveto( target.pos )

   left = width/2
   right = left - ( width-1 )
   bottom = height/2
   top = bottom - ( height-1 )

   #Draw sides
   for x in xrange( right+1,left+1 ):
      multi.addchtile( 0x63, x, top, 0 )
      multi.addchtile( 0x63, x, bottom, 0 )
         
   for y in xrange( top+1, bottom+1 ):
      multi.addchtile( 0x64, right, y, 0 )
      multi.addchtile( 0x64, left, y, 0 )
   
   #Draw stairs
   for x in xrange( right+1,left+1 ):
      multi.addchtile( 0x0751, x, bottom+1, 0 )

   #Draw floor
   for y in xrange( top+1,bottom+1 ):
      for x in xrange( right+1,left+1 ):
         char.socket.sysmessage( str(x) +" "+ str(y) )
         multi.addchtile( 0x31f4, x, y, 7 )

   #Draw corners
   multi.addchtile( 0x65, left, bottom, 0 )
   multi.addchtile( 0x66, right, top, 0 )

   
   multi.sendcustomhouse( char )
   