
import wolfpack
from wolfpack import console
from wolfpack.consts import *

def sendPage(socket, serial, page, lines):
	packetlength = 13

	for i in range(0, len(lines)):
		lines[i] = lines[i].encode('utf-8')
		packetlength += len(lines[i]) + 1

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

	packet.send(socket)

def onUse(char, item):
	char.objectdelay = 0

	# Send BookOpen packet
	# Author / Title
	author = ''
	title = item.name

	if item.hastag('author'):
		author = item.gettag('author')

	title = title.encode('utf-8')
	author = author.encode('utf-8')

	# Calculate Packet Length
	packetlength = 15 + len(author) + 1 + len(title) + 1

	packet = wolfpack.packet(0xd4, packetlength)
	packet.setshort(1, packetlength)		 # Packet length
	packet.setint(3, item.serial)			 # Book Serial

	if not item.hastag('protected'):
		packet.setbyte(7, 1) # Flag Writeable
		packet.setbyte(8, 1) # dito
		pass

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
			sendPage(char.socket, item.serial, page, item.gettag('page%u' % page).split("\n"))

	return 1

#
# The client either requested or updated a page.
#
def handlepage(socket, packet):
	item = wolfpack.finditem(packet.getint(3))

	if not item:
		console.log(LOG_ERROR, "Client tried to update non existing book %x." % (packet.getint(3)))
		return 1

	page = packet.getshort(9)
	linecount = packet.getshort(11)

	# A page was requested
	if linecount == -1:
		if item.hastag('pages'):
			pages = int(item.gettag('pages'))
		else:
			pages = 64

		if page > pages or page == 0:
			return

		if item.hastag('page%u' % page):
			content = item.gettag('page%u' % page).split("\n")
		else:
			content = []

		sendPage(socket, item.serial, page)

	# The client wants to update the page.
	else:
		if item.hastag('protected'):
			socket.sysmessage('This book is read only.')
			return 1

		if item.hastag('pages'):
			pages = int(item.gettag('pages'))
		else:
			pages = 64

		# Invalid page
		if page > pages or page < 1:
			socket.log("Client tried to update invalid page %d of book 0x%x.\n" % (page, item.serial))
			return 1

		offset = 13
		lines = []
		for i in range(0, linecount):
			line = packet.getascii(offset, 0)
			offset += len(line) + 1
			lines.append(unicode(line, 'utf-8'))

		content = "\n".join(lines)

		if not content or len(content.strip()) == 0:
			item.deltag('page%u' % page)
			return 1

		item.settag('page%u' % page, content)
		item.resendtooltip()

	return 1

#
# The client requested to update the information
# on the first page of this book.
#
def updatebook(socket, packet):
	item = wolfpack.finditem(packet.getint(3))

	if not item:
		console.log(LOG_ERROR, "Client tried to update non existing book %x." % (packet.getint(3)))
		return 1

	title_length = packet.getshort(11)
	title = ""
	for i in range(0, title_length):
		byte = packet.getbyte(13 + i)
		if byte > 0:
			title += chr(byte)
	title = unicode(title, 'utf-8')

	author_length = packet.getshort(13 + title_length)
	author = ""
	for i in range(0, author_length):
		byte = packet.getbyte(15 + title_length + i)
		if byte > 0:
			author += chr(byte)
	author = unicode(author, 'utf-8')

	if item.hastag('protected'):
		char.message('This book is read only.')
		return 1

	if len(author) == 0:
		item.deltag('author')
	else:
		item.settag('author', author)

	item.name = title
	item.resendtooltip()
	return 1

def onLoad():
	wolfpack.registerpackethook(0xD4, updatebook)
	wolfpack.registerpackethook(0x66, handlepage)
