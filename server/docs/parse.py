
import re

COMMAND_NAME_PATTERN = re.compile("\\\\command\s(\w+)", re.S)
COMMAND_DESCRIPTION_PATTERN = re.compile('\\\\description\s+(.*?)(?:\Z|[\s\n]+\\\\\w)', re.S)
COMMAND_USAGE_PATTERN = re.compile('\\\\usage\s+(.*?)(?:\Z|[\s\n]+\\\\\w)', re.S)
COMMAND_NOTES_PATTERN = re.compile('\\\\notes\s+(.*?)(?:\Z|[\s\n]+\\\\\w)', re.S)


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
	result = COMMAND_NOTES_PATTERN.search(text)
	if result:
		notes = result.group(1)
	
	return {
		'name': name,
		'description': processtext(description),
		'usage': processtext(usage),
		'notes': processtext(notes),
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

	return commands

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

	results = pattern.finditer(content)
	for result in results:
		text = result.group(1)
		if text.startswith('\\command'):
			command = parsecommand(text)
			if command:
				commands.append(command)

	return commands
