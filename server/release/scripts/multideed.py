import wolfpack
from wolfpack.gumps import cGump
from wolfpack.consts import *

#2-Story houses
ch7x7   = [ 7, 7, 0x13ec ]
ch7x8   = [ 7, 8, 0x13ed ]
ch7x9   = [ 7, 9, 0x13ee ]
ch7x10  = [ 7, 10, 0x13ef ]
ch7x11  = [ 7, 11, 0x13f0 ]
ch7x12  = [ 7, 12, 0x13f1 ]
ch8x7   = [ 8, 7, 0x13f8 ]
ch8x8   = [ 8, 8, 0x13f9 ]
ch8x9   = [ 8, 9, 0x13fa ]
ch8x10  = [ 8, 10, 0x13fb ]
ch8x11  = [ 8, 11, 0x13fc ]
ch8x12  = [ 8, 12, 0x13fd ]
ch8x13  = [ 8, 13, 0x13fe ]
ch9x7   = [ 9, 7, 0x1404 ]
ch9x8   = [ 9, 8, 0x1405 ]
ch9x9   = [ 9, 9, 0x1406 ]
ch9x10  = [ 9, 10, 0x1407 ]
ch9x11  = [ 9, 11, 0x1408 ]
ch9x12  = [ 9, 12, 0x1409 ]
ch9x13  = [ 9, 13, 0x140a ]
ch10x7  = [ 10, 7, 0x1410 ]
ch10x8  = [ 10, 8, 0x1411 ]
ch10x9  = [ 10, 9, 0x1412 ]
ch10x10 = [ 10, 10, 0x1413 ]
ch10x11 = [ 10, 11, 0x1414 ]
ch10x12 = [ 10, 12, 0x1415 ]
ch10x13 = [ 10, 13, 0x1416 ]
ch11x7  = [ 11, 7, 0x141c ]
ch11x8  = [ 11, 8, 0x141d ]
ch11x9  = [ 11, 9, 0x141e ]
ch11x10 = [ 11, 10, 0x141f ]
ch11x11 = [ 11, 11, 0x1420 ]
ch11x12 = [ 11, 12, 0x1421 ]
ch11x13 = [ 11, 13, 0x1422 ]
ch12x7  = [ 12, 7, 0x1428 ]
ch12x8  = [ 12, 8, 0x1429 ]
ch12x9  = [ 12, 9, 0x142a ]
ch12x10 = [ 12, 10, 0x142b ]
ch12x11 = [ 12, 11, 0x142c ]
ch12x12 = [ 12, 12, 0x142d ]
ch12x13 = [ 12, 13, 0x142e ]
ch13x8  = [ 13, 8, 0x1435 ]
ch13x9  = [ 13, 9, 0x1436 ]
ch13x10 = [ 13, 10, 0x1437 ]
ch13x11 = [ 13, 11, 0x1438 ]
ch13x12 = [ 13, 12, 0x1439 ]
ch13x13 = [ 13, 13, 0x143a ]

#3-Story houses

#buttons
button_ok = 1
button_cancel = 0

def onUse( char, item ):
   gump1( char, "multideed.gump1callback" )
   char.socket.attachmultitarget( "multideed.response", ch7x7[2], [ch7x7] )
   return 1

def response( char, args, target ):
   ar = args[0]
   foundation( char, target, ar[0], ar[1] )
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
         multi.addchtile( 0x31f4, x, y, 7 )

   #Draw corners
   multi.addchtile( 0x65, left, bottom, 0 )
   multi.addchtile( 0x66, right, top, 0 )

   
   multi.sendcustomhouse( char )

def gump2( char, callback ):
   mgump = cGump( 1 )
   mgump.startPage( 0 )
   mgump.addBackground( 5054, 420, 280 )
   mgump.addTiledGump( 10, 10, 400, 20, 2624 )
   mgump.addCheckerTrans( 10, 10, 400, 20 )
   mgump.addXmfHtmlGump( 10, 10, 400, 20, 1060635, 0, 0, 30720 )
   mgump.addTiledGump( 10, 40, 400, 200, 2624 )
   mgump.addCheckerTrans( 10, 40, 400, 200 )
   mgump.addXmfHtmlGump( 10, 40, 400, 200, 1049583, 0, 1, 32512 )
   mgump.addTiledGump( 10, 250, 400, 20, 2624 )
   mgump.addCheckerTrans( 10, 250, 400, 20 )
   mgump.addButton( 10, 250, 4005, 4007, button_ok )
   mgump.addXmfHtmlGump( 40, 250, 170, 20, 1011036, 0, 0, 32767 )
   mgump.addButton( 210, 250, 4005, 4007, button_cancel )
   mgump.addXmfHtmlGump( 240, 250, 170, 20, 1011012, 0, 0, 32767 )
   mgump.setCallback( callback )
   mgump.send( char )
   
def gump2callback( char, args, target ):
  char.socket.sysmessage( "Button : " + str( target.button ) )
  
def gump1( char, callback ):
   bank = char.getbankbox()
   if bank:
      gold = bank.countresource( 0xEED, 0x0 )
   gump_params = [
   str( gold ),
   "425",
   "212",
   "30500",
   "580",
   "290",
   "34500",
   "650",
   "325",
   "38500",
   "700",
   "350",
   "42500",
   "750",
   "375",
   "46500",
   "800",
   "400",
   "50500",
   "39000",
   "43500",
   "48000",
   "52500",
   "850",
   "57000",
   "1100",
   "550",
   "61500",
   "48500",
   "53500",
   "58500",
   "63500",
   "68500",
   "59000",
   "64500",
   "70000",
   "1150",
   "575",
   "75500",
   "70500",
   "76500",
   "1200",
   "600",
   "82500",
   "83000",
   "1250",
   "625",
   "89500",
   "1300",
   "96500" ]
   mygump = cGump()
   mygump.addRawLayout( "{page 0}" )
   mygump.addRawLayout( "{resizepic 0 0 5054 520 420}" )
   mygump.addRawLayout( "{gumppictiled 10 10 500 20 2624}" )
   mygump.addRawLayout( "{checkertrans 10 10 500 20}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 10 10 500 20 1060239 0 0 32767}" )
   mygump.addRawLayout( "{gumppictiled 10 40 500 20 2624}" )
   mygump.addRawLayout( "{checkertrans 10 40 500 20}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 40 225 20 1060235 0 0 32767}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 275 40 75 20 1060236 0 0 32767}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 350 40 75 20 1060237 0 0 32767}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 425 40 75 20 1060034 0 0 32767}" )
   mygump.addRawLayout( "{gumppictiled 10 70 500 280 2624}" )
   mygump.addRawLayout( "{checkertrans 10 70 500 280}" )
   mygump.addRawLayout( "{gumppictiled 10 360 500 20 2624}" )
   mygump.addRawLayout( "{checkertrans 10 360 500 20}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 10 360 250 20 1060645 0 0 32767}" )
   mygump.addRawLayout( "{text 250 360 1152 0}" )
   mygump.addRawLayout( "{gumppictiled 10 390 500 20 2624}" )
   mygump.addRawLayout( "{checkertrans 10 390 500 20}" )
   mygump.addRawLayout( "{button 10 390 4017 4019 1 0 0}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 390 100 20 3000363 0 0 32767}" )
   mygump.addRawLayout( "{page 1}" )
   mygump.addRawLayout( "{button 10 70 4005 4007 1 0 1}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 70 225 20 1060241 0 0 32767}" )
   mygump.addRawLayout( "{text 275 70 1152 1}" )
   mygump.addRawLayout( "{text 350 70 1152 2}" )
   mygump.addRawLayout( "{text 425 70 1152 3}" )
   mygump.addRawLayout( "{button 10 90 4005 4007 1 0 2}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 90 225 20 1060242 0 0 32767}" )
   mygump.addRawLayout( "{text 275 90 1152 4}" )
   mygump.addRawLayout( "{text 350 90 1152 5}" )
   mygump.addRawLayout( "{text 425 90 1152 6}" )
   mygump.addRawLayout( "{button 10 110 4005 4007 1 0 3}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 110 225 20 1060243 0 0 32767}" )
   mygump.addRawLayout( "{text 275 110 1152 7}" )
   mygump.addRawLayout( "{text 350 110 1152 8}" )
   mygump.addRawLayout( "{text 425 110 1152 9}" )
   mygump.addRawLayout( "{button 10 130 4005 4007 1 0 4}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 130 225 20 1060244 0 0 32767}" )
   mygump.addRawLayout( "{text 275 130 1152 10}" )
   mygump.addRawLayout( "{text 350 130 1152 11}" )
   mygump.addRawLayout( "{text 425 130 1152 12}" )
   mygump.addRawLayout( "{button 10 150 4005 4007 1 0 5}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 150 225 20 1060245 0 0 32767}" )
   mygump.addRawLayout( "{text 275 150 1152 13}" )
   mygump.addRawLayout( "{text 350 150 1152 14}" )
   mygump.addRawLayout( "{text 425 150 1152 15}" )
   mygump.addRawLayout( "{button 10 170 4005 4007 1 0 6}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 170 225 20 1060246 0 0 32767}" )
   mygump.addRawLayout( "{text 275 170 1152 16}" )
   mygump.addRawLayout( "{text 350 170 1152 17}" )
   mygump.addRawLayout( "{text 425 170 1152 18}" )
   mygump.addRawLayout( "{button 10 190 4005 4007 1 0 7}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 190 225 20 1060253 0 0 32767}" )
   mygump.addRawLayout( "{text 275 190 1152 4}" )
   mygump.addRawLayout( "{text 350 190 1152 5}" )
   mygump.addRawLayout( "{text 425 190 1152 6}" )
   mygump.addRawLayout( "{button 10 210 4005 4007 1 0 8}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 210 225 20 1060254 0 0 32767}" )
   mygump.addRawLayout( "{text 275 210 1152 7}" )
   mygump.addRawLayout( "{text 350 210 1152 8}" )
   mygump.addRawLayout( "{text 425 210 1152 19}" )
   mygump.addRawLayout( "{button 10 230 4005 4007 1 0 9}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 230 225 20 1060255 0 0 32767}" )
   mygump.addRawLayout( "{text 275 230 1152 10}" )
   mygump.addRawLayout( "{text 350 230 1152 11}" )
   mygump.addRawLayout( "{text 425 230 1152 20}" )
   mygump.addRawLayout( "{button 10 250 4005 4007 1 0 10}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 250 225 20 1060256 0 0 32767}" )
   mygump.addRawLayout( "{text 275 250 1152 13}" )
   mygump.addRawLayout( "{text 350 250 1152 14}" )
   mygump.addRawLayout( "{text 425 250 1152 21}" )
   mygump.addRawLayout( "{button 10 270 4005 4007 1 0 11}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 270 225 20 1060257 0 0 32767}" )
   mygump.addRawLayout( "{text 275 270 1152 16}" )
   mygump.addRawLayout( "{text 350 270 1152 17}" )
   mygump.addRawLayout( "{text 425 270 1152 22}" )
   mygump.addRawLayout( "{button 10 290 4005 4007 1 0 12}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 290 225 20 1060258 0 0 32767}" )
   mygump.addRawLayout( "{text 275 290 1152 23}" )
   mygump.addRawLayout( "{text 350 290 1152 1}" )
   mygump.addRawLayout( "{text 425 290 1152 24}" )
   mygump.addRawLayout( "{button 10 310 4005 4007 1 0 13}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 310 225 20 1060259 0 0 32767}" )
   mygump.addRawLayout( "{text 275 310 1152 25}" )
   mygump.addRawLayout( "{text 350 310 1152 26}" )
   mygump.addRawLayout( "{text 425 310 1152 27}" )
   mygump.addRawLayout( "{button 10 330 4005 4007 1 0 14}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 330 225 20 1060265 0 0 32767}" )
   mygump.addRawLayout( "{text 275 330 1152 7}" )
   mygump.addRawLayout( "{text 350 330 1152 8}" )
   mygump.addRawLayout( "{text 425 330 1152 9}" )
   mygump.addRawLayout( "{button 450 390 4005 4007 0 2 0}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 400 390 100 20 3000406 0 0 32767}" )
   mygump.addRawLayout( "{page 2}" )
   mygump.addRawLayout( "{button 200 390 4014 4016 0 1 0}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 250 390 100 20 3000405 0 0 32767}" )
   mygump.addRawLayout( "{button 10 70 4005 4007 1 0 15}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 70 225 20 1060266 0 0 32767}" )
   mygump.addRawLayout( "{text 275 70 1152 10}" )
   mygump.addRawLayout( "{text 350 70 1152 11}" )
   mygump.addRawLayout( "{text 425 70 1152 20}" )
   mygump.addRawLayout( "{button 10 90 4005 4007 1 0 16}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 90 225 20 1060267 0 0 32767}" )
   mygump.addRawLayout( "{text 275 90 1152 13}" )
   mygump.addRawLayout( "{text 350 90 1152 14}" )
   mygump.addRawLayout( "{text 425 90 1152 28}" )
   mygump.addRawLayout( "{button 10 110 4005 4007 1 0 17}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 110 225 20 1060268 0 0 32767}" )
   mygump.addRawLayout( "{text 275 110 1152 16}" )
   mygump.addRawLayout( "{text 350 110 1152 17}" )
   mygump.addRawLayout( "{text 425 110 1152 29}" )
   mygump.addRawLayout( "{button 10 130 4005 4007 1 0 18}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 130 225 20 1060269 0 0 32767}" )
   mygump.addRawLayout( "{text 275 130 1152 23}" )
   mygump.addRawLayout( "{text 350 130 1152 1}" )
   mygump.addRawLayout( "{text 425 130 1152 30}" )
   mygump.addRawLayout( "{button 10 150 4005 4007 1 0 19}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 150 225 20 1060270 0 0 32767}" )
   mygump.addRawLayout( "{text 275 150 1152 25}" )
   mygump.addRawLayout( "{text 350 150 1152 26}" )
   mygump.addRawLayout( "{text 425 150 1152 31}" )
   mygump.addRawLayout( "{button 10 170 4005 4007 1 0 20}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 170 225 20 1060271 0 0 32767}" )
   mygump.addRawLayout( "{text 275 170 1152 25}" )
   mygump.addRawLayout( "{text 350 170 1152 26}" )
   mygump.addRawLayout( "{text 425 170 1152 32}" )
   mygump.addRawLayout( "{button 10 190 4005 4007 1 0 21}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 190 225 20 1060277 0 0 32767}" )
   mygump.addRawLayout( "{text 275 190 1152 10}" )
   mygump.addRawLayout( "{text 350 190 1152 11}" )
   mygump.addRawLayout( "{text 425 190 1152 12}" )
   mygump.addRawLayout( "{button 10 210 4005 4007 1 0 22}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 210 225 20 1060278 0 0 32767}" )
   mygump.addRawLayout( "{text 275 210 1152 13}" )
   mygump.addRawLayout( "{text 350 210 1152 14}" )
   mygump.addRawLayout( "{text 425 210 1152 21}" )
   mygump.addRawLayout( "{button 10 230 4005 4007 1 0 23}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 230 225 20 1060279 0 0 32767}" )
   mygump.addRawLayout( "{text 275 230 1152 16}" )
   mygump.addRawLayout( "{text 350 230 1152 17}" )
   mygump.addRawLayout( "{text 425 230 1152 29}" )
   mygump.addRawLayout( "{button 10 250 4005 4007 1 0 24}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 250 225 20 1060280 0 0 32767}" )
   mygump.addRawLayout( "{text 275 250 1152 23}" )
   mygump.addRawLayout( "{text 350 250 1152 1}" )
   mygump.addRawLayout( "{text 425 250 1152 33}" )
   mygump.addRawLayout( "{button 10 270 4005 4007 1 0 25}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 270 225 20 1060281 0 0 32767}" )
   mygump.addRawLayout( "{text 275 270 1152 25}" )
   mygump.addRawLayout( "{text 350 270 1152 26}" )
   mygump.addRawLayout( "{text 425 270 1152 34}" )
   mygump.addRawLayout( "{button 10 290 4005 4007 1 0 26}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 290 225 20 1060282 0 0 32767}" )
   mygump.addRawLayout( "{text 275 290 1152 25}" )
   mygump.addRawLayout( "{text 350 290 1152 26}" )
   mygump.addRawLayout( "{text 425 290 1152 35}" )
   mygump.addRawLayout( "{button 10 310 4005 4007 1 0 27}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 310 225 20 1060283 0 0 32767}" )
   mygump.addRawLayout( "{text 275 310 1152 36}" )
   mygump.addRawLayout( "{text 350 310 1152 37}" )
   mygump.addRawLayout( "{text 425 310 1152 38}" )
   mygump.addRawLayout( "{button 10 330 4005 4007 1 0 28}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 330 225 20 1060289 0 0 32767}" )
   mygump.addRawLayout( "{text 275 330 1152 13}" )
   mygump.addRawLayout( "{text 350 330 1152 14}" )
   mygump.addRawLayout( "{text 425 330 1152 15}" )
   mygump.addRawLayout( "{button 450 390 4005 4007 0 3 0}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 400 390 100 20 3000406 0 0 32767}" )
   mygump.addRawLayout( "{page 3}" )
   mygump.addRawLayout( "{button 200 390 4014 4016 0 2 0}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 250 390 100 20 3000405 0 0 32767}" )
   mygump.addRawLayout( "{button 10 70 4005 4007 1 0 29}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 70 225 20 1060290 0 0 32767}" )
   mygump.addRawLayout( "{text 275 70 1152 16}" )
   mygump.addRawLayout( "{text 350 70 1152 17}" )
   mygump.addRawLayout( "{text 425 70 1152 22}" )
   mygump.addRawLayout( "{button 10 90 4005 4007 1 0 30}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 90 225 20 1060291 0 0 32767}" )
   mygump.addRawLayout( "{text 275 90 1152 23}" )
   mygump.addRawLayout( "{text 350 90 1152 1}" )
   mygump.addRawLayout( "{text 425 90 1152 30}" )
   mygump.addRawLayout( "{button 10 110 4005 4007 1 0 31}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 110 225 20 1060292 0 0 32767}" )
   mygump.addRawLayout( "{text 275 110 1152 25}" )
   mygump.addRawLayout( "{text 350 110 1152 26}" )
   mygump.addRawLayout( "{text 425 110 1152 34}" )
   mygump.addRawLayout( "{button 10 130 4005 4007 1 0 32}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 130 225 20 1060293 0 0 32767}" )
   mygump.addRawLayout( "{text 275 130 1152 25}" )
   mygump.addRawLayout( "{text 350 130 1152 26}" )
   mygump.addRawLayout( "{text 425 130 1152 39}" )
   mygump.addRawLayout( "{button 10 150 4005 4007 1 0 33}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 150 225 20 1060294 0 0 32767}" )
   mygump.addRawLayout( "{text 275 150 1152 36}" )
   mygump.addRawLayout( "{text 350 150 1152 37}" )
   mygump.addRawLayout( "{text 425 150 1152 40}" )
   mygump.addRawLayout( "{button 10 170 4005 4007 1 0 34}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 170 225 20 1060295 0 0 32767}" )
   mygump.addRawLayout( "{text 275 170 1152 41}" )
   mygump.addRawLayout( "{text 350 170 1152 42}" )
   mygump.addRawLayout( "{text 425 170 1152 43}" )
   mygump.addRawLayout( "{button 10 190 4005 4007 1 0 35}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 190 225 20 1060301 0 0 32767}" )
   mygump.addRawLayout( "{text 275 190 1152 16}" )
   mygump.addRawLayout( "{text 350 190 1152 17}" )
   mygump.addRawLayout( "{text 425 190 1152 18}" )
   mygump.addRawLayout( "{button 10 210 4005 4007 1 0 36}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 210 225 20 1060302 0 0 32767}" )
   mygump.addRawLayout( "{text 275 210 1152 23}" )
   mygump.addRawLayout( "{text 350 210 1152 1}" )
   mygump.addRawLayout( "{text 425 210 1152 24}" )
   mygump.addRawLayout( "{button 10 230 4005 4007 1 0 37}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 230 225 20 1060303 0 0 32767}" )
   mygump.addRawLayout( "{text 275 230 1152 25}" )
   mygump.addRawLayout( "{text 350 230 1152 26}" )
   mygump.addRawLayout( "{text 425 230 1152 31}" )
   mygump.addRawLayout( "{button 10 250 4005 4007 1 0 38}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 250 225 20 1060304 0 0 32767}" )
   mygump.addRawLayout( "{text 275 250 1152 25}" )
   mygump.addRawLayout( "{text 350 250 1152 26}" )
   mygump.addRawLayout( "{text 425 250 1152 35}" )
   mygump.addRawLayout( "{button 10 270 4005 4007 1 0 39}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 270 225 20 1060305 0 0 32767}" )
   mygump.addRawLayout( "{text 275 270 1152 36}" )
   mygump.addRawLayout( "{text 350 270 1152 37}" )
   mygump.addRawLayout( "{text 425 270 1152 40}" )
   mygump.addRawLayout( "{button 10 290 4005 4007 1 0 40}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 290 225 20 1060306 0 0 32767}" )
   mygump.addRawLayout( "{text 275 290 1152 41}" )
   mygump.addRawLayout( "{text 350 290 1152 42}" )
   mygump.addRawLayout( "{text 425 290 1152 44}" )
   mygump.addRawLayout( "{button 10 310 4005 4007 1 0 41}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 310 225 20 1060307 0 0 32767}" )
   mygump.addRawLayout( "{text 275 310 1152 45}" )
   mygump.addRawLayout( "{text 350 310 1152 46}" )
   mygump.addRawLayout( "{text 425 310 1152 47}" )
   mygump.addRawLayout( "{button 10 330 4005 4007 1 0 42}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 330 225 20 1060314 0 0 32767}" )
   mygump.addRawLayout( "{text 275 330 1152 25}" )
   mygump.addRawLayout( "{text 350 330 1152 26}" )
   mygump.addRawLayout( "{text 425 330 1152 27}" )
   mygump.addRawLayout( "{button 450 390 4005 4007 0 4 0}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 400 390 100 20 3000406 0 0 32767}" )
   mygump.addRawLayout( "{page 4}" )
   mygump.addRawLayout( "{button 200 390 4014 4016 0 3 0}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 250 390 100 20 3000405 0 0 32767}" )
   mygump.addRawLayout( "{button 10 70 4005 4007 1 0 43}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 70 225 20 1060315 0 0 32767}" )
   mygump.addRawLayout( "{text 275 70 1152 25}" )
   mygump.addRawLayout( "{text 350 70 1152 26}" )
   mygump.addRawLayout( "{text 425 70 1152 32}" )
   mygump.addRawLayout( "{button 10 90 4005 4007 1 0 44}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 90 225 20 1060316 0 0 32767}" )
   mygump.addRawLayout( "{text 275 90 1152 36}" )
   mygump.addRawLayout( "{text 350 90 1152 37}" )
   mygump.addRawLayout( "{text 425 90 1152 38}" )
   mygump.addRawLayout( "{button 10 110 4005 4007 1 0 45}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 110 225 20 1060317 0 0 32767}" )
   mygump.addRawLayout( "{text 275 110 1152 41}" )
   mygump.addRawLayout( "{text 350 110 1152 42}" )
   mygump.addRawLayout( "{text 425 110 1152 43}" )
   mygump.addRawLayout( "{button 10 130 4005 4007 1 0 46}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 130 225 20 1060318 0 0 32767}" )
   mygump.addRawLayout( "{text 275 130 1152 45}" )
   mygump.addRawLayout( "{text 350 130 1152 46}" )
   mygump.addRawLayout( "{text 425 130 1152 47}" )
   mygump.addRawLayout( "{button 10 150 4005 4007 1 0 47}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 150 225 20 1060319 0 0 32767}" )
   mygump.addRawLayout( "{text 275 150 1152 48}" )
   mygump.addRawLayout( "{text 350 150 1152 7}" )
   mygump.addRawLayout( "{text 425 150 1152 49}" )

   #Add params
   for line in gump_params:
      mygump.addRawText( line )

   mygump.setCallback( callback )
   mygump.send( char )
   
def gump1callback( char, args, target ):
  char.socket.sysmessage( "Button : " + str( target.button ) )
