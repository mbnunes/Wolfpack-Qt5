
from wolfpack.gumps import cGump
from wolfpack.consts import *
import wolfpack

def onShowTooltip(char, item, tooltip):
  tooltip.add(1041061, '')
  return 1

def response(char, args, response):
  if response.button == 0:
    char.socket.clilocmessage(1013009)
    return

  deed = wolfpack.finditem(args[0])

  if not deed or deed.container != char.getbackpack():
    char.socket.clilocmessage(1042001)
  else:
    oldhair = char.itemonlayer(LAYER_HAIR)
    oldcolor = 0

    if not oldhair and response.button == 1:
      char.socket.clilocmessage(1013010)
      return

    if oldhair:
      oldcolor = oldhair.color
      oldhair.delete() 

    newhair = None

    if response.button == 2:
      newhair = wolfpack.additem("203b")
    elif response.button == 3:
      newhair = wolfpack.additem("203c")
    elif response.button == 4:
      newhair = wolfpack.additem("203d")
    elif response.button == 5:
      newhair = wolfpack.additem("2044")
    elif response.button == 6:
      newhair = wolfpack.additem("2045")
    elif response.button == 7:
      newhair = wolfpack.additem("2048")
    elif response.button == 8:
      newhair = wolfpack.additem("2049")
    elif response.button == 9:
      newhair = wolfpack.additem("204a")

    if newhair:
      newhair.color = oldcolor
      char.additem(LAYER_HAIR, newhair)
      newhair.update()

    deed.delete()

def onUse(char, deed):
  if char.id != 0x190 or char.id != 0x191:
    char.socket.clilocmessage(1042298)
    return 1

  if not deed.container == char.getbackpack():
    char.socket.clilocmessage(1042001)
  else:
    char.socket.closegump(0x12feab08)

    gump = cGump(x=50, y=50, callback="deeds.hairstyle.response")
    gump.typeid = 0x12feab08
    gump.setArgs([deed.serial])
    
    gump.addResizeGump(100, 10, 0xA28, 400, 385)
    
    gump.addXmfHtmlGump(100, 25, 400, 35, 1013008, 0, 0)
    gump.addButton(175, 340, 0xFA5, 0xFA7, 0)
    
    gump.addXmfHtmlGump(210, 342, 90, 35, 1011012, 0, 0)
    
    gump.addResizeGump(220, 60, 0xA3C, 50, 50)
    gump.addResizeGump(220, 115, 0xA3C, 50, 50)
    gump.addResizeGump(220, 170, 0xA3C, 50, 50)
    gump.addResizeGump(220, 225, 0xA3C, 50, 50)
    gump.addResizeGump(425, 60, 0xA3C, 50, 50)
    gump.addResizeGump(425, 115, 0xA3C, 50, 50)
    gump.addResizeGump(425, 170, 0xA3C, 50, 50)
    gump.addResizeGump(425, 225, 0xA3C, 50, 50)
    gump.addResizeGump(425, 280, 0xA3C, 50, 50)
    
    gump.addXmfHtmlGump( 150, 75, 80, 35, 1011052, 0, 0 ) # Short
    gump.addXmfHtmlGump( 150, 130, 80, 35, 1011053, 0, 0 ) # Long
    gump.addXmfHtmlGump( 150, 185, 80, 35, 1011054, 0, 0 ) # Ponytail
    gump.addXmfHtmlGump( 150, 240, 80, 35, 1011055, 0, 0 ) # Mohawk
    gump.addXmfHtmlGump( 355, 75, 80, 35, 1011047, 0, 0 ) # Pageboy
    gump.addXmfHtmlGump( 355, 130, 80, 35, 1011048, 0, 0 ) # Receding
    gump.addXmfHtmlGump( 355, 185, 80, 35, 1011049, 0, 0 ) # 2-tails
    gump.addXmfHtmlGump( 355, 240, 80, 35, 1011050, 0, 0 ) # Topknot
    gump.addXmfHtmlGump( 355, 295, 80, 35, 1011064, 0, 0 ) # Bald
    
    gump.addGump( 153, 20, 0xC60C )
    gump.addGump( 153, 65, 0xED24 )
    gump.addGump( 153, 120, 0xED1E )
    gump.addGump( 153, 185, 0xC60F )
    gump.addGump( 358, 18, 0xED26 )
    gump.addGump( 358, 75, 0xEDE5 )
    gump.addGump( 358, 120, 0xED23 )
    gump.addGump( 362, 190, 0xED29 )
    
    gump.addButton(118, 73, 0xFA5, 0xFA7, 2)
    gump.addButton(118, 128, 0xFA5, 0xFA7, 3)
    gump.addButton(118, 183, 0xFA5, 0xFA7, 4)
    gump.addButton(118, 238, 0xFA5, 0xFA7, 5)
    gump.addButton(323, 73, 0xFA5, 0xFA7, 6)
    gump.addButton(323, 128, 0xFA5, 0xFA7, 7)
    gump.addButton(323, 183, 0xFA5, 0xFA7, 8)
    gump.addButton(323, 238, 0xFA5, 0xFA7, 9)
    gump.addButton(323, 292, 0xFA5, 0xFA7, 1)

    gump.send(char)

  return 1