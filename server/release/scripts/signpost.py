import wolfpack
from wolfpack.gumps import cGump



def onUse( char, item ):
   gump0( char, "signpost.gumpcallback", item )

def gumpcallback( char, args, target ):
   switchgump( char, target, args )


def gump0( char, callback, item ):
   gump_params = [
   "a house",
   "sign",
   "admin",
   "2003-05-22 17:57:40",
   "",
   "30500",
   "0"]

   mygump = cGump()
   mygump.addRawLayout( "{page 0}" )
   mygump.addRawLayout( "{resizepic 0 0 5054 420 440}" )
   mygump.addRawLayout( "{gumppictiled 10 10 400 100 2624}" )
   mygump.addRawLayout( "{checkertrans 10 10 400 100}" )
   mygump.addRawLayout( "{gumppictiled 10 120 400 260 2624}" )
   mygump.addRawLayout( "{checkertrans 10 120 400 260}" )
   mygump.addRawLayout( "{gumppictiled 10 390 400 40 2624}" )
   mygump.addRawLayout( "{checkertrans 10 390 400 40}" )
   mygump.addRawLayout( "{button 250 410 4005 4007 1 0 0}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 285 410 240 20 1060675 0 0 32767}" )
   mygump.addRawLayout( "{gumppic 10 10 100}" )
   mygump.addRawLayout( "{text 62 43 0 0}" )
   mygump.addRawLayout( "{text 74 57 0 1}" )
   mygump.addRawLayout( "{button 10 390 4005 4007 1 0 31}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 45 390 240 20 1060676 0 0 32767}" )
   mygump.addRawLayout( "{button 10 410 4005 4007 1 0 46}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 45 410 240 20 1060677 0 0 32767}" )
   mygump.addRawLayout( "{button 150 10 4006 4007 1 0 2}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 195 10 200 20 1060668 0 0 16927}" )
   mygump.addRawLayout( "{button 150 30 4005 4007 1 0 17}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 195 30 200 20 1060669 0 0 32767}" )
   mygump.addRawLayout( "{button 150 50 4005 4007 1 0 32}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 195 50 200 20 1060670 0 0 32767}" )
   mygump.addRawLayout( "{button 150 70 4005 4007 1 0 47}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 195 70 200 20 1060671 0 0 32767}" )
   mygump.addRawLayout( "{button 150 90 4005 4007 1 0 62}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 195 90 200 20 1060672 0 0 32767}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 20 130 200 20 1011242 0 0 32767}" )
   mygump.addRawLayout( "{text 210 130 1152 2}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 20 170 380 20 1018032 0 0 16927}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 20 190 380 20 1018035 0 0 16927}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 20 210 380 20 1060681 0 0 16927}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 20 230 380 20 1060679 0 0 16927}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 20 250 380 20 1062209 0 0 16927}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 20 290 200 20 1060692 0 0 16927}" )
   mygump.addRawLayout( "{text 250 290 1152 3}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 20 310 200 20 1060693 0 0 16927}" )
   mygump.addRawLayout( "{text 250 310 1152 4}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 20 330 200 20 1061793 0 0 16927}" )
   mygump.addRawLayout( "{text 250 330 1152 5}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 20 360 300 20 1011241 0 0 16927}" )
   mygump.addRawLayout( "{text 350 360 1152 6}" )
   mygump.setCallback( callback )
   mygump.setArgs( [item] )
   #Add params
   for line in gump_params:
      mygump.addRawText( line )
   mygump.send( char )

def gump1( char, callback, item ):
   gump_params = [
   "a house",
   "sign"]

   mygump = cGump()
   mygump.addRawLayout( "{page 0}" )
   mygump.addRawLayout( "{resizepic 0 0 5054 420 440}" )
   mygump.addRawLayout( "{gumppictiled 10 10 400 100 2624}" )
   mygump.addRawLayout( "{checkertrans 10 10 400 100}" )
   mygump.addRawLayout( "{gumppictiled 10 120 400 260 2624}" )
   mygump.addRawLayout( "{checkertrans 10 120 400 260}" )
   mygump.addRawLayout( "{gumppictiled 10 390 400 40 2624}" )
   mygump.addRawLayout( "{checkertrans 10 390 400 40}" )
   mygump.addRawLayout( "{button 250 410 4005 4007 1 0 0}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 285 410 240 20 1060675 0 0 32767}" )
   mygump.addRawLayout( "{gumppic 10 10 100}" )
   mygump.addRawLayout( "{text 62 43 0 0}" )
   mygump.addRawLayout( "{text 74 57 0 1}" )
   mygump.addRawLayout( "{button 10 390 4005 4007 1 0 31}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 45 390 240 20 1060676 0 0 32767}" )
   mygump.addRawLayout( "{button 10 410 4005 4007 1 0 46}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 45 410 240 20 1060677 0 0 32767}" )
   mygump.addRawLayout( "{button 150 10 4005 4007 1 0 2}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 195 10 200 20 1060668 0 0 32767}" )
   mygump.addRawLayout( "{button 150 30 4006 4007 1 0 17}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 195 30 200 20 1060669 0 0 16927}" )
   mygump.addRawLayout( "{button 150 50 4005 4007 1 0 32}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 195 50 200 20 1060670 0 0 32767}" )
   mygump.addRawLayout( "{button 150 70 4005 4007 1 0 47}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 195 70 200 20 1060671 0 0 32767}" )
   mygump.addRawLayout( "{button 150 90 4005 4007 1 0 62}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 195 90 200 20 1060672 0 0 32767}" )
   mygump.addRawLayout( "{button 10 130 4005 4007 1 0 4}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 45 130 240 20 1011266 0 0 32767}" )
   mygump.addRawLayout( "{button 10 150 4005 4007 1 0 19}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 45 150 240 20 1011267 0 0 32767}" )
   mygump.addRawLayout( "{button 10 170 4005 4007 1 0 34}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 45 170 240 20 1018036 0 0 32767}" )
   mygump.addRawLayout( "{button 10 190 4005 4007 1 0 49}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 45 190 240 20 1011268 0 0 32767}" )
   mygump.addRawLayout( "{button 10 220 4005 4007 1 0 64}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 45 220 240 20 1011243 0 0 32767}" )
   mygump.addRawLayout( "{button 10 240 4005 4007 1 0 79}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 45 240 240 20 1011244 0 0 32767}" )
   mygump.addRawLayout( "{button 10 260 4005 4007 1 0 94}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 45 260 240 20 1018037 0 0 32767}" )
   mygump.addRawLayout( "{button 10 280 4005 4007 1 0 109}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 45 280 240 20 1011245 0 0 32767}" )
   mygump.addRawLayout( "{button 10 310 4005 4007 1 0 154}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 45 310 240 20 1060699 0 0 32767}" )
   mygump.addRawLayout( "{button 10 330 4005 4007 1 0 169}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 45 330 240 20 1060700 0 0 32767}" )
   mygump.addRawLayout( "{button 210 130 4005 4007 1 0 199}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 245 130 240 20 1060694 0 0 32767}" )                   
   mygump.setCallback( callback )   
   mygump.setArgs( [item] )
   #Add params
   for line in gump_params:
      mygump.addRawText( line )
   mygump.send( char )
   
   
def gump2( char, callback, item ):
   gump_params = [
   "a house",
   "sign",
   "0",
   "425",
   "212"]

   mygump = cGump()   
   mygump.addRawLayout( "{page 0}" )
   mygump.addRawLayout( "{resizepic 0 0 5054 420 440}" )
   mygump.addRawLayout( "{gumppictiled 10 10 400 100 2624}" )
   mygump.addRawLayout( "{checkertrans 10 10 400 100}" )
   mygump.addRawLayout( "{gumppictiled 10 120 400 260 2624}" )
   mygump.addRawLayout( "{checkertrans 10 120 400 260}" )
   mygump.addRawLayout( "{gumppictiled 10 390 400 40 2624}" )
   mygump.addRawLayout( "{checkertrans 10 390 400 40}" )
   mygump.addRawLayout( "{button 250 410 4005 4007 1 0 0}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 285 410 240 20 1060675 0 0 32767}" )
   mygump.addRawLayout( "{gumppic 10 10 100}" )
   mygump.addRawLayout( "{text 62 43 0 0}" )
   mygump.addRawLayout( "{text 74 57 0 1}" )
   mygump.addRawLayout( "{button 10 390 4005 4007 1 0 31}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 45 390 240 20 1060676 0 0 32767}" )
   mygump.addRawLayout( "{button 10 410 4005 4007 1 0 46}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 45 410 240 20 1060677 0 0 32767}" )
   mygump.addRawLayout( "{button 150 10 4005 4007 1 0 2}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 195 10 200 20 1060668 0 0 32767}" )
   mygump.addRawLayout( "{button 150 30 4005 4007 1 0 17}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 195 30 200 20 1060669 0 0 32767}" )
   mygump.addRawLayout( "{button 150 50 4006 4007 1 0 32}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 195 50 200 20 1060670 0 0 16927}" )
   mygump.addRawLayout( "{button 150 70 4005 4007 1 0 47}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 195 70 200 20 1060671 0 0 32767}" )
   mygump.addRawLayout( "{button 150 90 4005 4007 1 0 62}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 195 90 200 20 1060672 0 0 32767}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 10 130 400 20 1060682 0 0 32767}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 10 170 275 20 1011237 0 0 32767}" )
   mygump.addRawLayout( "{text 310 170 1152 2}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 10 190 275 20 1011238 0 0 32767}" )
   mygump.addRawLayout( "{text 310 190 1152 3}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 10 210 275 20 1011239 0 0 32767}" )
   mygump.addRawLayout( "{text 310 210 1152 2}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 10 230 275 20 1011240 0 0 32767}" )
   mygump.addRawLayout( "{text 310 230 1152 4}" )
   mygump.setCallback( callback )
   mygump.setArgs( [item] )
   #Add params
   for line in gump_params:
      mygump.addRawText( line )
   mygump.send( char )


def gump3( char, callback, item ):
   gump_params = [
   "a house",
   "sign"]

   mygump = cGump()
   mygump.addRawLayout( "{page 0}" )
   mygump.addRawLayout( "{resizepic 0 0 5054 420 440}" )
   mygump.addRawLayout( "{gumppictiled 10 10 400 100 2624}" )
   mygump.addRawLayout( "{checkertrans 10 10 400 100}" )
   mygump.addRawLayout( "{gumppictiled 10 120 400 260 2624}" )
   mygump.addRawLayout( "{checkertrans 10 120 400 260}" )
   mygump.addRawLayout( "{gumppictiled 10 390 400 40 2624}" )
   mygump.addRawLayout( "{checkertrans 10 390 400 40}" )
   mygump.addRawLayout( "{button 250 410 4005 4007 1 0 0}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 285 410 240 20 1060675 0 0 32767}" )
   mygump.addRawLayout( "{gumppic 10 10 100}" )
   mygump.addRawLayout( "{text 62 43 0 0}" )
   mygump.addRawLayout( "{text 74 57 0 1}" )
   mygump.addRawLayout( "{button 10 390 4005 4007 1 0 31}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 45 390 240 20 1060676 0 0 32767}" )
   mygump.addRawLayout( "{button 10 410 4005 4007 1 0 46}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 45 410 240 20 1060677 0 0 32767}" )
   mygump.addRawLayout( "{button 150 10 4005 4007 1 0 2}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 195 10 200 20 1060668 0 0 32767}" )
   mygump.addRawLayout( "{button 150 30 4005 4007 1 0 17}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 195 30 200 20 1060669 0 0 32767}" )
   mygump.addRawLayout( "{button 150 50 4005 4007 1 0 32}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 195 50 200 20 1060670 0 0 32767}" )
   mygump.addRawLayout( "{button 150 70 4006 4007 1 0 47}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 195 70 200 20 1060671 0 0 16927}" )
   mygump.addRawLayout( "{button 150 90 4005 4007 1 0 62}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 195 90 200 20 1060672 0 0 32767}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 45 120 240 20 1060759 0 0 16912}" )
   mygump.addRawLayout( "{button 10 160 4005 4007 1 0 21}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 45 160 240 20 1060765 0 0 32767}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 45 180 240 20 1060760 0 0 16912}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 45 210 240 20 1060761 0 0 16912}" )
   mygump.addRawLayout( "{button 10 230 4005 4007 1 0 66}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 45 230 240 20 1060762 0 0 32767}" )
   mygump.addRawLayout( "{button 10 250 4005 4007 1 0 81}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 45 250 240 20 1060763 0 0 32767}" )
   mygump.addRawLayout( "{button 10 280 4005 4007 1 0 96}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 45 280 240 20 1062004 0 0 32767}" )
   mygump.addRawLayout( "{button 10 310 4005 4007 1 0 111}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 45 310 240 20 1060764 0 0 32767}" )
   mygump.setCallback( callback )
   mygump.setArgs( [item] )
   #Add params
   for line in gump_params:
      mygump.addRawText( line )

   mygump.send( char )

def gump4( char, callback, item ):
   gump_params = [
   "a house",
   "sign"]
   mygump = cGump()

   mygump.addRawLayout( "{page 0}" )
   mygump.addRawLayout( "{resizepic 0 0 5054 420 440}" )
   mygump.addRawLayout( "{gumppictiled 10 10 400 100 2624}" )
   mygump.addRawLayout( "{checkertrans 10 10 400 100}" )
   mygump.addRawLayout( "{gumppictiled 10 120 400 260 2624}" )
   mygump.addRawLayout( "{checkertrans 10 120 400 260}" )
   mygump.addRawLayout( "{gumppictiled 10 390 400 40 2624}" )
   mygump.addRawLayout( "{checkertrans 10 390 400 40}" )
   mygump.addRawLayout( "{button 250 410 4005 4007 1 0 0}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 285 410 240 20 1060675 0 0 32767}" )
   mygump.addRawLayout( "{gumppic 10 10 100}" )
   mygump.addRawLayout( "{text 62 43 0 0}" )
   mygump.addRawLayout( "{text 74 57 0 1}" )
   mygump.addRawLayout( "{button 10 390 4005 4007 1 0 31}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 45 390 240 20 1060676 0 0 32767}" )
   mygump.addRawLayout( "{button 10 410 4005 4007 1 0 46}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 45 410 240 20 1060677 0 0 32767}" )
   mygump.addRawLayout( "{button 150 10 4005 4007 1 0 2}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 195 10 200 20 1060668 0 0 32767}" )
   mygump.addRawLayout( "{button 150 30 4005 4007 1 0 17}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 195 30 200 20 1060669 0 0 32767}" )
   mygump.addRawLayout( "{button 150 50 4005 4007 1 0 32}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 195 50 200 20 1060670 0 0 32767}" )
   mygump.addRawLayout( "{button 150 70 4005 4007 1 0 47}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 195 70 200 20 1060671 0 0 32767}" )
   mygump.addRawLayout( "{button 150 90 4006 4007 1 0 62}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 195 90 200 20 1060672 0 0 16927}" )
   mygump.addRawLayout( "{button 10 130 4005 4007 1 0 7}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 45 130 240 20 1061794 0 0 32767}" )
   mygump.addRawLayout( "{button 10 150 4005 4007 1 0 22}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 45 150 240 20 1061797 0 0 32767}" )
   mygump.addRawLayout( "{xmfhtmlgumpcolor 45 190 240 20 1061798 0 0 16912}" )
   mygump.setCallback( callback )
   mygump.setArgs( [item] )
   #Add params
   for line in gump_params:
      mygump.addRawText( line )
   mygump.send( char )   

def customize( char, item ):
   multi = wolfpack.findmulti( item.morex )
   multi.sendcustomhouse( char )
   #char.socket.sysmessage( "Multi serial : %i" % multi.serial )
   char.moveto( wolfpack.coord( multi.pos.x, multi.pos.y, multi.pos.z+7, multi.pos.map ) )
   char.update()
   #woodenpost = wolfpack.finditem( item.morez )
   #woodenpost.delete()
   char.socket.customize( item )
   return

def switchgump( char, target, args ):
   item = args[0]
   gumphandler = {
   2:gump0,
   17:gump1,
   32:gump2,
   47:gump3,
   62:gump4 }  
   actionhandler = {
   21:customize }

   button = target.button
   if gumphandler.has_key( button ):
      gumphandler[ button ]( char, "signpost.gumpcallback", item )
   else: 
      if actionhandler.has_key( button ):
         actionhandler[ button ]( char, item )
      else:
         char.socket.sysmessage( "Button %i isn't working yet" % button )
         return 1
   
