#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: Correa                         #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#

"""
	\command decoration
	\description Places decoration read from a xml template found
	in wolfpack's data folder.
	\notes The decoration will be read from a XML files called
	'data\decoration.?.xml, where ? is the map number'.
"""

import wolfpack
from wolfpack.utilities import hex2dec
import os
import xml.sax
from xml.sax.handler import *

class DecorationHandler( ContentHandler ):
    def __init__(self):
        self.itemid = 0
        self.hue = 0
        self.amount = 0
        self.statements = []
    def startElement( self, name, atts ):
        if name == "item":
            self.itemid = str(atts.getValue("id"));
            if atts.has_key("hue"):
                self.hue = int(hex2dec(str(atts.getValue("hue"))));
            else:
                self.hue = 0
            if atts.has_key("amount"):
                self.amount = int(atts.getValue("amount"));
            else:
                self.amount = 0
            self.statements = []
        elif name == "attribute":
            type = "str"
            if atts.has_key("type"):
                type = str(atts.getValue("type"))
            if atts.has_key("value") and atts.has_key("key"):
                self.statements.append( str(atts.getValue("key")) + "," + type + ","+ str(atts.getValue("value")) )
        elif name == "pos":
            item = wolfpack.additem( "%x" %  hex2dec( self.itemid ) )
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
            item.update()
            item.decay = 0 # no decay


class DecorationSaveHandler:
    def __init__(self, socket):
        self.socket = socket
        self.maps = {}

    def sort( self ):
        self.socket.sysmessage("In sort()")
        it = wolfpack.itemiterator()
        item = it.first
        while item:
            if not self.filter(item):
                self.socket.sysmessage("Adding item")
                if not self.maps.has_key(item.pos.map):
                    self.maps[item.pos.map] = {}
                if not self.maps[item.pos.map].has_key(item.id):
                    self.maps[item.pos.map][item.id] = []
                self.maps[item.pos.map][item.id].append(item)
            item = it.next

    def filter( self, item ):
        if item.multi or item.newbie or item.free or item.spawnregion:
            return True

        return False

    def save( self ):
        for map in self.maps:
            file = open( "save_decoration.%i.xml" % map, "w" )
            file.write("<decoration>\n")
            for id in self.maps[map]:
                tiledata = wolfpack.tiledata(id)
                file.write("""\t<!-- %s -->\n""" % tiledata["name"] )
                file.write("""\t<item id="0x%x">\n""" % id )
                for item in self.maps[map][id]:
                    pos = item.pos
                    file.write("""\t\t<pos x="%i" y="%i" z="%i" map="%i">\n""" % (pos.x, pos.y, pos.z, pos.map) )
                file.write("\t</item>\n")
            file.write("</decoration>\n")
            file.close()





def decoration( socket, command, arguments ):
    if len(arguments) > 0:
        args = str(arguments)
        if args == 'save':
            saveObject = DecorationSaveHandler(socket)
            socket.sysmessage("Sorting items")
            saveObject.sort()
            saveObject.save()

    else:
        parser = xml.sax.make_parser()
        handler = DecorationHandler()
        parser.setContentHandler(handler)
        if wolfpack.hasmap(0):
            parser.parse("data/decoration.0.xml")
        if wolfpack.hasmap(1):
            parser.parse("data/decoration.1.xml")
        if wolfpack.hasmap(2):
            parser.parse("data/decoration.2.xml")
        if wolfpack.hasmap(3):
            parser.parse("data/decoration.3.xml")

def onLoad():
	wolfpack.registercommand( "decoration", decoration )
