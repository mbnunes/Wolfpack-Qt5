#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: DarkStorm                      #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#
# Python class for gumps.                     #
#===============================================================#

"""
	\library wolfpack.gumps
	\description Contains several functions for gumps.
"""

from types import *

class cGump:
  def __init__(self, noclose=0, nomove=0, nodispose=0, x=50, y=50, callback="", args = [], type = 0, serial = 0):
    self.layout = []
    self.texts = []
    self.args = args
    self.callback = callback
    self.noclose = noclose
    self.nomove = nomove
    self.nodispose = nodispose
    self.x = x
    self.y = y
    self.typeid = type
    self.serialid = serial

  def copy(othergump):
    new = cGump()
    new.layout = othergump.layout
    new.texts = othergump.texts
    new.args = othergump.args
    new.callback = othergump.callback
    new.noclose = othergump.noclose
    new.nomove = othergump.nomove
    new.nodispose = othergump.nodispose
    new.x = othergump.x
    new.y = othergump.y
    new.typeid = othergump.typeid
    new.serialid = othergump.serialid
    return new

  # Send the gump
  def send( self, char ):
    # There are two possibilities
    socket = None

    if type( char ).__name__ == "wpchar":
      socket = char.socket
    elif type( char ).__name__ == "wpsocket":
      socket = char
    else:
      raise TypeError( "You passed an invalid socket." )

    # Dump the gump data
    #file = open( 'dump.txt', 'w' )
    #file.write( "--------\nLayout:\n" )
    #for line in self.layout:
    # file.write( line + "\n" )
    #file.write( "--------\nText:\n" )
    #for line in self.texts:
    # file.write( line + "\n" )
    #file.close()

    if( self.noclose == 1 ):
      self.layout.insert( 0, '{ noclose }' )

    if( self.nodispose == 1 ):
      self.layout.insert( 0, '{ nodispose }' )

    if( self.nomove == 1 ):
      self.layout.insert( 0, '{ nomove }' )

    socket.sendgump( self.x, self.y, 0, 0, 0, self.serialid, self.typeid, self.layout, self.texts, self.callback, self.args )

  # For "rawly" modifying the list
  def addRawLayout( self, data ):
    self.layout.append( data )

  def addRawText( self, data ):
    # Find the text
    if data in self.texts:
      return self.texts.index(data)
    else:
    # Insert the text
      self.texts.append(data)
      return len(self.texts) - 1

  # Sets the Callback function which is going to be called whenever the user
  # clicks something on the gump
  def setCallback( self, callback ):
    if not type( callback ) is StringType:
      raise TypeError( "You have to pass a string to setCallback" )
    else:
      self.callback = callback

  # Set the arguments you want to pass to the gump-response handler later on
  def setArgs( self, args ):
    if not type( args ) is ListType:
      raise TypeError( "You have to pass a list to setArgs" )
    else:
      self.args = args

  # Set the Gump-Type (this can be used to close it later on)
  def setType( self, typeid ):
    if not type( typeid ) is IntType:
      raise TypeError( "You have to pass an integer to setType" )
    else:
      self.typeid = typeid

  # Set the Gump-Serial (this can be used to only open one gump of the same type at the same moment)
  # This will auto-close the first gump
  # 0 means the Gump will automatically choose a serial
  def setSerial( self, serialid ):
    if not type( serialid ) is IntType:
      raise TypeError( "You have to pass an integer to setSerial" )
    else:
      self.serialid = serialid

  ### Here are the helper functions for adding the layout elements
  def addTooltip(self, id):
    if not type(id) is IntType:
      raise TypeError("addTooltip only accepts cliloc ids.")
    self.layout.append('{tooltip %d}' % id)

  def startPage( self, page ):
    self.layout.append( "{page %u}" % page )

  def startGroup( self, groupid ):
    self.layout.append( "{group %u}" % groupid )

  def addText( self, x, y, text, hue=0 ):
    self.layout.append("{text %i %i %u %u}" % (x, y, hue, self.addRawText(text)))

  def addBackground( self, id, width, height ):
    self.addResizeGump( 0, 0, id, width, height )

  def addResizeGump( self, x, y, id, width, height ):
    self.layout.append( "{resizepic %i %i %u %u %u}" % ( x, y, id, width, height ) )

  def addCroppedText( self, x, y, width, height, text, hue=0 ):
    self.layout.append( "{croppedtext %i %i %u %u %u %u}" % ( x, y, width, height, hue, self.addRawText( text ) ) )

  def addButton( self, x, y, up, down, returncode ):
    self.layout.append( "{button %i %i %u %u 1 0 %u}" % ( x, y, up, down, returncode ) )

  def addPageButton( self, x, y, up, down, page ):
    self.layout.append( "{button %i %i %u %u 0 %u 0}" % ( x, y, up, down, page ) )

  def addGump( self, x, y, id, hue = -1 ):
    # A non-colored gump
    if hue == -1:
      self.layout.append( "{gumppic %i %i %u}" % ( x, y, id ) )
    else:
      self.layout.append( "{gumppic %i %i %u hue=%u}" % ( x, y, id, hue ) )

  # Seems NOT hueable!
  def addTiledGump( self, x, y, width, height, id, hue = 0 ):
    if hue == -1:
      self.layout.append( "{gumppictiled %i %i %u %u %u}" % ( x, y, width, height, id ) )
    else:
      self.layout.append( "{gumppictiled %i %i %u %u %u hue=%u}" % ( x, y, width, height, id, hue ) )

  # Sadly it's not possible to hue this
  def addTilePic( self, x, y, id, hue = -1 ):
    self.layout.append( "{tilepic %i %i %u}" % ( x, y, id ) )

  def addInputField( self, x, y, width, height, hue, id, starttext ):
    self.layout.append( "{textentry %i %i %u %u %u %u %u}" % ( x, y, width, height, hue, id, self.addRawText( starttext ) ) )

  def addCheckbox( self, x, y, off, on, id, checked = 0 ):
    # Just to prevent errors
    if( checked != 0 ):
      checked = 1

    self.layout.append( "{checkbox %i %i %u %u %u %u}" % ( x, y, off, on, checked, id ) )

  def addRadioButton( self, x, y, off, on, id, selected = 0 ):
    # Just to prevent errors
    if( selected != 0 ):
      selected = 1

    self.layout.append( "{radio %i %i %u %u %u %u}" % ( x, y, off, on, selected, id ) )

  def addHtmlGump(self, x, y, width, height, html, hasBack = 0, canScroll = 0):
    if( canScroll != 0 ):
      canScroll = 1
    if( hasBack != 0 ):
      hasBack = 1

    self.layout.append( "{htmlgump %i %i %u %u %u %u %u}" % ( x, y, width, height, self.addRawText( html ), hasBack, canScroll ) )

  def addXmfHtmlGump( self, x, y, width, height, clilocid, hasBack = 0, canScroll = 0, color = 0 ):
    if( canScroll != 0 ):
      canScroll = 1
    if( hasBack != 0 ):
      hasBack = 1

    if color != 0:
      self.layout.append( "{xmfhtmlgumpcolor %i %i %u %u %u %u %u %u}" % ( x, y, width, height, clilocid, hasBack, canScroll, color ) )
    else:
      self.layout.append( "{xmfhtmlgump %i %i %u %u %u %u %u}" % ( x, y, width, height, clilocid, hasBack, canScroll ) )

  def addCheckerTrans( self, x, y, width, height ):
    self.layout.append( "{checkertrans %i %i %u %u}" % ( x, y, width, height ) )

  def add( self, line ):
    self.layout.append( line )



class WarningGump:

  def __init__(self, header, headerColor, content, contentColor, width, height, callback, state ):
    self.callback = callback
    self.state = state

    self.gump = cGump( 1, 0, 0, (640 - width) / 2, (480 - height) / 2 )
    self.gump.startPage( 0 )
    self.gump.addBackground( 5054, width, height )
    self.gump.addTiledGump( 10, 10, width - 20, 20, 2624 )
    self.gump.addCheckerTrans( 10, 10, width - 20, 20 )
    self.gump.addXmfHtmlGump( 10, 10, width - 20, 20, header, 0, 0, headerColor )
    self.gump.addTiledGump( 10, 40, width - 20, height - 80, 2624 )
    self.gump.addCheckerTrans( 10, 40, width - 20, height - 80 )
    if type( content ) is IntType:
      self.gump.addXmfHtmlGump( 10, 40, width - 20, height - 80, content, 0, 1, contentColor )
    else:
      self.gump.addHtmlGump( 10, 40, width - 20, height - 80, "<BASEFONT COLOR=#%x>%s</BASEFONT>" % ( contentColor, content ), 0, 1 )

    self.gump.addTiledGump( 10, height - 30, width - 20, 20, 2624 )
    self.gump.addCheckerTrans( 10, height - 30, width - 20, 20 )
    self.gump.addButton( 10, height - 30, 4005, 4007, 1 )
    self.gump.addXmfHtmlGump( 40, height - 30, 170, 20, 1011036, 0, 0, 32767 )

    self.gump.addButton( 10 + ((width - 20) / 2), height - 30, 4005, 4007, 0 )
    self.gump.addXmfHtmlGump( 40 + ((width - 20) / 2), height - 30, 170, 20, 1011012, 0, 0, 32767 )

    # set the callback

  def send( self, char ) :
    # There are two possibilities
    socket = None

    if type( char ).__name__ == "wpchar":
      socket = char.socket
    elif type( char ).__name__ == "wpsocket":
      socket = char
    else:
      raise TypeError( "You passed an invalid socket." )

    self.gump.setArgs([self.callback, self.state])
    self.gump.setCallback("wolfpack.gumps.WarningGump_onResponse")
    self.gump.send( socket )



def WarningGump_onResponse( player, args, choice ):
  socket = player.socket
  callback = args[0]
  state = args[1]
  if not callback:
    return
  if choice.button == 0 or not socket:
    callback( player, False, state )
  if choice.button == 1:
    callback( player, True, state )


class NoticeGump:

  def __init__(self, header, headerColor, content, contentColor, width, height, callback, state ):
    self.callback = callback
    self.state = state

    self.gump = cGump( 1, 0, 0, (640 - width) / 2, (480 - height) / 2 )
    self.gump.startPage( 0 )
    self.gump.addBackground( 5054, width, height )
    self.gump.addTiledGump( 10, 10, width - 20, 20, 2624 )
    self.gump.addCheckerTrans( 10, 10, width - 20, 20 )
    self.gump.addXmfHtmlGump( 10, 10, width - 20, 20, header, 0, 0, headerColor )
    self.gump.addTiledGump( 10, 40, width - 20, height - 80, 2624 )
    self.gump.addCheckerTrans( 10, 40, width - 20, height - 80 )
    if type( content ) is IntType:
      self.gump.addXmfHtmlGump( 10, 40, width - 20, height - 80, content, 0, 1, contentColor )
    else:
      self.gump.addHtmlGump( 10, 40, width - 20, height - 80, "<BASEFONT COLOR=#%x>%s</BASEFONT>" % ( contentColor, content ), 0, 1 )

    self.gump.addTiledGump( 10, height - 30, width - 20, 20, 2624 )
    self.gump.addCheckerTrans( 10, height - 30, width - 20, 20 )
    self.gump.addButton( 10, height - 30, 4005, 4007, 0 )
    self.gump.addXmfHtmlGump( 40, height - 30, 170, 20, 1011036, 0, 0, 32767 )

  def send( self, char ) :
    # There are two possibilities
    socket = None

    if type( char ).__name__ == "wpchar":
      socket = char.socket
    elif type( char ).__name__ == "wpsocket":
      socket = char
    else:
      raise TypeError( "You passed an invalid socket." )

    self.gump.setArgs([self.callback, self.state])
    self.gump.setCallback("wolfpack.gumps.NoticeGump_onResponse")
    self.gump.send( socket )


def NoticeGump_onResponse( player, args, choice ):
  socket = player.socket
  callback = args[0]
  state = args[1]
  if not callback:
    return
  if choice.button == 0 or not socket:
    callback( player, state )
