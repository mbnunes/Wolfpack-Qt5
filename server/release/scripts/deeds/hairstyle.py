
from wolfpack.gumps import cGump
import wolfpack

def onShowTooltip(char, item, tooltip):
  tooltip.add(1041061, '')
  return 1

def onUse(char, deed):

  if not deed.container == char.getbackpack():
    char.socket.clilocmessage(1042001)
  else:
    gump = cGump(x=50, y=50)
    
    gump.addResizeGump(100, 10, 0xA28, 400, 385)
    
    #gump.addXmfHtmlGump(100, 25, 400, 35, 1013008, 0, 0)
    #gump.addButton(175, 340, 0xFA5, 0xFA7, 0)
    
    #gump.addXmfHtmlGump(210, 342, 90, 35, 1011012, 0, 0)
    
    #gump.addResizeGump(220, 60, 0xA3C, 50, 50)
    #gump.addResizeGump(220, 115, 0xA3C, 50, 50)
    #gump.addResizeGump(220, 170, 0xA3C, 50, 50)
    #gump.addResizeGump(220, 225, 0xA3C, 50, 50)
    #gump.addResizeGump(425, 60, 0xA3C, 50, 50)
    #gump.addResizeGump(425, 115, 0xA3C, 50, 50)
    #gump.addResizeGump(425, 170, 0xA3C, 50, 50)
    #gump.addResizeGump(425, 225, 0xA3C, 50, 50)
    #gump.addResizeGump(425, 280, 0xA3C, 50, 50)
    
    #gump.addXmfHtmlGump( 150, 75, 80, 35, 1011052, 0, 0 ) # Short
    #gump.addXmfHtmlGump( 150, 130, 80, 35, 1011053, 0, 0 ) # Long
    #gump.addXmfHtmlGump( 150, 185, 80, 35, 1011054, 0, 0 ) # Ponytail
    #gump.addXmfHtmlGump( 150, 240, 80, 35, 1011055, 0, 0 ) # Mohawk
    #gump.addXmfHtmlGump( 355, 75, 80, 35, 1011047, 0, 0 ) # Pageboy
    #gump.addXmfHtmlGump( 355, 130, 80, 35, 1011048, 0, 0 ) # Receding
    #gump.addXmfHtmlGump( 355, 185, 80, 35, 1011049, 0, 0 ) # 2-tails
    #gump.addXmfHtmlGump( 355, 240, 80, 35, 1011050, 0, 0 ) # Topknot
    #gump.addXmfHtmlGump( 355, 295, 80, 35, 1011064, 0, 0 ) # Bald
    
    gump.addTilePic( 153, 20, 0xC60C )
    #gump.addTilePic( 153, 65, 0xED24 )
    #gump.addTilePic( 153, 120, 0xED1E )
    #gump.addTilePic( 153, 185, 0xC60F )
    #gump.addTilePic( 358, 18, 0xED26 )
    #gump.addTilePic( 358, 75, 0xEDE5 )
    #gump.addTilePic( 358, 120, 0xED23 )
    #gump.addTilePic( 362, 190, 0xED29 )
    
    #gump.addButton(118, 73, 0xFA5, 0xFA7, 2)
    #gump.addButton(118, 128, 0xFA5, 0xFA7, 3)
    #gump.addButton(118, 183, 0xFA5, 0xFA7, 4)
    #gump.addButton(118, 238, 0xFA5, 0xFA7, 5)
    #gump.addButton(323, 73, 0xFA5, 0xFA7, 6)
    #gump.addButton(323, 128, 0xFA5, 0xFA7, 7)
    #gump.addButton(323, 183, 0xFA5, 0xFA7, 8)
    #gump.addButton(323, 238, 0xFA5, 0xFA7, 9)
    #gump.addButton(323, 292, 0xFA5, 0xFA7, 1)

    gump.send(char)

  return 1