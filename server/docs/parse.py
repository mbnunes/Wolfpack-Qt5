
import re

COMMAND_NAME_PATTERN = re.compile("\\\\command\s(\w+)", re.S)
PROPERTY_NAME_PATTERN = re.compile("\\\\rproperty\s([\w\.]+)", re.S)
RPROPERTY_NAME_PATTERN = re.compile("\\\\property\s([\w\.]+)", re.S)
OBJECT_NAME_PATTERN = re.compile("\\\\object\s(\w+)", re.S)
METHOD_NAME_PATTERN = re.compile("\\\\method\s([\w\.]+)", re.S)
DESCRIPTION_PATTERN = re.compile('\\\\description\s+(.*?)(?=\Z|[\s\n]+\\\\\w)', re.S)
USAGE_PATTERN = re.compile('\\\\usage\s+(.*?)(?=\Z|[\s\n]+\\\\\w)', re.S)
NOTES_PATTERN = re.compile('\\\\notes\s+(.*?)(?=\Z|[\s\n]+\\\\\w)', re.S)
RETURNVALUE_PATTERN = re.compile('\\\\return\s+(.*?)(?=\Z|[\s\n]+\\\\\w)', re.S)
CALLCONDITION_PATTERN = re.compile('\\\\condition\s+(.*?)(?=\Z|[\s\n]+\\\\\w)', re.S)
EVENT_NAME_PATTERN = re.compile("\\\\event\s(\w+)", re.S)
PARAM_PATTERN = re.compile('\\\\param\s+(\w+)\s+(.*?)(?=\Z|[\s\n]+\\\\\w)', re.S)
INHERIT_PATTERN = re.compile('\\\\inherit\s+(\w+)\s*(?=\Z|[\s\n]+\\\\\w)', re.S)

VERSION = "Unknown"
BETA = ""

def getVersion():
	if len(BETA) == 0:
		return 'Wolfpack ' + VERSION
	else:
		return 'Wolfpack ' + VERSION + ' ' + BETA

# <object id="char">....</object>
# <object id="item">....</object>

def processtext(text):
	return text.strip().replace("\\", "\\\\").replace("\n", "<br>\n")

#
# Parse a command comment
#
def parsecommand(text):
	name = ''
	description = ''
	notes = ''
	usage = ''

	# Get the command name we're documenting
	result = COMMAND_NAME_PATTERN.search(text)
	if not result:
		print "Command comment with invalid command name..."
		return None
	else:
		name = result.group(1).upper()

	# Search for the description
	result = DESCRIPTION_PATTERN.search(text)
	if result:
		description = result.group(1)
		
	# Search for the usage
	result = USAGE_PATTERN.search(text)
	if result:
		usage = result.group(1)
		
	# Search for the notes
	result = NOTES_PATTERN.search(text)
	if result:
		notes = result.group(1)
	
	return {
		'name': processtext(name),
		'description': processtext(description),
		'usage': processtext(usage),
		'notes': processtext(notes),
	}

#
# Parse an object comment
#
def parseobject(text):
	name = ''
	description = ''

	# Get the object name we're documenting
	result = OBJECT_NAME_PATTERN.search(text)
	if not result:
		return None
	else:
		name = result.group(1).upper()

	# Search for the description
	result = DESCRIPTION_PATTERN.search(text)
	if result:
		description = result.group(1)
	
	inherit = []
	
	results = INHERIT_PATTERN.findall(text)
	for result in results:
		inherit.append(result.strip().upper())
	
	return {
		'object': processtext(name),
		'description': processtext(description),
		'inherit': inherit,
	}	
	
#
# Parse an object method comment
#
def parsemethod(text):
	object = ''
	method = ''
	description = ''
	prototype = ''
	parameters = ''
	returnvalue = ''

	# Get the object name we're documenting
	result = METHOD_NAME_PATTERN.search(text)
	if not result:
		return None
	else:
		name = result.group(1).split('.', 1)
		if len(name) != 2:
			return None
		else:
			object = name[0].upper()
			method = name[1]

	# Search for the description
	result = DESCRIPTION_PATTERN.search(text)
	if result:
		description = result.group(1)
	
	# Search for the returnvalue
	result = RETURNVALUE_PATTERN.search(text)
	if result:
		returnvalue = result.group(1)	

	parameters = [] # Generate a list of parameters
	paramnames = []
	
	results = PARAM_PATTERN.findall(text)
	for result in results:
		param = result[0].strip()
		desc = result[1].strip()
		parameters.append("- <i>%s</i>\n%s" % (param, desc))
		paramnames.append(param)

	# Generate the prototype
	prototype = "%s(%s)" % (method, ', '.join(paramnames))
	
	return {
		'object': processtext(object),
		'method': processtext(method),
		'description': processtext(description),
		'returnvalue': processtext(returnvalue),
		'parameters': processtext("\n\n".join(parameters)),
		'prototype': processtext(prototype),
	}		

#
# Parse a event comment
#
def parseevent(text):
	name = ''
	notes = ''
	returnvalue = ''
	callcondition = ''

	# Get the command name we're documenting
	result = EVENT_NAME_PATTERN.search(text)
	if not result:
		print "Event comment with invalid command name..."
		return None
	else:
		name = result.group(1)

	# Search for the notes
	result = NOTES_PATTERN.search(text)
	if result:
		notes = result.group(1)
		
	# Search for the returnvalue
	result = RETURNVALUE_PATTERN.search(text)
	if result:
		returnvalue = result.group(1)
		
	# Search for the notes
	result = CALLCONDITION_PATTERN.search(text)
	if result:
		callcondition = result.group(1)				

	parameters = [] # Generate a list of parameters
	paramnames = []
	
	results = PARAM_PATTERN.findall(text)
	for result in results:
		param = result[0].strip()
		description = result[1].strip()
		parameters.append("- <i>%s</i>\n%s" % (param, description))
		paramnames.append(param)

	# Generate the prototype
	prototype = "def %s(%s):\n&nbsp;&nbsp;pass" % (name, ', '.join(paramnames))
	
	return {
		'name': processtext(name),
		'notes': processtext(notes),
		'callcondition': processtext(callcondition),
		'returnvalue': processtext(returnvalue),
		'parameters': processtext("\n\n".join(parameters)),
		'prototype': processtext(prototype),
	}

#
# The following function parses a python file 
# and searches for comments.
#
def parsepython(filename):
	pattern = re.compile("\"\"\"\s*\n*\s*(.*?)\s*\n*\s*\"\"\"", re.S) # Multiline comment pattern

	file = open(filename, 'rU')
	content = file.read()
	file.close()

	commands = []
	results = pattern.finditer(content)
	for result in results:
		text = result.group(1)
		if text.startswith('\\command'):
			command = parsecommand(text)
			if command:
				commands.append(command)
	return (commands, [], [], [], [])

#
# The following function parses a C++ file 
# and searches for comments.
#
def parsecpp(filename):
	pattern = re.compile("\/\*\s*\n*\s*(.*?)\s*\n*\s*\*\/", re.S) # Multiline comment pattern	

	file = open(filename, 'rU')
	content = file.read()
	file.close()

	commands = []
	events = []
	objects = []
	objectsmethods = []
	objectsproperties = []

	results = pattern.finditer(content)
	for result in results:
		text = result.group(1)
		if text.startswith('\\command'):
			command = parsecommand(text)
			if command:
				commands.append(command)
		elif text.startswith('\\event'):
			event = parseevent(text)
			if event:
				events.append(event)
		elif text.startswith('\\object'):
			object = parseobject(text)
			if object:
				objects.append(object)
		elif text.startswith('\\method'):
			method = parsemethod(text)
			if method:
				objectsmethods.append(method)
		elif text.startswith('\\property '):
			text = text[10:]
			if ' ' in text:
				(name, description) = text.split(' ', 1)
				if not '.' in name:
					continue					
				(object, property) = name.split('.', 1)
				objectsproperties.append({
					'object': processtext(object.upper()),
					'property': processtext(property),
					'description': processtext(description),
					'readonly': '0'
				})
		elif text.startswith('\\rproperty '):
			text = text[11:]
			if ' ' in text:
				(name, description) = text.split(' ', 1)
				if not '.' in name:
					continue					
				(object, property) = name.split('.', 1)
				objectsproperties.append({
					'object': processtext(object.upper()),
					'property': processtext(property),
					'description': processtext(description),
					'readonly': '1'
				})				

	# Get singleline comments
	pattern = re.compile("\/\/\ *(.*?)$", re.M) # Single line comment pattern
	
	results = pattern.finditer(content)
	for result in results:
		text = result.group(1).strip()
		if text.startswith('\\property '):
			text = text[10:]
			if ' ' in text:
				(name, description) = text.split(' ', 1)
				if not '.' in name:
					continue					
				(object, property) = name.split('.', 1)
				objectsproperties.append({
					'object': processtext(object.upper()),
					'property': processtext(property),
					'description': processtext(description),
					'readonly': '0'
				})
		elif text.startswith('\\rproperty '):
			text = text[11:]
			if ' ' in text:
				(name, description) = text.split(' ', 1)
				if not '.' in name:
					continue					
				(object, property) = name.split('.', 1)
				objectsproperties.append({
					'object': processtext(object.upper()),
					'property': processtext(property),
					'description': processtext(description),
					'readonly': '1'
				})

	# See if there is a version string in this file
	result = re.compile("inline\s+const\s+char\s*\*\s*productVersion\(\)\s*\{\s*return\s+\"([^\"]+)\"\;", re.S).search(content)
	
	if result:
		global VERSION
		VERSION = result.group(1)
		
	# See if there is a version string in this file
	result = re.compile("inline\s+const\s+char\s*\*\s*productBeta\(\)\s*\{\s*return\s+\"([^\"]+)\"\;", re.S).search(content)
	
	if result:
		global BETA
		BETA = result.group(1)		

	return (commands, events, objects, objectsmethods, objectsproperties)
