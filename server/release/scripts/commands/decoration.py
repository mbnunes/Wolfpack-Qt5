#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: Correa                         #
#   )).-' {{ ;'`   | Revised by: BtbN                           #
#  ( (  ;._ \\ ctr | Last Modification: Added savedoors         #
#===============================================================#

r"""
	\command decoration
	\description Places decoration read from a xml template found
	in wolfpack's data folder.
	\usage - <code>decoration save</code>
	- <code>decoration savedoors</code>
	- <code>decoration load &lt;map&gt;</code>
	
	Where map is the map you want to decorate (0-5).
	\notes The decoration will be read from a XML files called
	'data\decoration.?.xml, where ? is the map number'.
r"""

import wolfpack
from wolfpack.utilities import hex2dec
from wolfpack import tr, console
from wolfpack.consts import LOG_ERROR, LOG_MESSAGE, WPDT_MULTI
import os
import xml.sax
from xml.sax.handler import *

class DecorationHandler( ContentHandler ):
	def __init__(self):
		self.itemid = 0
		self.hue = 0
		self.amount = 0
		self.statements = []
		self.doLink = False
		self.linkTo = None

	def startElement( self, name, atts ):
		if name == "item":
			self.itemid = str(atts.getValue("id"));
			if "hue" in atts:
				self.hue = int(hex2dec(str(atts.getValue("hue"))));
			else:
				self.hue = 0
			if "amount" in atts:
				self.amount = int(atts.getValue("amount"));
			else:
				self.amount = 0
			self.statements = []
		elif name == "attribute":
			type = "str"
			if "type" in atts:
				type = str(atts.getValue("type"))
			if "value" in atts and "key" in atts:
				self.statements.append( str(atts.getValue("key")) + "," + type + ","+ str(atts.getValue("value")) )
		elif name == "pos":
			try:
				dec_id = hex2dec( self.itemid )
				if dec_id <= 0:
					return
			except:
				return

			hex_id = "%x" % dec_id
			if wolfpack.getdefinition( WPDT_MULTI, hex_id ):
				item = wolfpack.addmulti( hex_id )
			else:
				item = wolfpack.additem( hex_id )
				if not item and dec_id >= 0x4000:
					item = wolfpack.addmulti( hex_id )

			if not item or item == None:
				return
			if self.hue > 0:
				item.color = self.hue
			if self.amount > 0:
				item.amount = self.amount
			for p in self.statements:
				parts = p.split(",")
				if hasattr(item, parts[0]):
					if parts[1] == "str":
						value = parts[2]
					elif parts[1] == "int":
						value = int(parts[2])
					setattr(item, parts[0], value)

			x = int( atts.getValue("x") )
			y = int( atts.getValue("y") )
			z = int( atts.getValue("z") )
			map = int( atts.getValue("map") )
			item.moveto( x, y, z, map )
			item.movable = 3 # not movable
			item.decay = 0 # no decay
			item.update()

			if item.hasscript("door"):
				nearby = wolfpack.items(x, y, map, 1)
				for other in nearby:
					if other != item and other.hasscript("door") and other.pos.z == item.pos.z:
						if not item.hastag("link") and not other.hastag("link"):
							item.settag("link", other.serial)
							other.settag("link", item.serial)
							break
		elif name == "include":
			path = atts.getValue("file")
			if not os.path.isfile(path):
				console.log(LOG_ERROR, tr("File '%s' not found.\n") % (path))
				return
			parser = xml.sax.make_parser()
			handler = DecorationHandler()
			parser.setContentHandler(handler)
			parser.parse(path)


class DecorationSaveHandler:
	def __init__( self ):
		self.maps = {}

	def sort( self ):
		it = wolfpack.itemiterator()
		item = it.first
		while item:
			if not self.filter(item):
				if not item.pos.map in self.maps:
					self.maps[item.pos.map] = {}
				if not item.id in self.maps[item.pos.map]:
					self.maps[item.pos.map][item.id] = []
				self.maps[item.pos.map][item.id].append(item)
			item = it.next

	def filter( self, item ):
		if item.multi or item.newbie or item.free or item.spawnregion or item.hasscript("door"):
			return True
		return False

	def save( self ):
		for map in self.maps:
			file = open( "save_decoration.%i.xml" % map, "w" )
			file.write("<decoration>\n")
			for id in self.maps[map]:
				itemsbyhue = {}
				for item in self.maps[map][id]:
					if not item.color in itemsbyhue:
						itemsbyhue[item.color] = []
					itemsbyhue[item.color].append(item)
				tiledata = wolfpack.tiledata(id)
				for hue in itemsbyhue:
					file.write("""\t<!-- %s -->\n""" % tiledata["name"] )
					if hue != 0:
						file.write("""\t<item id="0x%x" hue="0x%x">\n""" % (id, hue) )
					else:
						file.write("""\t<item id="0x%x">\n""" % id )
					for item in itemsbyhue[hue]:
						pos = item.pos
						file.write("""\t\t<pos x="%i" y="%i" z="%i" map="%i" />\n""" % (pos.x, pos.y, pos.z, pos.map) )
					file.write("\t</item>\n")
			file.write("</decoration>\n")
			file.close()


class DoorSaveHandler:
	def __init__( self ):
		self.maps = {}

	def sort( self ):
		it = wolfpack.itemiterator()
		item = it.first
		while item:
			if not self.filter(item):
				if not item.pos.map in self.maps:
					self.maps[item.pos.map] = {}
				if not item.id in self.maps[item.pos.map]:
					self.maps[item.pos.map][item.id] = []
				self.maps[item.pos.map][item.id].append(item)
			item = it.next

	def filter( self, item ):
		if item.hasscript("door") and not item.hasscript("housing.security"):
			return False
		return True

	def save( self ):
		for map in self.maps:
			file = open( "save_doors.%i.xml" % map, "w" )
			file.write("<decoration>\n")
			for id in self.maps[map]:
				itemsbyhue = {}
				for item in self.maps[map][id]:
					if not item.color in itemsbyhue:
						itemsbyhue[item.color] = []
					itemsbyhue[item.color].append(item)
				tiledata = wolfpack.tiledata(id)
				for hue in itemsbyhue:
					file.write("""\t<!-- %s -->\n""" % tiledata["name"] )
					if hue != 0:
						file.write("""\t<item id="0x%x" hue="0x%x">\n""" % (id, hue) )
					else:
						file.write("""\t<item id="0x%x">\n""" % id )
					for item in itemsbyhue[hue]:
						pos = item.pos
						file.write("""\t\t<pos x="%i" y="%i" z="%i" map="%i" />\n""" % (pos.x, pos.y, pos.z, pos.map) )
					file.write("\t</item>\n")
			file.write("</decoration>\n")
			file.close()



def load_decoration(map_id, socket=None):
	if not wolfpack.hasmap(map_id):
		msg = tr("Map %i does not exist.") % map_id
		console.log(LOG_ERROR, msg + "\n")
		if socket:
			socket.sysmessage(msg)
		return
	filename = "definitions/decoration/decoration.%i.xml" % map_id
	if not os.path.isfile(filename):
		msg = tr("Decoration file '%s' not found.") % filename
		console.log(LOG_ERROR, msg + "\n")
		if socket:
			socket.sysmessage(msg)
		return
	msg = tr("Decorating map %i, please wait...") % map_id
	console.log(LOG_MESSAGE, msg + "\n")
	if socket:
		socket.sysmessage(msg)
	parser = xml.sax.make_parser()
	handler = DecorationHandler()
	parser.setContentHandler(handler)
	parser.parse(filename)
	done_msg = tr("Done decorating map %i.") % map_id
	console.log(LOG_MESSAGE, done_msg + "\n")
	if socket:
		socket.sysmessage(done_msg)

def decoration( socket, command, arguments ):
	if len(arguments) > 0:
		args = str(arguments.split()[0])
		if args == 'save':
			saveObject = DecorationSaveHandler()
			socket.sysmessage(tr("Sorting items, please wait..."))
			saveObject.sort()
			socket.sysmessage(tr("Writing file..."))
			saveObject.save()
			socket.sysmessage(tr("Done."))
		elif args == 'savedoors':
			saveObject = DoorSaveHandler()
			socket.sysmessage(tr("Sorting doors, please wait..."))
			saveObject.sort()
			socket.sysmessage(tr("Writing file..."))
			saveObject.save()
			socket.sysmessage(tr("Done."))
		elif args == 'load':
			maps = []
			try:
				maponly = int(arguments.split()[1])
				maps.append(maponly)
			except (ValueError, IndexError):
				maps = [1, 0, 2, 3, 4, 5]
				pass
		
			for m in maps:
				if wolfpack.hasmap(m):
					load_decoration(m, socket)
		else:
			socket.sysmessage( tr('Usage: decoration save|savedoors|load') )
	else:
		socket.sysmessage( tr('Usage: decoration save|savedoors|load') )

def onLoad():
	wolfpack.registercommand( "decoration", decoration )
