
import wolfpack

def sendPage(char, serial, page, lines):
  packetlength = 13
  
  for line in lines:
    packetlength += len(line) + 1
  
  packet = wolfpack.packet(0x66, packetlength)
  packet.setshort(1, packetlength)
  packet.setint(3, serial)
  packet.setshort(7, 1) # Always a single page
  
  # Page Info
  packet.setshort(9, page)
  packet.setshort(11, len(lines))
  
  offset = 13
  for line in lines:
    packet.setascii(offset, line)
    offset += len(line) + 1

  packet.send(char.socket)

def onUse(char, item):
  char.objectdelay = 0

  # Send BookOpen packet
  # Author / Title
  author = ''
  title = item.name
  
  if item.hastag('author'):
    author = item.gettag('author')
      
  # Calculate Packet Length
  packetlength = 15 + len(author) + 1 + len(title) + 1
  
  packet = wolfpack.packet(0xd4, packetlength)
  packet.setshort(1, packetlength)     # Packet length
  packet.setint(3, item.serial)       # Book Serial
  
  if not item.hastag('protected'):
    packet.setbyte(7, 1) # Flag Writeable
    packet.setbyte(8, 1) # dito
    
  pages = 64
  if item.hastag('pages'):
    pages = int(item.gettag('pages'))
      
  packet.setshort(9, pages)
  
  packet.setshort(11, len(title) + 1)
  packet.setascii(13, title)
  
  packet.setshort(13 + len(title) + 1, len(author) + 1)
  packet.setascii(15 + len(title) + 1, author)

  packet.send(char.socket)

  if item.hastag('protected'):
    return 1

  # Send a packet for each page !!
  # We could easily create packets bigger than 65k otherwise...
  for page in range(1, pages+1):
    if item.hastag('page%u' % page):
      sendPage(char, item.serial, page, item.gettag('page%u' % page).split("\n"))
      
  return 1

def onBookRequestPage(char, item, page):
  pages = 64
  
  if item.hastag('pages'):
    pages = int(item.gettag('pages'))
    
  if page > pages or page == 0:
    return

  if item.hastag('page%u' % page):
    content = item.gettag('page%u' % page)
  else:
    content = ''
    
  lines = content.split("\n")
  
  sendPage(char, item.serial, page, lines)

# Request Update
def onBookUpdatePage(char, item, page, content):
  if item.hastag('protected'):
    char.message('This book is read only.')
    return
  
  if item.hastag('pages'):
    pages = int(item.gettag('pages'))
  else:
    pages = 64
    
  if page > pages or page < 1:
    return
  
  if not content or len(content.strip()) == 0:
    item.deltag('page%u' % page)
    return
    
  item.settag('page%u' % page, content)
  item.resendtooltip()

def onBookUpdateInfo(char, item, title, author):
  if item.hastag('protected'):
    char.message('This book is read only.')
    return

  if len(author) == 0:
    item.deltag('author')
  else:
    item.settag('author', author)

  item.name = title
  item.resendtooltip()
