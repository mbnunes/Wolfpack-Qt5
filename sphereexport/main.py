
from script import *
from config import *
from sections import *
from resources import *
from conversion import *

def loadscript(filename):
	print "Loading %s..." % filename
	spheretables = Script()
	spheretables.load(filename)
	spheretables.finalize()

#
# Load tiledata
#
print "Loading tiledata..."
loadtiledata(TILEDATA)

#
# Load the scripts
#
loadscript(SPHERETABLES)

for resource in RESOURCES[RES_RESOURCES]:
	for filename in resource.files:
		loadscript(filename)

print "Resolving inheritance..."

#
# Postprocess item scripts
#
for resource in RESOURCES[RES_ITEMDEF]:
	resource.resolve()
	
for resource in RESOURCES[RES_CHARDEF]:
	resource.resolve()

#
# Now load the worldsave
#
loadscript(SAVEDIRECTORY + "spherechars.scp")
loadscript(SAVEDIRECTORY + "sphereworld.scp")

print "Exporting SQL file..."

output = file('output.sql', 'wt')

output.write("PRAGMA CACHE_SIZE = 20000;\n")
output.write("BEGIN;\n")

#
# Export all world items
#
for item in WORLDITEMS:
	convertitem(output, item)

#
# Export all world chars
#
for char in WORLDCHARS:
	convertchar(output, char)

output.write("END;\n")

output.close()

print "Missing NPC translations in config.py:"

for npc in MISSINGNPCS:
	print "\t'%s': ''," % npc
