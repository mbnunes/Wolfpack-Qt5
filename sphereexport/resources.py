
from sections import *
from struct import unpack

#
# Global Resources
#
RESOURCES = {}

for i in range(RES_ACCOUNT, RES_QTY):
	RESOURCES[i] = []

ITEMDEFS = {}

CHARDEFS = {}

MISSINGNPCS = []

WORLDCHARS = []

WORLDITEMS = []

REGISTRY = {}

TILEINFO = {}

def findobject(serial):
	global REGISTRY
	if not REGISTRY.has_key(serial):
		return None
	else:
		return REGISTRY[serial]

def registerobject(object):
	global REGISTRY
	REGISTRY[object.serial] = object

#
# Searches for a definition in the resources
# and replaces the value if needed.
#
def resolvevalue(value):
	for resource in RESOURCES[RES_DEFNAME]:
		if resource.definitions.has_key(value.upper()):
			return resource.definitions[value.upper()]

	return value

#
# Resolve a type value
#
def resolvetype(value):
	for resource in RESOURCES[RES_TYPEDEFS]:
		if resource.definitions.has_key(value.upper()):
			return int(resource.definitions[value.upper()])

	try:
		return int(value)
	except:
		return 0

#
# Load tiledata information.
#
def loadtiledata(filename):
	file = open(filename, 'rb')
	
	# Skip the landscape part
	file.seek(512 * 836)
		
	# Read 0x4000 items
	for block in range(0, 512):
		file.read(4)
		for item in range(0, 32):
			itemid = block * 32 + item
			(flags, weight, layer, unknown1, unknown2, quantity, anim, unknown3, hue, unknown4, unknown5, height, name) = unpack('<I2BH2BH5B20s', file.read(37))
			info = {
				'flags': flags,
				'weight': weight,
				'layer': layer,
				'unknown1': unknown1,
				'unknown2': unknown2,
				'quantity': quantity,
				'anim': anim,
				'unknown3': unknown3,
				'hue': hue,
				'unknown4': unknown4,
				'unknown5': unknown5,
				'height': height,
				'name': name
			}
			TILEINFO[itemid] = info
	
	file.close()
