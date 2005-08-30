
PATH = 'C:\\Games\\Ultima Online\\'

from struct import unpack

fp = open(PATH + 'blueprints.tbp', 'rb')

(bpid, count1) = unpack('<bb', fp.read(2))
print "File Version: %u" % bpid
bytes = []
for i in range(0, count1):
	bytes.append('0x%x' % unpack('<b', fp.read(1)))
print "Unknown Bytes: [%s]" % ", ".join(bytes)
(textures1, textures2) = unpack('<hh', fp.read(4))
print "Number of Groups: %u" % textures1
print "Number of Individual Textures: %u" % textures2

GROUPS = {}

# Read the textures
for i in range(0, textures2):
	(unknown1, width, height, tilecount) = unpack('<hhhb', fp.read(7))
	print "\tTexture Dimensions: %ux%u" % (width, height)
	print "\tGroup ID: %u" % unknown1
	print "\tTile Count: %u" % tilecount
	GROUPS[unknown1] = 1
	
	for j in range(0, tilecount):
		(tileid, xoffset, yoffset) = unpack('<hhh', fp.read(6))
		print "\t\tTile: 0x%x, X/Y: %u,%u" % (tileid, xoffset, yoffset)

fp.close()

print "Number of Groups: %u" % len(GROUPS.keys())