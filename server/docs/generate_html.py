
import re
import math
import urllib
import time
import os
import os.path
import sys
from parse import parsepython, parsecpp, getVersion, getVersionNumber
from glob import glob

paths = sys.argv[1:]
commands = []
events = []
objects = []
objectsmethods = []
objectsproperties = []
functions = []
constants = []

if len(paths) == 0:
	print "Usage: python generate.py path1[,path2,...]"
	sys.exit()

def examine(path):
	global commands
	global events
	global objects
	global objectsmethods
	global objectsproperties
	global functions
	global constants
	
	#print "Examining %s..." % path
	files = glob(path + '/*.py')
	
	for file in files:
		if os.path.isfile(file):
			(newcommands, newevents, newobjects, newobjectsmethods, newobjectsproperties, newfunctions, newconstants) = parsepython(file)
			commands += newcommands
			events += newevents
			functions += newfunctions
			constants += newconstants
			
	files = glob(path + '/*.cpp')
	
	for file in files:
		if os.path.isfile(file):
			(newcommands, newevents, newobjects, newobjectsmethods, newobjectsproperties, newfunctions, newconstants) = parsecpp(file)
			commands += newcommands
			events += newevents
			objects += newobjects
			objectsmethods += newobjectsmethods
			objectsproperties += newobjectsproperties
			functions += newfunctions
			
	files = glob(path + '/*.h')
	
	for file in files:
		if os.path.isfile(file):
			(newcommands, newevents, newobjects, newobjectsmethods, newobjectsproperties, newfunctions, newconstants) = parsecpp(file)
			commands += newcommands
			events += newevents
			objects += newobjects
			objectsmethods += newobjectsmethods
			objectsproperties += newobjectsproperties			
			functions += newfunctions
	
	# Get subdirectories and process them
	entries = glob(path + '/*')
	for entry in entries:
		if os.path.isdir(entry):
			examine(entry)
					
for path in paths:
	examine(path)
	
def quote(text):
	return text.replace("'", "\\'")

# Time
version = getVersion()
generated = time.strftime("%d.%m.%Y %H:%M")

def namesort(a, b):
	return cmp(a['name'], b['name'])

# Compile a command overview
overview = ''
cols = 7
rows = int(math.ceil(len(commands) / 7.0))
commands.sort(namesort)

for row in range(0, rows):
	overview += "<tr>\n"

	for col in range(0, cols):
		id = col * rows + row
		if id < len(commands):
			command = commands[id]
			overview += '<td>- <a href="command_%s.html">%s</a></td>' % (command['name'].lower(), command['name'])
		else:
			overview += "<td>&nbsp;</td>\n";

	overview += "</tr>\n" 	

# Write an index file for the events.
template = open('templates/commands.html')
text = template.read()
template.close()

text = text.replace('{GENERATED}', generated)
text = text.replace('{VERSION}', version)
text = text.replace('{OVERVIEW}', overview)

output = open('webroot/commands.html', "wt")
output.write(text)
output.close()
	
for command in commands:
	template = open('templates/command.html')
	text = template.read()
	template.close()

	# Replace Tokens
	text = text.replace('{NAME}', command['name'])
	text = text.replace('{SHORTDESC}', command['description'])
	text = text.replace('{OVERVIEW}', overview)
	text = text.replace('{GENERATED}', generated)
	text = text.replace('{VERSION}', version)

	if len(command['usage']) > 0:
		usage = "<strong>Usage: </strong><br>%s<br><br>" % command['usage']
	else:
		usage = ""

	text = text.replace('{USAGE}', usage)

        if len(command['notes']) > 0:
                notes = "<strong>Notes: </strong><br>%s<br><br>" % command['notes']
        else:
                notes = ""

        text = text.replace('{NOTES}', notes)

	output = open('webroot/command_%s.html' % command['name'].lower(), "wt")
	output.write(text)
	output.close()

# Compile an event overview
overview = ''
cols = 7
rows = int(math.ceil(len(events) / 7.0))
events.sort(namesort)

for row in range(0, rows):
	overview += "<tr>\n"

	for col in range(0, cols):
		id = col * rows + row
		if id < len(events):
			event = events[id]
			overview += '<td><a href="event_%s.html">%s</a></td>' % (event['name'].lower(), event['name'])
		else:
			overview += "<td>&nbsp;</td>\n";

	overview += "</tr>\n" 	

# Write an index file for the events.
template = open('templates/events.html')
text = template.read()
template.close()

text = text.replace('{OTHEREVENTS}', overview)
text = text.replace('{GENERATED}', generated)
text = text.replace('{VERSION}', version)

output = open('webroot/events.html', "wt")
output.write(text)
output.close()
	
for event in events:
	template = open('templates/event.html')
	text = template.read()
	template.close()

	# Replace Tokens
	text = text.replace('{NAME}', event['name'])
	text = text.replace('{PROTOTYPE}', event['prototype'])
	text = text.replace('{PARAMETERS}', "<strong>Parameters:</strong><br>%s<br><br>" % event['parameters'])
	text = text.replace('{RETURNVALUE}', "<strong>Return Value:</strong><br>%s<br><br>" % event['returnvalue'])
	text = text.replace('{CALLCONDITION}', "<strong>Call Condition:</strong><br>%s<br><br>" % event['callcondition'])
	text = text.replace('{NOTES}', "<strong>Notes:</strong><br>%s<br><br>" % event['notes'])
	text = text.replace('{OTHEREVENTS}', overview)
	text = text.replace('{GENERATED}', generated)
	text = text.replace('{VERSION}', version)
	
	output = open('webroot/event_%s.html' % event['name'].lower(), "wt")
	output.write(text)
	output.close()

def objectcmp(a, b):
	return cmp(a['object'], b['object'])
	
def methodcmp(a, b):
	return cmp(a['method'], b['method'])	
	
def propertycmp(a, b):
	return cmp(a['property'], b['property'])	
	
objects.sort(objectcmp)

# Compile an event overview
overview = ''
cols = 7
rows = int(math.ceil(len(objects) / 7.0))

for row in range(0, rows):
	overview += "<tr>\n"

	for col in range(0, cols):
		id = col * rows + row
		if id < len(objects):
			object = objects[id]
			overview += '<td><a href="object_%s.html">%s</a></td>' % (object['object'].lower(), object['object'])
		else:
			overview += "<td>&nbsp;</td>\n";

	overview += "</tr>\n" 	

# Write an index file for the objects.
template = open('templates/objects.html')
text = template.read()
template.close()

text = text.replace('{OVERVIEW}', overview)
text = text.replace('{GENERATED}', generated)
text = text.replace('{VERSION}', version)

output = open('webroot/objects.html', "wt")
output.write(text)
output.close()
	
for object in objects:
	# Compile a list of objects we inherit from (complete, recursive)
	inherited = object['inherit'][:] # Create a copy
	
	for i in range(0, len(objects)):
		o = objects[i]
		if o['object'] in inherited:
			inherited += o['inherit']
			i = 0 # Begin anew

	# Compile a complete list of properties and methods
	# in this object and all parent classes
	methods = []
	for method in objectsmethods:
		if method['object'] in inherited or method['object'] == object['object']:
			methods.append(method)

	properties = []
	for property in objectsproperties:
		if property['object'] in inherited or property['object'] == object['object']:
			properties.append(property)
	
	properties.sort(propertycmp)
	methods.sort(methodcmp)

	# Generate the Template
	template = open('templates/object.html')
	text = template.read()
	template.close()
	
	text = text.replace('{GENERATED}', generated)
	text = text.replace('{VERSION}', version)	
		
	# Compile an overview 	
	overview = ''
	cols = 7
	rows = int(math.ceil(len(methods) / 7.0))
	
	for row in range(0, rows):
		overview += "<tr>\n"
	
		for col in range(0, cols):
			id = col * rows + row
			if id < len(methods):
				method = methods[id]
				overview += '<td width="15%%">-&#160;<a href="#meth_%s">%s</a></td>' % (method['method'].lower(), method['method'])
			else:
				overview += "<td width=\"15%\">&nbsp;</td>\n";
	
		overview += "</tr>\n"
		
	text = text.replace('{METHODOVERVIEW}', overview)
	
	# Compile an overview 	
	overview = ''
	cols = 7
	rows = int(math.ceil(len(properties) / 7.0))
	
	for row in range(0, rows):
		overview += "<tr>\n"
	
		for col in range(0, cols):
			id = col * rows + row
			if id < len(properties):
				property = properties[id]
				overview += '<td width="15%%">-&#160;<a href="#prop_%s">%s</a></td>' % (property['property'].lower(), property['property'])
			else:
				overview += "<td width=\"15%\">&nbsp;</td>\n";
	
		overview += "</tr>\n"
		
	text = text.replace('{PROPERTYOVERVIEW}', overview)
	
	# Generate a list of methods
	overview = ''
	for i in range(0, len(methods)):
		method = methods[i]
		
		parameters = ''
		if len(method['parameters']) > 0:
			parameters = "%s<br/><br/>\n" % method['parameters']
			
		returnvalue = ''
		if len(method['returnvalue']) > 0:
			returnvalue = "<span class=\"style2\">Return Value:</span><br/>%s<br/><br/>\n" % method['returnvalue']
		
		description = ''
		if len(method['description']) > 0:
			description = "<span class=\"style2\">Description:</span><br />%s<br />\n" % method['description']
		
		overview += "<a name=\"meth_%(lowername)s\"></a> \
					<b><code style=\"font-size: 12px\">%(prototype)s</code></b><br />\
					<br/>%(parameters)s\n\
					%(returnvalue)s\n\
					%(description)s\n\
						<br /><a href=\"#top\">Back to top</a>\n" % {
							'lowername': method['method'].lower(),
							'name': method['method'],
							'prototype': method['prototype'],
							'parameters': parameters,
							'returnvalue': returnvalue,
							'description': description
						}
						
		if i != len(methods) - 1:
			overview += '<hr size="1">'
						
	text = text.replace('{OBJECTMETHODS}', overview)

	# Generate a list of properties
	overview = ''
	for i in range(0, len(properties)):
		property = properties[i]

		description = ''
		if len(property['description']) > 0:
			description = "<span class=\"style2\">Description:</span><br />%s<br />\n" % property['description']

		readonly = ''
		if property['readonly'] == '1':
			readonly = ' (read only)'
		
		overview += """<a name="prop_%(lowername)s"></a>\
		<b><code style="font-size: 12px">%(name)s</code></b>%(readonly)s<br /><br />\
		%(description)s
		<br /><a href="#top">Back to top</a>\n""" % {
			'lowername': property['property'].lower(),
			'name': property['property'],
			'description': description,
			'readonly': readonly
		}

		if i != len(properties) - 1:
			overview += '<hr size="1">'
						
	text = text.replace('{OBJECTPROPERTIES}', overview)

	# Replace Tokens
	text = text.replace('{NAME}', object['object'])
	if len(object['description']) != 0:
		text = text.replace('{DESCRIPTION}', '<strong>Description:</strong><br>%s<br><br>' % object['description'])
	else:
		text = text.replace('{DESCRIPTION}', '')
	
	output = open('webroot/object_%s.html' % object['object'].lower(), "wt")
	output.write(text)
	output.close()

# Generate a module list
modules = []

for function in functions:
	module = function['module']
	current = ''
	fmodules = module.split('.')
	for module in fmodules:
		if current != '':
			current += '.' + module
		else:
			current = module		
			
		if current not in modules:
			modules.append(current)
			
for constant in constants:
	module = constant['module']
	current = ''
	fmodules = module.split('.')
	for module in fmodules:
		if current != '':
			current += '.' + module
		else:
			current = module		
			
		if current not in modules:
			modules.append(current)			

# Create an overview
# Compile a command overview
overview = ''
cols = 7
rows = int(math.ceil(len(modules) / 7.0))
modules.sort()

for row in range(0, rows):
	overview += "<tr>\n"

	for col in range(0, cols):
		id = col * rows + row
		if id < len(modules):
			module = modules[id]
			overview += '<td>- <a href="module_%s.html">%s</a></td>' % (module.replace('.', '_').lower(), module)
		else:
			overview += "<td>&nbsp;</td>\n";

	overview += "</tr>\n" 	

# Write an index file for the objects.
template = open('templates/modules.html')
text = template.read()
template.close()

text = text.replace('{OVERVIEW}', overview)
text = text.replace('{GENERATED}', generated)
text = text.replace('{VERSION}', version)

output = open('webroot/modules.html', "wt")
output.write(text)
output.close()

# Find every function for the module and write
# a module index file.
for module in modules:
	# Create a list of functions for this module
	localfunctions = []
	for function in functions:
		if function['module'] == module:
			localfunctions.append(function)
			
	localconstants = []
	for constant in constants:
		if constant['module'] == module:
			localconstants.append(constant)
			
	template = open('templates/module.html')
	text = template.read()
	template.close()
	
	text = text.replace('{GENERATED}', generated)
	text = text.replace('{VERSION}', version)

	# Create a function overview first
	overview = ''
	cols = 7
	rows = int(math.ceil(len(localfunctions) / 7.0))
	
	localfunctions.sort(namesort)
	
	for row in range(0, rows):
		overview += "<tr>\n"
	
		for col in range(0, cols):
			id = col * rows + row
			if id < len(localfunctions):
				function = localfunctions[id]
				overview += '<td width="15%%">-&#160;<a href="#func_%s">%s</a></td>' % (function['name'].lower(), function['name'])
			else:
				overview += "<td width=\"15%\">&nbsp;</td>\n";
	
		overview += "</tr>\n"
		
	text = text.replace('{FUNCTIONOVERVIEW}', overview)
		
	# Generate a list of methods
	overview = ''
	if len(localfunctions) > 0:
		overview += """<p><span class="sectiontitle">MODULE FUNCTIONS</span><br>
			<br>"""
	
	for i in range(0, len(localfunctions)):
		function = localfunctions[i]
		
		parameters = ''
		if len(function['parameters']) > 0:
			parameters = "%s<br/><br/>\n" % function['parameters']
			
		returnvalue = ''
		if len(function['returnvalue']) > 0:
			returnvalue = "<span class=\"style2\">Return Value:</span><br/>%s<br/><br/>\n" % function['returnvalue']
		
		description = ''
		if len(function['description']) > 0:
			description = "<span class=\"style2\">Description:</span><br />%s<br />\n" % function['description']
		
		overview += "<a name=\"func_%(lowername)s\"></a> \
					<b><code style=\"font-size: 12px\">%(prototype)s</code></b><br />\
					<br/>%(parameters)s\n\
					%(returnvalue)s\n\
					%(description)s\n\
						<br /><a href=\"#top\">Back to top</a>\n" % {
							'lowername': function['name'].lower(),
							'name': function['name'],
							'prototype': function['prototype'],
							'parameters': parameters,
							'returnvalue': returnvalue,
							'description': description
						}
						
		if i != len(localfunctions) - 1:
			overview += '<hr size="1">'
	
	text = text.replace('{MODULEFUNCTIONS}', overview)
			
	# Create a function overview first
	overview = ''
	cols = 4
	rows = int(math.ceil(len(localconstants) / 4.0))
	
	localconstants.sort(namesort)
	
	for row in range(0, rows):
		if row == 0:
			overview += """
			<br><strong>Constants:</strong>
			<table width="100%"  border="0" cellspacing="0" cellpadding="2">"""
		
		overview += "<tr>\n"
	
		for col in range(0, cols):
			id = col * rows + row
			if id < len(localconstants):
				constant = localconstants[id]
				overview += '<td width="25%%">-&#160;<a href="#const_%s">%s</a></td>' % (urllib.quote(constant['name'].lower()), constant['name'])
			else:
				overview += "<td width=\"25%\">&nbsp;</td>\n";
	
		overview += "</tr>\n"
	
		if row == rows - 1:
			overview += "</table>"
		
	text = text.replace('{CONSTANTSOVERVIEW}', overview)			
			
	# Generate a list of constants
	overview = ''
	
	if len(localconstants) > 0:
		overview += """<p><span class="sectiontitle">MODULE CONSTANTS</span><br>
			<br>"""
	
	for i in range(0, len(localconstants)):
		constant = localconstants[i]
		consttext = ''
				
		for const in constant['constants']:
			# If this constant is for a number, 
			# color it red, otherwise color it grey
			quotecolor = re.compile('((?<!\\\\)".*?(?<!\\\\)")')
			curpos = 0
			while 1:
				result = quotecolor.search(const, curpos)
				if not result:
					break
				newconst = const[:result.start()] + '<font color="#008000">%s</font>' % result.group(0)
				curpos = len(newconst)
				newconst += const[result.end():]
				const = newconst
				
			quotecolor = re.compile('\\#.*')
			curpos = 0
			while 1:
				result = quotecolor.search(const, curpos)
				if not result:
					break
				newconst = const[:result.start()] + '<font color="#008000">%s</font>' % result.group(0)
				curpos = len(newconst)
				newconst += const[result.end():]
				const = newconst				
				
			consttext += const
			consttext += "<br>\n"

		if len(constant['description']) > 0:
			constant['description'] += '<br>'		

		overview += "<a name=\"const_%(anchor)s\"></a><b><code style=\"font-size: 12px\">%(name)s</code></b><br />\
					%(description)s<br/><code>%(constants)s</code>\n\
						<br /><a href=\"#top\">Back to top</a>\n" % {
							'name': constant['name'],
							'description': constant['description'],
							'anchor': urllib.quote(constant['name'].lower()),
							'constants': consttext
						}
						
		if i != len(localconstants) - 1:
			overview += '<hr size="1">'			
						
	text = text.replace('{MODULECONSTANTS}', overview)
	
	output = open('webroot/module_%s.html' % module.replace('.', '_').lower(), "wt")
	output.write(text)
	output.close()

# Generate index page
template = open('templates/index.html')
text = template.read()
template.close()

text = text.replace('{GENERATED}', generated)
text = text.replace('{VERSION}', version)
text = text.replace('{COMMANDS}', str(len(commands)))
text = text.replace('{OBJECTS}', str(len(objects)))
text = text.replace('{EVENTS}', str(len(events)))
text = text.replace('{MODULES}', str(len(modules)))

output = open('webroot/index.html', "wt")
output.write(text)
output.close()

output = open('version', 'wt')
output.write(str(getVersionNumber()))
output.close()
