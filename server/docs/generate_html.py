
import math
import time
import os
import os.path
import sys
from parse import parsepython, parsecpp, getVersion
from glob import glob

paths = sys.argv[1:]
commands = []
events = []
objects = []
objectsmethods = []
objectsproperties = []

if len(paths) == 0:
	print "Usage: python generate_html.py path1[,path2,...]"
	sys.exit()

try:
	os.mkdir('output')
except:
	pass

def examine(path):
	global commands
	global events
	global objects
	global objectsmethods
	global objectsproperties
	
	#print "Examining %s..." % path
	files = glob(path + '/*.py')
	
	for file in files:
		if os.path.isfile(file):
			(newcommands, newevents, newobjects, newobjectsmethods, newobjectsproperties) = parsepython(file)
			commands += newcommands
			events += newevents
			
	files = glob(path + '/*.cpp')
	
	for file in files:
		if os.path.isfile(file):
			(newcommands, newevents, newobjects, newobjectsmethods, newobjectsproperties) = parsecpp(file)
			commands += newcommands
			events += newevents
			objects += newobjects
			objectsmethods += newobjectsmethods
			objectsproperties += newobjectsproperties
			
	files = glob(path + '/*.h')
	
	for file in files:
		if os.path.isfile(file):
			(newcommands, newevents, newobjects, newobjectsmethods, newobjectsproperties) = parsecpp(file)
			commands += newcommands
			events += newevents
			objects += newobjects
			objectsmethods += newobjectsmethods
			objectsproperties += newobjectsproperties			
	
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

print generated
# print "INSERT INTO documentation_settings VALUES('generated', %u);" % int(time.time())
# print "INSERT INTO documentation_settings VALUES('version', '%s');" % (quote(getVersion()))

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

output = open('output/commands.html', "wt")
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

	output = open('output/command_%s.html' % command['name'].lower(), "wt")
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

output = open('output/events.html', "wt")
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
	
	output = open('output/event_%s.html' % event['name'].lower(), "wt")
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

output = open('output/objects.html', "wt")
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
				overview += '<td>-&#160;<a href="#meth_%s">%s</a></td>' % (method['method'].lower(), method['method'])
		else:
			overview += "<td>&nbsp;</td>\n";
	
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
			if id < len(methods):
				property = properties[id]
				overview += '<td>-&#160;<a href="#prop_%s">%s</a></td>' % (property['property'].lower(), property['property'])
		else:
			overview += "<td>&nbsp;</td>\n";
	
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
	
	output = open('output/object_%s.html' % object['object'].lower(), "wt")
	output.write(text)
	output.close()
			
	#print "INSERT INTO documentation_objects VALUES('%s', '%s');" % (quote(object['object']), quote(object['description']))
	
#for method in objectsmethods:
	#print "INSERT INTO documentation_objects_methods VALUES('%s', '%s', '%s', '%s', '%s', '%s');" % (quote(method['object']), quote(method['method']), quote(method['prototype']), quote(method['parameters']), quote(method['returnvalue']), quote(method['description']))

#for property in objectsproperties:
	#print "INSERT INTO documentation_objects_properties VALUES('%s', '%s', '%s', '%s');" % (quote(property['object']), quote(property['property']), quote(property['description']), quote(property['readonly']))
