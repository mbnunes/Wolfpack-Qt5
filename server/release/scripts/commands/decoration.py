#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: Correa                         #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#

"""
	\command decoration
	\description Places decoration read from a xml file.
	\notes The decoration will be read from a XML file called
	'data\decoration.1.xml'.
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
            item.update()
            item.decay = 0


def decoration( socket, command, arguments ):
    parser = xml.sax.make_parser()
    handler = DecorationHandler()
    parser.setContentHandler(handler)
    parser.parse("data/decoration.1.xml")

def onLoad():
	wolfpack.registercommand( "decoration", decoration )
