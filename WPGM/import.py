
## Try 2
## Python Import Script
##

basepath = 'C:/Wolfpack/'
files = ['ancientrealms/index.xml', 'definitions.xml']

print "DELETE FROM categories;"
print "DELETE FROM items;"
print "DELETE FROM locationcategories;"
print "DELETE FROM locations;"

import sys
import xml.dom.minidom
from xml.parsers.expat import ExpatError

categories = {}
items = {}
lastcategory = 0

locations = {}
locationcategories = {}
lastlocationcategory = 0

def escapestring( string ):
	return string.replace( "'", "''" )

def hex2dec( string ):
	if string.lower().startswith( '0x' ):
		return int( string[2:], 16 )
	else:
		return int( string )

def processLocation(location, node):
	global lastlocationcategory
	
	category = node.getAttribute('category')
	
	if not category:
		return location
		
	location['categoryname'] = escapestring(category)
	
	# Remove everything until and including the last '\'
	category = category[:category.rfind('\\')]

	if not locationcategories.has_key(category):
		lastlocationcategory += 1
		locationcategories[category] = lastlocationcategory
		
	location['category'] = locationcategories[category]
	
	position = node.firstChild.nodeValue
	position = position.split(',')
	
	if len(position) >= 1:
		location['posx'] = int(position[0])
	if len(position) >= 2:
		location['posy'] = int(position[1])
	if len(position) >= 3:
		location['posz'] = int(position[2])
	if len(position) >= 4:
		location['posmap'] = int(position[3])
	
	return location

def processItem( item, node ):
	global lastcategory
	
	# Parse Node and modify item appropiately
	for element in node.childNodes:
		if element.nodeType != element.ELEMENT_NODE:
			continue
			
		if element.nodeName == 'id':
			if not element.firstChild or element.firstChild.nodeType != element.TEXT_NODE:
				continue
						
			item[ 'dispid' ] = hex2dec( element.firstChild.nodeValue )
		
		elif element.nodeName == 'name':
			if not element.firstChild or element.firstChild.nodeType != element.TEXT_NODE:
				continue
						
			item[ 'name' ] = escapestring( element.firstChild.nodeValue )
			
		elif element.nodeName == 'color':
			if element.firstChild and element.firstChild.nodeType == element.TEXT_NODE:
				item[ 'color' ] = hex2dec( element.firstChild.nodeValue )

		elif element.nodeName == 'category':
			if element.firstChild and element.firstChild.nodeType == element.TEXT_NODE:
				category = element.firstChild.nodeValue
				item[ 'categoryname' ] = escapestring( category )
				
				# Remove everything until and including the last '\'
				category = category[:category.rfind('\\')]
			
				if not categories.has_key( category ):
					lastcategory += 1
					categories[ category ] = lastcategory
					
				item[ 'category' ] = categories[ category ]

		# Inherit another node's properties
		elif element.nodeName == 'inherit':
			id = element.getAttribute( 'id' )
			
			if id and items.has_key( id ):
				try:
					item = processItem( item, items[ id ] )
				except:
					raise Exception, 'Error processing item id: ' + id
		
	return item

def parseFile( basepath, filename ):
	global items

	#print "Parsing " + filename

	try:
		dom = xml.dom.minidom.parse( basepath + filename )
	except:
		print "An Error occured while parsing " + filename + "."
		raise		
	
	# Safety Thing
	assert dom.documentElement.tagName == "definitions"
	
	for element in dom.documentElement.childNodes:
		if element.nodeType != element.ELEMENT_NODE:
			continue
		
		# Include Processing
		if element.nodeName == 'include':
			parseFile( basepath, element.getAttribute( 'file' ) )
		
		# Item Processing
		elif element.nodeName == 'item':
			id = element.getAttribute( 'id' )
			
			if not id:
				continue
		
			# Start Processing the Item before we insert it into our map
			items[ id ] = element.cloneNode( 1 )
		
		elif element.nodeName == 'location':
			id = element.getAttribute('id')
			if not id:
				continue
			locations[id] = element.cloneNode(1)
						
	dom.unlink() # Forced Cleanup
	
for file in files:
	parseFile(basepath, file)

processed = {}

for ( key, element ) in locations.items():
	location = {
		'id': key,
		'name': None,
		'posx': 0,
		'posy': 0,
		'posz': 0,
		'posmap': 0,
		'category': 0,
		'categoryname': None
	}
	
	location = processLocation(location, element)

	if not location['categoryname']:
		continue

	location['name'] = "'" + location[ 'categoryname' ][location['categoryname'].rfind('\\')+1:] + "'"

	#print "INSERT INTO items VALUES(NULL,%s,%u,%u,%u,'%s');" % ( item[ 'name' ], item[ 'category' ], item[ 'dispid' ], item[ 'color' ], item[ 'id' ] )
	print "INSERT INTO locations VALUES(NULL,%s,%u,%u,%u,%d,%u,'%s');" % (location['name'], location['category'], location['posx'], location['posy'], location['posz'], location['posmap'], location['id'])

# Split and Process Category Tags (2-pass)
for ( category, id ) in locationcategories.items():
	if category.find( '\\' ) < 0:
		continue

	parent_name = category[ : category.rfind('\\') ]
		
	# Create the Parent Categories if they don't exist
	while len( parent_name ) > 0:
		if not locationcategories.has_key( parent_name ):
			lastlocationcategory += 1
			locationcategories[ parent_name ] = lastlocationcategory			
				
		if parent_name.rfind('\\') < 0:
			break
				
		parent_name = parent_name[:parent_name.rfind('\\')]
		
for ( category, id ) in locationcategories.items():
	if category.rfind('\\') == -1:
		parent_name = None
	else:
		parent_name = category[:category.rfind('\\')]
		
	if not parent_name:
		parent_id = 0
	else:
		parent_id = locationcategories[ parent_name ]

	# Last Part only
	if category.find( '\\' ) >= 0:
		category = category[category.rfind('\\')+1:]

	print "INSERT INTO locationcategories VALUES(%u,'%s',%u,0);" % ( id, category, parent_id )

for ( key, element ) in items.items():
	# Initialize Item Structure
	item = {
		'id': key,
		'name': None,
		'color': 0,
		'dispid': 0,
		'category': 0,
		'categoryname': None
	}

	item = processItem( item, element )

	if not item[ 'categoryname' ]:
		continue

	if item[ 'name' ]:
		item[ 'name' ] = "'%s'" % item[ 'name' ]
	else:
		item[ 'name' ] = "'" + item[ 'categoryname' ][item[ 'categoryname' ].rfind('\\')+1:] + "'"

	print "INSERT INTO items VALUES(NULL,%s,%u,%u,%u,'%s');" % ( item[ 'name' ], item[ 'category' ], item[ 'dispid' ], item[ 'color' ], item[ 'id' ] )

# Split and Process Category Tags (2-pass)
for ( category, id ) in categories.items():
	if category.find( '\\' ) < 0:
		continue

	parent_name = category[ : category.rfind('\\') ]
		
	# Create the Parent Categories if they don't exist
	while len( parent_name ) > 0:
		if not categories.has_key( parent_name ):
			lastcategory += 1
			categories[ parent_name ] = lastcategory			
				
		if parent_name.rfind('\\') < 0:
			break
				
		parent_name = parent_name[:parent_name.rfind('\\')]
		
for ( category, id ) in categories.items():
	if category.rfind('\\') == -1:
		parent_name = None
	else:
		parent_name = category[:category.rfind('\\')]
		
	if not parent_name:
		parent_id = 0
	else:
		parent_id = categories[ parent_name ]

	# Last Part only
	if category.find( '\\' ) >= 0:
		category = category[category.rfind('\\')+1:]

	print "INSERT INTO categories VALUES(%u,'%s',%u,0);" % ( id, category, parent_id )
