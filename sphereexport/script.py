
from sections import *
import glob
from paths import SPHEREPATH
from resources import *
import random

#
# The baseclass for all sections encountered in a script
#
class Section:
	def __init__(self, deftype):
		self.deftype = deftype

	def processline(self, line):
		pass

	def finalize(self):
		pass

	def hex2dec(self, value):
		try:
			if value.startswith('0'):
				return int(value, 16)
			else:
				return int(value)
		except:
			return 0			

	def processint(self, value):
		try:
			value = value.strip()
	
			# See if it resolves to a def
			value = resolvevalue(value)
	
			# Process Math Options
			if len(value) >= 2:
				if value[0] == '{' and value[-1] == '}' and value.count(' ') == 1:
					values = value[1:-1].split(' ')
					return random.randint(self.hex2dec(values[0]), self.hex2dec(values[1]))
	
			if value.count(',') == 1:
				(minval, maxval) = value.split(',')
				return random.randint(self.hex2dec(minval), self.hex2dec(maxval))

			return self.hex2dec(value)
		except:
			return 0

#
# Unknown section
#
class UnknownSection(Section):
	def __init__(self, name):
		Section.__init__(self, None)
		self.name = name
		self.lines = 0

	def processline(self, line):
		self.lines += 1

	def finalize(self):
		#print 'Unknown section "%s" encountered with %u lines.' % (self.name, self.lines)
		pass

#
# Defname section
#
class DefnameSection(Section):
	def __init__(self, name):
		Section.__init__(self, RES_DEFNAME)
		self.name = name
		self.definitions = {}

	def processline(self, line):
		if not "\t" in line and not " " in line:
			return
		(key, value) = line.split(None, 1)
		self.definitions[key.strip().upper()] = value.strip()

#
# Typedef sections
#
class TypedefSection(Section):
	def __init__(self, name):
		Section.__init__(self, RES_TYPEDEFS)
		self.name = name
		self.definitions = {}

	def processline(self, line):
		if not "\t" in line and not " " in line:
			return
		(key, value) = line.split(None, 1)
		self.definitions[key.strip().upper()] = value.strip()

#
# Item definition
#
class ItemdefSection(Section):
	# Loading States
	PROPERTIES = 0
	EVENTS = 1
	FINISHED = 2

	def __init__(self, name):
		Section.__init__(self, RES_ITEMDEF)
		self.name = name
		self.defname = None
		self.defname2 = None
		self.state = ItemdefSection.PROPERTIES		
		self.baseitem = 0
		self.resolved = 0
		self.id = None # We dont inherit by default

		# If name is a valid hexadecimal number, 
		# the dispid is the name
		try:
			number = int(name, 16)
			if number < 0x4000:
				self.dispid = number
				self.baseitem = 1
				self.type = 0
				self.dye = 0
				self.layer = TILEINFO[number]['layer']
				self.price = 0
				self.weight = 0.0 # WARNING!! GET WEIGHT FROM TILEDATA.MUL HERE BEFORE IMPORTING!
		except:
			pass	 

	#
	# Copy important value from another itemdef
	#
	def copyfrom(self, itemdef):
		# Fully resolve the section we're inheriting from first
		if not itemdef.resolved:
			itemdef.resolve()

		if not hasattr(self, 'dye'):
			self.dye = itemdef.dye

		if not hasattr(self, 'type'):
			self.type = itemdef.type
			
		if not hasattr(self, 'layer'):
			self.layer = itemdef.layer

		if not hasattr(self, 'dispid'):
			self.dispid = itemdef.dispid

		if not hasattr(self, 'weight'):
			self.weight = itemdef.weight
			
		if not hasattr(self, 'price'):
			self.price = itemdef.price
			
	#
	# Process a line from the scripts.
	#
	def processline(self, line):
		assert(self.state != ItemdefSection.FINISHED, 'Processing lines after itemdef was finalized.')
		
		if self.state == ItemdefSection.PROPERTIES:
			if '=' in line:
				(key, value) = line.split('=', 1)
			elif ' ' in line:
				(key, value) = line.split(' ', 1)
			else:
				raise RuntimeError, "Invalid line in itemdef %s properties: %s" % (self.name, line)

			key = key.strip().upper()
			value = value.strip()

			if key == 'DYE':
				self.dye = value.upper != 'N' and value != '0'
			elif key == 'ID':
				# Complicated inheritance stuff
				# For baseitems setting ID= is stupid
				value = value.strip().upper()
				if self.baseitem:					
					raise RuntimeError, "Unexpected ID= block for baseitem %s." % self.name
				else:
					self.id = value
					self.resolved = 0

			elif key == 'DEFNAME':
				self.defname = value
			elif key == 'LAYER':
				self.layer = self.processint(value)
			elif key == 'DEFNAME2':
				self.defname2 = value
			elif key == 'REQSTR':
				self.reqstr = int(value)
			elif key == 'RESMAKE':
				self.resmake = value
			elif key == 'SKILL':
				self.skill = value
			elif key == 'SKILLMAKE':
				self.skillmake = value
			elif key == 'SPEED':
				self.speed = value
			elif key == 'TDATA1':
				self.tdata1 = value
			elif key == 'TDATA2':
				self.tdata2 = value
			elif key == 'TDATA3':
				self.tdata3 = value
			elif key == 'TDATA4':
				self.tdata4 = value
			elif key == 'TWOHANDS':
				self.twohanded = value.upper != 'N' and value != '0'
			elif key == 'TYPE':
				self.type = int(resolvetype(value))

			elif key == 'VALUE':
				value = self.processint(value)

				# Use the highest
				self.price = value
			elif key == 'WEIGHT':
				if not '.' in value:
					self.weight = int(value) / 10.0
				else:
					self.weight = float(value)

			# Enter event parsing mode
			elif key == 'ON':
				self.state = ItemdefSection.EVENTS
			
		# Event Parsing. Only allowed keyword now: ON
		else:
			pass			

	#
	# Resolve inheritance
	#
	def resolve(self):
		if not self.resolved and self.id:
			number = self.hex2dec(self.id)
			if number != 0:
				baseid = ('0%x' % number).upper()
			else:
				baseid = self.id

			if not ITEMDEFS.has_key(baseid):
				raise RuntimeError, "Unknown id=%s for item %s." % (baseid, self.name)
			else:
				self.copyfrom(ITEMDEFS[baseid])
			self.resolved = 1

	def finalize(self):
		self.state = ItemdefSection.FINISHED
			
		if not self.baseitem and not self.id:
			# Set basic properties:
			self.type = 0
			self.dispid = 0xeed # gold coin
			self.weight = 0.0
			self.layer = 0
			self.price = 0

		# Register us in the baseitem section
		ITEMDEFS[self.name.upper()] = self

		# Register us under a second name as well
		if self.defname:
			ITEMDEFS[self.defname.upper()] = self
		if self.defname2:
			ITEMDEFS[self.defname2.upper()] = self

#
# Char definition
#
class ChardefSection(Section):
	# Loading States
	PROPERTIES = 0
	EVENTS = 1
	FINISHED = 2

	def __init__(self, name):
		Section.__init__(self, RES_CHARDEF)
		self.name = name
		self.defname = None
		self.defname2 = None
		self.state = ChardefSection.PROPERTIES
		self.basechar = 0
		self.resolved = 0
		self.id = None # We dont inherit by default

		# If name is a valid hexadecimal number, 
		# the dispid is the name
		try:
			number = int(name, 16)
			if number < 0x400:
				self.dispid = number
				self.basechar = 1
				self.charname = None
		except:
			pass

	#
	# Copy important value from another chardef
	#
	def copyfrom(self, chardef):
		# Fully resolve the section we're inheriting from first
		if not chardef.resolved:
			chardef.resolve()

		if not hasattr(self, 'dispid'):
			self.dispid = chardef.dispid
		if not hasattr(self, 'charname'):
			self.charname = chardef.charname

	#
	# Process a line from the scripts.
	#
	def processline(self, line):
		assert(self.state != ChardefSection.FINISHED, 'Processing lines after chardef was finalized.')
		
		if self.state == ChardefSection.PROPERTIES:
			if '=' in line:
				(key, value) = line.split('=', 1)
			elif ' ' in line:
				(key, value) = line.split(' ', 1)
			else:
				raise RuntimeError, "Invalid line in chardef %s properties: %s" % (self.name, line)

			key = key.strip().upper()
			value = value.strip()

			if key == 'ID':
				# Complicated inheritance stuff
				# For basechars setting ID= is stupid
				value = value.strip().upper()
				if self.basechar:					
					raise RuntimeError, "Unexpected ID= block for basechar %s." % self.name
				else:
					self.id = value
					self.resolved = 0

			elif key == 'DEFNAME':
				self.defname = value
			elif key == 'DEFNAME2':
				self.defname2 = value
			elif key == 'NAME':
				self.charname = value
			
			# Enter event parsing mode
			elif key == 'ON':
				self.state = ChardefSection.EVENTS
			
		# Event Parsing. Only allowed keyword now: ON
		else:
			pass

	#
	# Resolve inheritance
	#
	def resolve(self):
		if not self.resolved and self.id:
			number = self.hex2dec(self.id)
			if number != 0:
				baseid = ('0%x' % number).upper()
			else:
				baseid = self.id

			if not CHARDEFS.has_key(baseid):
				raise RuntimeError, "Unknown id=%s for char %s." % (baseid, self.name)
			else:
				self.copyfrom(CHARDEFS[baseid])
			self.resolved = 1

	def finalize(self):
		self.state = ChardefSection.FINISHED
			
		if not self.basechar and not self.id:
			# Set basic properties:
			self.dispid = 0 # gold coin

		# Register us in the basechar section
		CHARDEFS[self.name.upper()] = self

		# Register us under a second name as well
		if self.defname:
			CHARDEFS[self.defname.upper()] = self
		if self.defname2:
			CHARDEFS[self.defname2.upper()] = self

#
# This section contains a lot of filenames.
#
class Resources(Section):
	def __init__(self, name):
		assert(len(name) == 0, 'Invalid resource section name: ' + name)
		Section.__init__(self, RES_RESOURCES)
		self.files = []

	def processline(self, line):
		# Convert path separators
		filename = (SPHEREPATH + line).replace('\\', '/')
		
		# Retrieve all .scp files in that directory
		if filename[-1] == '/':
			files = glob.glob(filename + '*.scp')
			files = map(lambda x: x.replace('\\', '/'), files)			
		else:
			files = [filename]

		self.files += files

	def finalize(self):
		pass

#
# Worlditem Section
#
class WorlditemSection(Section):
	def isitem(self):
		return 1
		
	def ischar(self):
		return 0

	def __init__(self, name):
		Section.__init__(self, RES_WORLDITEM)

		# See if the itemsection exists
		if not ITEMDEFS.has_key(name.upper()):
			raise RuntimeError, "Invalid itemid: %s." % name.upper()

		itemdef = ITEMDEFS[name.upper()]
			
		# Get basic data from the baseid
		try:
			#assert(itemdef.dispid != 0, )
			if itemdef.dispid == 0:
				raise RuntimeError, 'ITEMDEF %s has invalid display id.' % itemdef.name

			self.price = itemdef.price			
			self.dispid = itemdef.dispid
			self.layer = itemdef.layer
			self.type = itemdef.type
			self.weight = itemdef.weight
			if hasattr(itemdef, 'dye'):
			 self.dye = itemdef.dye
			else:
			 self.dye = 0
		except:
			print name
			raise
		self.baseid = name
		self.restock = 1
		self.content = [] # Contained items
		self.container = None
		self.link = 0 # Link to another item
		self.properties = {} # Untyped properties
		self.tags = {} # Itemtags
		self.name = ''
		self.serial = 0
		self.pos = [0, 0, 0, 1]
		self.saved = 0
		self.attr = 0
		self.color = 0
		self.amount = 1

	def processline(self, line):
		if '=' in line:
			(key, value) = line.split('=', 1)
		else:
			#raise RuntimeError, "Invalid line in worlditem %s: %s" % (self.baseid, line)
			return

		key = key.strip().upper()
		value = value.strip()

		if key == 'NAME':
			self.name = value
		elif key == 'LAYER':
			self.layer = self.processint(value)
		elif key == 'COLOR':
			self.color = self.hex2dec(value)
		elif key == 'TYPE':
			self.type = int(resolvetype(value))
		elif key == 'LINK':
			self.link = self.hex2dec(value)
		elif key == 'AMOUNT':
			self.amount = self.hex2dec(value)
		elif key == 'PRICE':
			self.price = self.hex2dec(value)
		elif key == 'DISPID':
			self.dispid = self.hex2dec(value)
			if self.dispid == 0:
				self.dispid = ITEMDEFS[value.upper()].dispid
		elif key == 'CONT':
			self.container = findobject(self.hex2dec(value) & 0x7FFFFFFF)
			if not self.container:
				self.container = -1
		elif key == 'ATTR':
			self.attr = self.hex2dec(value)
		elif key == 'P':
			values = map(lambda x: int(x), value.split(','))
			assert(len(values) > 0 and len(values) < 5, 'Invalid position for item %x' % self.serial)
			for i in range(0, len(values)):
				self.pos[i] = values[i]
			restock = self.pos[2] # Z Value = Restock

		elif key == 'SERIAL':
			self.serial = self.hex2dec(value) & 0x7FFFFFFF

		else:
			self.properties[key] = value
	
	#
	# Returns if the object has untyped properties
	#
	def hasproperty(self, name):
		return self.properties.has_key(name)

	#
	# Returns an untyped property
	#
	def getproperty(self, name, default=None):
		if not self.properties.has_key(name):
			return default
		else:
			return self.properties[name]

	def finalize(self):
		# Lost item
		if self.container == -1:
			return

		assert((self.pos[0] != 0) or (self.pos[1] != 0), 'Invalid position for object 0x%x' % self.serial)
		assert(self.serial != 0, 'Serial not set for object.')

		if not self.container:
			WORLDITEMS.append(self)
		else:
			self.container.content.append(self)
		registerobject(self)
		
#
# Worldchar Section
#
class WorldcharSection(Section):
	def ischar(self):
		return 1
		
	def isitem(self):
		return 0

	def __init__(self, name):
		Section.__init__(self, RES_WORLDCHAR)

		# See if the charsection exists
		if not CHARDEFS.has_key(name.upper()):
			raise RuntimeError, "Invalid charid: %s." % name.upper()

		chardef = CHARDEFS[name.upper()]
		self.basechar = chardef
			
		# Get basic data from the baseid
		try:
			if chardef.dispid == 0:
				raise RuntimeError, 'CHARDEF %s has invalid display id.' % chardef.name

			self.dispid = chardef.dispid
		except:
			print name
			raise
		self.baseid = name
		self.content = [] # Contained items
		self.properties = {} # Untyped properties
		self.tags = {} # Chartags
		self.events = []
		self.name = ''
		self.owner = -1
		self.serial = 0
		self.pos = [0, 0, 0, 1]
		self.attr = 0
		self.color = 0
		self.npc = 0
		self.action = 0
		self.hitpoints = 0
		self.stamina = 0
		self.mana = 0
		self.strength = 0
		self.dexterity = 0
		self.saved = 0
		self.stablemaster = -1
		self.intelligence = 0

	def processline(self, line):
		if '=' in line:
			(key, value) = line.split('=', 1)
		else:
			#raise RuntimeError, "Invalid line in worldchar %s: %s" % (self.baseid, line)
			return

		key = key.strip().upper()
		value = value.strip()

		if key == 'NAME':
			self.name = value
		elif key == 'BRAIN' or key == 'NPC':
			self.npc = self.hex2dec(value)
		elif key == 'COLOR':
			self.color = self.hex2dec(value)
		elif key == 'FLAGS':
			self.attr = self.hex2dec(value)
		elif key == 'OSTR':
			self.strength = self.hex2dec(value)
		elif key == 'ODEX':
			self.dexterity = self.hex2dec(value)
		elif key == 'OINT':
			self.intelligence = self.hex2dec(value)
		elif key == 'HITS':
			self.hitpoints = self.hex2dec(value)
		elif key == 'STAM':
			self.stamina = self.hex2dec(value)
		elif key == 'MANA':
			self.mana = self.hex2dec(value)
		elif key == 'ACTION':
			self.action = self.hex2dec(value)
		elif key == 'EVENTS':
			self.events.append(value.upper())
		elif key == 'P':
			values = map(lambda x: int(x), value.split(','))
			assert(len(values) > 0 and len(values) < 5, 'Invalid position for char %x' % self.serial)
			for i in range(0, len(values)):
				self.pos[i] = values[i]
		elif key == 'SERIAL':
			self.serial = self.hex2dec(value) & 0x7FFFFFFF
		elif key.startswith('TAG.'):
			tagname = key[4:]
			self.tags[tagname] = value

		else:
			self.properties[key] = value
	
	#
	# Returns if the object has untyped properties
	#
	def hasproperty(self, name):
		return self.properties.has_key(name)

	#
	# Returns an untyped property
	#
	def getproperty(self, name, default=None):
		if not self.properties.has_key(name):
			return default
		else:
			return self.properties[name]

	def finalize(self):
		assert((self.pos[0] != 0) or (self.pos[1] != 0), 'Invalid position for object 0x%x' % self.serial)
		assert(self.serial != 0, 'Serial not set for object.')

		WORLDCHARS.append(self)
		registerobject(self)


DEFTYPES = {
	'RESOURCES': Resources,
	'ITEMDEF': ItemdefSection,
	'CHARDEF': ChardefSection,
	'DEFNAME': DefnameSection,
	'TYPEDEFS': TypedefSection,
	'WORLDITEM': WorlditemSection,
	'WORLDCHAR': WorldcharSection,
}

#
# This class encapsulates a sphere script file.
#
class Script:
	def __init__(self):
		# Sections encountered in the script
		self.sections = {}

		# Properties outside of sections
		self.properties = {}

		for i in range(RES_ACCOUNT, RES_QTY):
			self.sections[i] = []		

	#
	# Helper function to see if a line qualifies as a 
	# section header
	#
	def isheader(self, line):
		return line[0] == '[' and line[-1] == ']'

	#
	# Sees if the line qualifies as an assignment
	#
	def isassignment(self, line):
		return '=' in line

	#
	# Gets a line from the line list
	#
	def getline(self):
		if len(self.lines) == 0:
			return None

		line = self.lines.pop().strip()

		# Ignore comments and empty lines
		if line.startswith('//') or len(line) == 0:
			return self.getline()

		# Process comments at end of line
		if '//' in line:
			line = line[:line.index('//')].strip()

		return line

	#
	# Load this script from a file.
	#
	def load(self, filename):
		self.filename = filename

		file = open(filename, 'rU') 
		self.lines = file.readlines()
		self.lines.reverse() # Reverse so the first line is actually the last
		file.close()

		# Process all lines
		while 1:
			line = self.getline()

			if not line:
				break

			if not self.isheader(line):
				if not self.isassignment(line):
					#raise RuntimeError, "Invalid statement (%s): %s" % (filename, line)
					pass
				else:
					(key, value) = line.split('=', 1)
					self.properties[key] = value
			else:
				deftype = line[1:-1].upper()

				if deftype == 'EOF':
					break

				# See if the deftype has an id
				if ' ' in deftype:
					(deftype, name) = deftype.split(' ', 1)
				else:
					name = ''

				if not DEFTYPES.has_key(deftype):
					if len(name) > 0:
						section = UnknownSection(deftype + ' ' + name)
					else:
						section = UnknownSection(deftype)
				else:
					section = DEFTYPES[deftype](name)

				while 1:
					line = self.getline()
					if not line:
						break

					if self.isheader(line):
						self.lines.append(line)
						break
					else:
						section.processline(line)
				
				# Only append known deftypes
				if self.sections.has_key(section.deftype):
					self.sections[section.deftype].append(section)

				section.finalize()

	#
	# Copy all loaded resources to the global resource array
	#
	def finalize(self):
		for i in range(RES_ACCOUNT, RES_QTY):
			RESOURCES[i] += self.sections[i]
