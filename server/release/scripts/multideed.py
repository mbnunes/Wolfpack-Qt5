import wolfpack
from wolfpack.gumps import cGump
from wolfpack.consts import *

#2-Story houses
ch2story = [
[ 7, 7, 0x13ec ],
[ 7, 8, 0x13ed ],
[ 7, 9, 0x13ee ],
[ 7, 10, 0x13ef ],
[ 7, 11, 0x13f0 ],
[ 7, 12, 0x13f1 ],
[ 8, 7, 0x13f8 ],
[ 8, 8, 0x13f9 ],
[ 8, 9, 0x13fa ],
[ 8, 10, 0x13fb ],
[ 8, 11, 0x13fc ],
[ 8, 12, 0x13fd ],
[ 8, 13, 0x13fe ],
[ 9, 7, 0x1404 ],
[ 9, 8, 0x1405 ],
[ 9, 9, 0x1406 ],
[ 9, 10, 0x1407 ],
[ 9, 11, 0x1408 ],
[ 9, 12, 0x1409 ],
[ 9, 13, 0x140a ],
[ 10, 7, 0x1410 ],
[ 10, 8, 0x1411 ],
[ 10, 9, 0x1412 ],
[ 10, 10, 0x1413 ],
[ 10, 11, 0x1414 ],
[ 10, 12, 0x1415 ],
[ 10, 13, 0x1416 ],
[ 11, 7, 0x141c ],
[ 11, 8, 0x141d ],
[ 11, 9, 0x141e ],
[ 11, 10, 0x141f ],
[ 11, 11, 0x1420 ],
[ 11, 12, 0x1421 ],
[ 11, 13, 0x1422 ],
[ 12, 7, 0x1428 ],
[ 12, 8, 0x1429 ],
[ 12, 9, 0x142a ],
[ 12, 10, 0x142b ],
[ 12, 11, 0x142c ],
[ 12, 12, 0x142d ],
[ 12, 13, 0x142e ],
[ 13, 8, 0x1435 ],
[ 13, 9, 0x1436 ],
[ 13, 10, 0x1437 ],
[ 13, 11, 0x1438 ],
[ 13, 12, 0x1439 ],
[ 13, 13, 0x143a ] ]

#3-Story houses
ch3story = [
[ 9, 14, 0x140b ],
[ 10, 14, 0x1417 ],
[ 10, 15, 0x1418 ],
[ 11, 14, 0x1423 ],
[ 11, 15, 0x1424 ],
[ 11, 16, 0x1425 ],
[ 12, 14, 0x142f ],
[ 12, 15, 0x1430 ],
[ 12, 16, 0x1431 ],
[ 12, 17, 0x1432 ],
[ 13, 14, 0x143b ],
[ 13, 15, 0x143c ],
[ 13, 16, 0x143d ],
[ 13, 17, 0x143e ],
[ 13, 18, 0x143f ],
[ 14, 9, 0x1442 ],
[ 14, 10, 0x1443 ],
[ 14, 11, 0x1444 ],
[ 14, 12, 0x1445 ],
[ 14, 13, 0x1446 ],
[ 14, 14, 0x1447 ],
[ 14, 15, 0x1448 ],
[ 14, 16, 0x1449 ],
[ 14, 17, 0x144a ],
[ 14, 18, 0x144b ],
[ 15, 10, 0x144f ],
[ 15, 11, 0x1450 ],
[ 15, 12, 0x1451 ],
[ 15, 13, 0x1452 ],
[ 15, 14, 0x1453 ],
[ 15, 15, 0x1454 ],
[ 15, 16, 0x1455 ],
[ 15, 17, 0x1456 ],
[ 15, 18, 0x1457 ],
[ 16, 11, 0x145c ],
[ 16, 12, 0x145d ],
[ 16, 13, 0x145e ],
[ 16, 14, 0x145f ],
[ 16, 15, 0x1460 ],
[ 16, 16, 0x1461 ],
[ 16, 17, 0x1462 ],
[ 16, 18, 0x1463 ],
[ 17, 12, 0x1469 ],
[ 17, 13, 0x146a ],
[ 17, 14, 0x146b ],
[ 17, 15, 0x146c ],
[ 17, 16, 0x146d ],
[ 17, 17, 0x146e ],
[ 17, 18, 0x146f ],
[ 18, 13, 0x1476 ],
[ 18, 14, 0x1477 ],
[ 18, 15, 0x1478 ],
[ 18, 16, 0x1479 ],
[ 18, 17, 0x147a ],
[ 18, 18, 0x147b ]]

#buttons
button_ok = 1
button_cancel = 0

def onUse( char, item ):
   gump0( char, "multideed.gump0callback" )
   return 1

#Selection of custom house type ( 2-story / 3-story )
def gump0callback( char, args, target ): 
   button = target.button

   #Old style houses
   if button == 1: char.socket.clilocmessage( 1005691 )
   #2-Story houses
   if button == 2: gump1( char, "multideed.gump1callback", ch2story )
   #3-Story houses
   if button == 3: gump3( char, "multideed.gump1callback", ch3story )

#Selection of 2-story size
def gump1callback( char, args, target ):
   button = target.button
   if button > 0: char.socket.attachmultitarget( "multideed.response", args[0][button-1][2], args[0][button-1] )

#CH placement warning gump callback
def gump2callback( char, args, target ):
   button = target.button
   if button == 1: foundation( char, args[1], args[0][0], args[0][1] )

def response( char, args, target ):
   gump2( char, "multideed.gump2callback", args, target )
   return 1

def foundation( char, target, width, height ):
   multi = wolfpack.multi( CUSTOMHOUSE )
   multi.decay = FALSE
   multi.moveto( target.pos )

   left = width/2
   right = left - ( width-1 )
   bottom = height/2
   top = bottom - ( height-1 )

   #Draw floor
   for y in xrange( top+1,bottom+1 ):
      for x in xrange( right+1,left+1 ):
         if x == 0 and y == 0:
            multi.addchtile( 0x1, 0, 0, 0 )
         multi.addchtile( 0x31f4, x, y, 7 )

   #Draw corners
   multi.addchtile( 0x66, right, top, 0 )
   multi.addchtile( 0x65, left, bottom, 0 )

   #Draw sides
   for x in xrange( right+1,left+1 ):
      multi.addchtile( 0x63, x, top, 0 )
      if x < left:
         multi.addchtile( 0x63, x, bottom, 0 )
         
   for y in xrange( top+1, bottom+1 ):
      multi.addchtile( 0x64, right, y, 0 )
      multi.addchtile( 0x64, left, y, 0 )
   
   #Draw stairs
   for x in xrange( right+1,left+1 ):
      multi.addchtile( 0x0751, x, bottom+1, 0 )

   
   multi.sendcustomhouse( char )
   #woodenpost = wolfpack.additem( "9" )
   signpost = wolfpack.additem( "b98" )
   sign = wolfpack.additem( "bd2" )
   #woodenpost.decay = FALSE
   signpost.decay = FALSE
   sign.decay = FALSE
   x = multi.pos.x + right
   y = multi.pos.y + bottom
   z = multi.pos.z + 7
   map = multi.pos.map
   newpos = wolfpack.coord( x, y, z, map )
   #woodenpost.moveto( newpos )
   newpos.y += 1
   signpost.moveto( newpos )
   sign.moveto( newpos )
   sign.morex = multi.serial
   sign.morey = signpost.serial
   #sign.morez = woodenpost.serial
   sign.events = ["signpost"]
   #woodenpost.update()
   signpost.update()
   sign.update()

   

#House type selection common/2-story/3-story
def gump0( char, callback ):
   mygump = cGump()
   mygump.addRawLayout( "{page 0}" )
   mygump.addRawLayout( "{resizepic 0 0 5054 270 145}" )
   mygump.addRawLayout( "{gumppictiled 10 10 250 125 2624}" )
   mygump.addRawLayout( "{checkertrans 10 10 250 125}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 10 10 250 20 1060239 0 0 32767}" )
   mygump.addRawLayout( "{button 10 110 4017 4019 1 0 0}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 45 110 150 20 3000363 0 0 32767}" )
   mygump.addRawLayout( "{button 10 40 4005 4007 1 0 1}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 45 40 200 20 1060390 0 0 32767}" )
   mygump.addRawLayout( "{button 10 60 4005 4007 1 0 2}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 45 60 200 20 1060391 0 0 32767}" )
   mygump.addRawLayout( "{button 10 80 4005 4007 1 0 3}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 45 80 200 20 1060392 0 0 32767}" )
   mygump.setCallback( callback )
   mygump.send( char )

#2-story house size selection
def gump1( char, callback, chsizes ):
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

   mygump.setArgs( [chsizes] )
   mygump.setCallback( callback )
   mygump.send( char )


def gump3( char, callback, chsizes ):
   bank = char.getbankbox()
   if bank:
      gold = bank.countresource( 0xEED, 0x0 )
   gump_params = [
   str( gold ),
   "1150",
   "575",
   "73500",
   "1200",
   "600",
   "81000",
   "1250",
   "625",
   "86500",
   "88500",
   "1300",
   "650",
   "94500",
   "1350",
   "675",
   "100500",
   "96000",
   "102500",
   "1370",
   "685",
   "109000",
   "115500",
   "103500",
   "110500",
   "117500",
   "2119",
   "1059",
   "124500",
   "131500",
   "111000",
   "118500",
   "126000",
   "133500",
   "141000",
   "126500",
   "134500",
   "142500",
   "150500",
   "143000",
   "151500",
   "160000",
   "160500",
   "169500",
   "179000"]

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
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 70 225 20 1060272 0 0 32767}" )
   mygump.addRawLayout( "{text 275 70 1152 1}" )
   mygump.addRawLayout( "{text 350 70 1152 2}" )
   mygump.addRawLayout( "{text 425 70 1152 3}" )
   mygump.addRawLayout( "{button 10 90 4005 4007 1 0 2}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 90 225 20 1060284 0 0 32767}" )
   mygump.addRawLayout( "{text 275 90 1152 4}" )
   mygump.addRawLayout( "{text 350 90 1152 5}" )
   mygump.addRawLayout( "{text 425 90 1152 6}" )
   mygump.addRawLayout( "{button 10 110 4005 4007 1 0 3}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 110 225 20 1060285 0 0 32767}" )
   mygump.addRawLayout( "{text 275 110 1152 7}" )
   mygump.addRawLayout( "{text 350 110 1152 8}" )
   mygump.addRawLayout( "{text 425 110 1152 9}" )
   mygump.addRawLayout( "{button 10 130 4005 4007 1 0 4}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 130 225 20 1060296 0 0 32767}" )
   mygump.addRawLayout( "{text 275 130 1152 7}" )
   mygump.addRawLayout( "{text 350 130 1152 8}" )
   mygump.addRawLayout( "{text 425 130 1152 10}" )
   mygump.addRawLayout( "{button 10 150 4005 4007 1 0 5}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 150 225 20 1060297 0 0 32767}" )
   mygump.addRawLayout( "{text 275 150 1152 11}" )
   mygump.addRawLayout( "{text 350 150 1152 12}" )
   mygump.addRawLayout( "{text 425 150 1152 13}" )
   mygump.addRawLayout( "{button 10 170 4005 4007 1 0 6}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 170 225 20 1060298 0 0 32767}" )
   mygump.addRawLayout( "{text 275 170 1152 14}" )
   mygump.addRawLayout( "{text 350 170 1152 15}" )
   mygump.addRawLayout( "{text 425 170 1152 16}" )
   mygump.addRawLayout( "{button 10 190 4005 4007 1 0 7}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 190 225 20 1060308 0 0 32767}" )
   mygump.addRawLayout( "{text 275 190 1152 11}" )
   mygump.addRawLayout( "{text 350 190 1152 12}" )
   mygump.addRawLayout( "{text 425 190 1152 17}" )
   mygump.addRawLayout( "{button 10 210 4005 4007 1 0 8}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 210 225 20 1060309 0 0 32767}" )
   mygump.addRawLayout( "{text 275 210 1152 14}" )
   mygump.addRawLayout( "{text 350 210 1152 15}" )
   mygump.addRawLayout( "{text 425 210 1152 18}" )
   mygump.addRawLayout( "{button 10 230 4005 4007 1 0 9}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 230 225 20 1060310 0 0 32767}" )
   mygump.addRawLayout( "{text 275 230 1152 19}" )
   mygump.addRawLayout( "{text 350 230 1152 20}" )
   mygump.addRawLayout( "{text 425 230 1152 21}" )
   mygump.addRawLayout( "{button 10 250 4005 4007 1 0 10}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 250 225 20 1060311 0 0 32767}" )
   mygump.addRawLayout( "{text 275 250 1152 19}" )
   mygump.addRawLayout( "{text 350 250 1152 20}" )
   mygump.addRawLayout( "{text 425 250 1152 22}" )
   mygump.addRawLayout( "{button 10 270 4005 4007 1 0 11}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 270 225 20 1060320 0 0 32767}" )
   mygump.addRawLayout( "{text 275 270 1152 14}" )
   mygump.addRawLayout( "{text 350 270 1152 15}" )
   mygump.addRawLayout( "{text 425 270 1152 23}" )
   mygump.addRawLayout( "{button 10 290 4005 4007 1 0 12}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 290 225 20 1060321 0 0 32767}" )
   mygump.addRawLayout( "{text 275 290 1152 19}" )
   mygump.addRawLayout( "{text 350 290 1152 20}" )
   mygump.addRawLayout( "{text 425 290 1152 24}" )
   mygump.addRawLayout( "{button 10 310 4005 4007 1 0 13}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 310 225 20 1060322 0 0 32767}" )
   mygump.addRawLayout( "{text 275 310 1152 19}" )
   mygump.addRawLayout( "{text 350 310 1152 20}" )
   mygump.addRawLayout( "{text 425 310 1152 25}" )
   mygump.addRawLayout( "{button 10 330 4005 4007 1 0 14}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 330 225 20 1060323 0 0 32767}" )
   mygump.addRawLayout( "{text 275 330 1152 26}" )
   mygump.addRawLayout( "{text 350 330 1152 27}" )
   mygump.addRawLayout( "{text 425 330 1152 28}" )
   mygump.addRawLayout( "{button 450 390 4005 4007 0 2 0}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 400 390 100 20 3000406 0 0 32767}" )
   mygump.addRawLayout( "{page 2}" )
   mygump.addRawLayout( "{button 200 390 4014 4016 0 1 0}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 250 390 100 20 3000405 0 0 32767}" )
   mygump.addRawLayout( "{button 10 70 4005 4007 1 0 15}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 70 225 20 1060324 0 0 32767}" )
   mygump.addRawLayout( "{text 275 70 1152 26}" )
   mygump.addRawLayout( "{text 350 70 1152 27}" )
   mygump.addRawLayout( "{text 425 70 1152 29}" )
   mygump.addRawLayout( "{button 10 90 4005 4007 1 0 16}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 90 225 20 1060327 0 0 32767}" )
   mygump.addRawLayout( "{text 275 90 1152 1}" )
   mygump.addRawLayout( "{text 350 90 1152 2}" )
   mygump.addRawLayout( "{text 425 90 1152 3}" )
   mygump.addRawLayout( "{button 10 110 4005 4007 1 0 17}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 110 225 20 1060328 0 0 32767}" )
   mygump.addRawLayout( "{text 275 110 1152 4}" )
   mygump.addRawLayout( "{text 350 110 1152 5}" )
   mygump.addRawLayout( "{text 425 110 1152 6}" )
   mygump.addRawLayout( "{button 10 130 4005 4007 1 0 18}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 130 225 20 1060329 0 0 32767}" )
   mygump.addRawLayout( "{text 275 130 1152 7}" )
   mygump.addRawLayout( "{text 350 130 1152 8}" )
   mygump.addRawLayout( "{text 425 130 1152 10}" )
   mygump.addRawLayout( "{button 10 150 4005 4007 1 0 19}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 150 225 20 1060330 0 0 32767}" )
   mygump.addRawLayout( "{text 275 150 1152 11}" )
   mygump.addRawLayout( "{text 350 150 1152 12}" )
   mygump.addRawLayout( "{text 425 150 1152 17}" )
   mygump.addRawLayout( "{button 10 170 4005 4007 1 0 20}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 170 225 20 1060331 0 0 32767}" )
   mygump.addRawLayout( "{text 275 170 1152 14}" )
   mygump.addRawLayout( "{text 350 170 1152 15}" )
   mygump.addRawLayout( "{text 425 170 1152 23}" )
   mygump.addRawLayout( "{button 10 190 4005 4007 1 0 21}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 190 225 20 1060332 0 0 32767}" )
   mygump.addRawLayout( "{text 275 190 1152 19}" )
   mygump.addRawLayout( "{text 350 190 1152 20}" )
   mygump.addRawLayout( "{text 425 190 1152 30}" )
   mygump.addRawLayout( "{button 10 210 4005 4007 1 0 22}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 210 225 20 1060333 0 0 32767}" )
   mygump.addRawLayout( "{text 275 210 1152 19}" )
   mygump.addRawLayout( "{text 350 210 1152 20}" )
   mygump.addRawLayout( "{text 425 210 1152 31}" )
   mygump.addRawLayout( "{button 10 230 4005 4007 1 0 23}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 230 225 20 1060334 0 0 32767}" )
   mygump.addRawLayout( "{text 275 230 1152 26}" )
   mygump.addRawLayout( "{text 350 230 1152 27}" )
   mygump.addRawLayout( "{text 425 230 1152 32}" )
   mygump.addRawLayout( "{button 10 250 4005 4007 1 0 24}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 250 225 20 1060335 0 0 32767}" )
   mygump.addRawLayout( "{text 275 250 1152 26}" )
   mygump.addRawLayout( "{text 350 250 1152 27}" )
   mygump.addRawLayout( "{text 425 250 1152 33}" )
   mygump.addRawLayout( "{button 10 270 4005 4007 1 0 25}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 270 225 20 1060336 0 0 32767}" )
   mygump.addRawLayout( "{text 275 270 1152 26}" )
   mygump.addRawLayout( "{text 350 270 1152 27}" )
   mygump.addRawLayout( "{text 425 270 1152 34}" )
   mygump.addRawLayout( "{button 10 290 4005 4007 1 0 26}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 290 225 20 1060340 0 0 32767}" )
   mygump.addRawLayout( "{text 275 290 1152 7}" )
   mygump.addRawLayout( "{text 350 290 1152 8}" )
   mygump.addRawLayout( "{text 425 290 1152 9}" )
   mygump.addRawLayout( "{button 10 310 4005 4007 1 0 27}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 310 225 20 1060341 0 0 32767}" )
   mygump.addRawLayout( "{text 275 310 1152 11}" )
   mygump.addRawLayout( "{text 350 310 1152 12}" )
   mygump.addRawLayout( "{text 425 310 1152 13}" )
   mygump.addRawLayout( "{button 10 330 4005 4007 1 0 28}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 330 225 20 1060342 0 0 32767}" )
   mygump.addRawLayout( "{text 275 330 1152 14}" )
   mygump.addRawLayout( "{text 350 330 1152 15}" )
   mygump.addRawLayout( "{text 425 330 1152 18}" )
   mygump.addRawLayout( "{button 450 390 4005 4007 0 3 0}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 400 390 100 20 3000406 0 0 32767}" )
   mygump.addRawLayout( "{page 3}" )
   mygump.addRawLayout( "{button 200 390 4014 4016 0 2 0}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 250 390 100 20 3000405 0 0 32767}" )
   mygump.addRawLayout( "{button 10 70 4005 4007 1 0 29}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 70 225 20 1060343 0 0 32767}" )
   mygump.addRawLayout( "{text 275 70 1152 19}" )
   mygump.addRawLayout( "{text 350 70 1152 20}" )
   mygump.addRawLayout( "{text 425 70 1152 24}" )
   mygump.addRawLayout( "{button 10 90 4005 4007 1 0 30}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 90 225 20 1060344 0 0 32767}" )
   mygump.addRawLayout( "{text 275 90 1152 19}" )
   mygump.addRawLayout( "{text 350 90 1152 20}" )
   mygump.addRawLayout( "{text 425 90 1152 31}" )
   mygump.addRawLayout( "{button 10 110 4005 4007 1 0 31}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 110 225 20 1060345 0 0 32767}" )
   mygump.addRawLayout( "{text 275 110 1152 26}" )
   mygump.addRawLayout( "{text 350 110 1152 27}" )
   mygump.addRawLayout( "{text 425 110 1152 35}" )
   mygump.addRawLayout( "{button 10 130 4005 4007 1 0 32}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 130 225 20 1060346 0 0 32767}" )
   mygump.addRawLayout( "{text 275 130 1152 26}" )
   mygump.addRawLayout( "{text 350 130 1152 27}" )
   mygump.addRawLayout( "{text 425 130 1152 36}" )
   mygump.addRawLayout( "{button 10 150 4005 4007 1 0 33}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 150 225 20 1060347 0 0 32767}" )
   mygump.addRawLayout( "{text 275 150 1152 26}" )
   mygump.addRawLayout( "{text 350 150 1152 27}" )
   mygump.addRawLayout( "{text 425 150 1152 37}" )
   mygump.addRawLayout( "{button 10 170 4005 4007 1 0 34}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 170 225 20 1060348 0 0 32767}" )
   mygump.addRawLayout( "{text 275 170 1152 26}" )
   mygump.addRawLayout( "{text 350 170 1152 27}" )
   mygump.addRawLayout( "{text 425 170 1152 38}" )
   mygump.addRawLayout( "{button 10 190 4005 4007 1 0 35}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 190 225 20 1060353 0 0 32767}" )
   mygump.addRawLayout( "{text 275 190 1152 14}" )
   mygump.addRawLayout( "{text 350 190 1152 15}" )
   mygump.addRawLayout( "{text 425 190 1152 16}" )
   mygump.addRawLayout( "{button 10 210 4005 4007 1 0 36}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 210 225 20 1060354 0 0 32767}" )
   mygump.addRawLayout( "{text 275 210 1152 19}" )
   mygump.addRawLayout( "{text 350 210 1152 20}" )
   mygump.addRawLayout( "{text 425 210 1152 21}" )
   mygump.addRawLayout( "{button 10 230 4005 4007 1 0 37}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 230 225 20 1060355 0 0 32767}" )
   mygump.addRawLayout( "{text 275 230 1152 19}" )
   mygump.addRawLayout( "{text 350 230 1152 20}" )
   mygump.addRawLayout( "{text 425 230 1152 25}" )
   mygump.addRawLayout( "{button 10 250 4005 4007 1 0 38}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 250 225 20 1060356 0 0 32767}" )
   mygump.addRawLayout( "{text 275 250 1152 26}" )
   mygump.addRawLayout( "{text 350 250 1152 27}" )
   mygump.addRawLayout( "{text 425 250 1152 32}" )
   mygump.addRawLayout( "{button 10 270 4005 4007 1 0 39}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 270 225 20 1060357 0 0 32767}" )
   mygump.addRawLayout( "{text 275 270 1152 26}" )
   mygump.addRawLayout( "{text 350 270 1152 27}" )
   mygump.addRawLayout( "{text 425 270 1152 36}" )
   mygump.addRawLayout( "{button 10 290 4005 4007 1 0 40}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 290 225 20 1060358 0 0 32767}" )
   mygump.addRawLayout( "{text 275 290 1152 26}" )
   mygump.addRawLayout( "{text 350 290 1152 27}" )
   mygump.addRawLayout( "{text 425 290 1152 39}" )
   mygump.addRawLayout( "{button 10 310 4005 4007 1 0 41}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 310 225 20 1060359 0 0 32767}" )
   mygump.addRawLayout( "{text 275 310 1152 26}" )
   mygump.addRawLayout( "{text 350 310 1152 27}" )
   mygump.addRawLayout( "{text 425 310 1152 40}" )
   mygump.addRawLayout( "{button 10 330 4005 4007 1 0 42}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 330 225 20 1060360 0 0 32767}" )
   mygump.addRawLayout( "{text 275 330 1152 26}" )
   mygump.addRawLayout( "{text 350 330 1152 27}" )
   mygump.addRawLayout( "{text 425 330 1152 41}" )
   mygump.addRawLayout( "{button 450 390 4005 4007 0 4 0}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 400 390 100 20 3000406 0 0 32767}" )
   mygump.addRawLayout( "{page 4}" )
   mygump.addRawLayout( "{button 200 390 4014 4016 0 3 0}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 250 390 100 20 3000405 0 0 32767}" )
   mygump.addRawLayout( "{button 10 70 4005 4007 1 0 43}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 70 225 20 1060366 0 0 32767}" )
   mygump.addRawLayout( "{text 275 70 1152 19}" )
   mygump.addRawLayout( "{text 350 70 1152 20}" )
   mygump.addRawLayout( "{text 425 70 1152 22}" )
   mygump.addRawLayout( "{button 10 90 4005 4007 1 0 44}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 90 225 20 1060367 0 0 32767}" )
   mygump.addRawLayout( "{text 275 90 1152 26}" )
   mygump.addRawLayout( "{text 350 90 1152 27}" )
   mygump.addRawLayout( "{text 425 90 1152 28}" )
   mygump.addRawLayout( "{button 10 110 4005 4007 1 0 45}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 110 225 20 1060368 0 0 32767}" )
   mygump.addRawLayout( "{text 275 110 1152 26}" )
   mygump.addRawLayout( "{text 350 110 1152 27}" )
   mygump.addRawLayout( "{text 425 110 1152 33}" )
   mygump.addRawLayout( "{button 10 130 4005 4007 1 0 46}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 130 225 20 1060369 0 0 32767}" )
   mygump.addRawLayout( "{text 275 130 1152 26}" )
   mygump.addRawLayout( "{text 350 130 1152 27}" )
   mygump.addRawLayout( "{text 425 130 1152 37}" )
   mygump.addRawLayout( "{button 10 150 4005 4007 1 0 47}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 150 225 20 1060370 0 0 32767}" )
   mygump.addRawLayout( "{text 275 150 1152 26}" )
   mygump.addRawLayout( "{text 350 150 1152 27}" )
   mygump.addRawLayout( "{text 425 150 1152 40}" )
   mygump.addRawLayout( "{button 10 170 4005 4007 1 0 48}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 170 225 20 1060371 0 0 32767}" )
   mygump.addRawLayout( "{text 275 170 1152 26}" )
   mygump.addRawLayout( "{text 350 170 1152 27}" )
   mygump.addRawLayout( "{text 425 170 1152 42}" )
   mygump.addRawLayout( "{button 10 190 4005 4007 1 0 49}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 190 225 20 1060372 0 0 32767}" )
   mygump.addRawLayout( "{text 275 190 1152 26}" )
   mygump.addRawLayout( "{text 350 190 1152 27}" )
   mygump.addRawLayout( "{text 425 190 1152 43}" )
   mygump.addRawLayout( "{button 10 210 4005 4007 1 0 50}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 210 225 20 1060379 0 0 32767}" )
   mygump.addRawLayout( "{text 275 210 1152 26}" )
   mygump.addRawLayout( "{text 350 210 1152 27}" )
   mygump.addRawLayout( "{text 425 210 1152 29}" )
   mygump.addRawLayout( "{button 10 230 4005 4007 1 0 51}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 230 225 20 1060380 0 0 32767}" )
   mygump.addRawLayout( "{text 275 230 1152 26}" )
   mygump.addRawLayout( "{text 350 230 1152 27}" )
   mygump.addRawLayout( "{text 425 230 1152 34}" )
   mygump.addRawLayout( "{button 10 250 4005 4007 1 0 52}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 250 225 20 1060381 0 0 32767}" )
   mygump.addRawLayout( "{text 275 250 1152 26}" )
   mygump.addRawLayout( "{text 350 250 1152 27}" )
   mygump.addRawLayout( "{text 425 250 1152 38}" )
   mygump.addRawLayout( "{button 10 270 4005 4007 1 0 53}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 270 225 20 1060382 0 0 32767}" )
   mygump.addRawLayout( "{text 275 270 1152 26}" )
   mygump.addRawLayout( "{text 350 270 1152 27}" )
   mygump.addRawLayout( "{text 425 270 1152 41}" )
   mygump.addRawLayout( "{button 10 290 4005 4007 1 0 54}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 290 225 20 1060383 0 0 32767}" )
   mygump.addRawLayout( "{text 275 290 1152 26}" )
   mygump.addRawLayout( "{text 350 290 1152 27}" )
   mygump.addRawLayout( "{text 425 290 1152 43}" )
   mygump.addRawLayout( "{button 10 310 4005 4007 1 0 55}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 50 310 225 20 1060384 0 0 32767}" )
   mygump.addRawLayout( "{text 275 310 1152 26}" )
   mygump.addRawLayout( "{text 350 310 1152 27}" )
   mygump.addRawLayout( "{text 425 310 1152 44}" )

   #Add params
   for line in gump_params:
      mygump.addRawText( line )

   mygump.setArgs( [chsizes] )
   mygump.setCallback( callback )
   mygump.send( char )
   

def gump2( char, callback, args, target ):
   mygump = cGump( 1 )
   mygump.startPage( 0 )
   mygump.addBackground( 5054, 420, 280 )
   mygump.addTiledGump( 10, 10, 400, 20, 2624 )
   mygump.addCheckerTrans( 10, 10, 400, 20 )
   mygump.addXmfHtmlGump( 10, 10, 400, 20, 1060635, 0, 0, 30720 )
   mygump.addTiledGump( 10, 40, 400, 200, 2624 )
   mygump.addCheckerTrans( 10, 40, 400, 200 )
   mygump.addXmfHtmlGump( 10, 40, 400, 200, 1049583, 0, 1, 32512 )
   mygump.addTiledGump( 10, 250, 400, 20, 2624 )
   mygump.addCheckerTrans( 10, 250, 400, 20 )
   mygump.addButton( 10, 250, 4005, 4007, button_ok )
   mygump.addXmfHtmlGump( 40, 250, 170, 20, 1011036, 0, 0, 32767 )
   mygump.addButton( 210, 250, 4005, 4007, button_cancel )
   mygump.addXmfHtmlGump( 240, 250, 170, 20, 1011012, 0, 0, 32767 )
   mygump.setCallback( callback )
   mygump.setArgs( [args, target] )
   mygump.send( char )
   
