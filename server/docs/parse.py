
import re

COMMAND_NAME_PATTERN = re.compile("\\\\command\s(\w+)", re.S)
COMMAND_DESCRIPTION_PATTERN = re.compile('\\\\description\s+(.*?)(?=\Z|[\s\n]+\\\\\w)', re.S)
COMMAND_USAGE_PATTERN = re.compile('\\\\usage\s+(.*?)(?=\Z|[\s\n]+\\\\\w)', re.S)
NOTES_PATTERN = re.compile('\\\\notes\s+(.*?)(?=\Z|[\s\n]+\\\\\w)', re.S)
RETURNVALUE_PATTERN = re.compile('\\\\return\s+(.*?)(?=\Z|[\s\n]+\\\\\w)', re.S)
CALLCONDITION_PATTERN = re.compile('\\\\condition\s+(.*?)(?=\Z|[\s\n]+\\\\\w)', re.S)
EVENT_NAME_PATTERN = re.compile("\\\\event\s(\w+)", re.S)
PARAM_PATTERN = re.compile('\\\\param\s+(\w+)\s+(.*?)(?=\Z|[\s\n]+\\\\\w)', re.S)

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
	result = COMMAND_DESCRIPTION_PATTERN.search(text)
	if result:
		description = result.group(1)
		
	# Search for the usage
	result = COMMAND_USAGE_PATTERN.search(text)
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
	events = []

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

	return (commands, events)

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

	return (commands, events)
