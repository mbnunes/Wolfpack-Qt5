import wolfpack.map
import wolfpack.console
import wolfpack.items
import wolfpack

# XML Handling stuff
#import sys
#from xml.dom.ext.reader import Sax2
#reader = HtmlLib.Reader()
#html_doc = reader.fromStream(stream)
#import xml.parsers.xmlproc.xmlproc
#import xml.parsers.xmlproc.xmlapp

import xml.dom.minidom

regions = ()
loaded = 0

# The pickaxe needs to be equipped
def onUse( Char, Item ):
	if( wolfpack.mining.loaded == 0 ):
		loadMining()

	if( ( Item.container == None ) or ( Item.container.serial != Char.serial ) ):
		Char.sysmessage( "This items needs to be equipped in order to be used" )
		return 1

	Char.sysmessage( "Where do you want to mine for ore?" )
	
	# Our pickaxe and our character need to be stored
	Char.requesttarget( "wolfpack.mining.onTarget", ( Char, Item ) )

	return 1 # We DO want to handle this item do we
	
# Argument list needs to match the argument list you pass to requesttarget
def onTarget( Target, Char, Pickaxe ):
	#Char.sysmessage( "Target: " + Target.target ) # Either "char", "item", "ground" or "static"
	
	if( Target.target == "ground" ):
		Char.sysmessage( "Creating item at the given coordinates" )
		myItem = wolfpack.items.add( "44", Target.x, Target.y, Target.z )
		#mapTile = wolfpack.map.gettile( Target.x, Target.y )
		#Char.sysmessage( "You targetted: " + str( mapTile ) )

def loadMining():
	# Read the mining.xml configuration file
	document = xml.dom.minidom.parse( "mining.xml" )
	orelist = document.getElementsByTagName( "ore" )
	
	wolfpack.console.send( "Found " + len( orelist ) + " ores" )
	
	for ore in orelist:
		id = ore.getAttribute( "id" )
		item = ore.getAttribute( "item" )
		minskill = ore.minSkill( "minskill" )
		
		wolfpack.console.send( "Found an ore " + id + " " + item + " " + minskill )
			
	document.unlink()	

# This event is only used in the webserver for example
#def onReload():
