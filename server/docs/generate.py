
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
	print "Usage: python generate.py path1[,path2,...]"
	sys.exit()

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

print "DELETE FROM documentation_commands;"
print "DELETE FROM documentation_settings;"
print "DELETE FROM documentation_events;"
print "DELETE FROM documentation_objects;"
print "DELETE FROM documentation_objects_methods;"
print "DELETE FROM documentation_objects_properties;"

# Time
print "INSERT INTO documentation_settings VALUES('generated', %u);" % int(time.time())
print "INSERT INTO documentation_settings VALUES('version', '%s');" % (quote(getVersion()))

for command in commands:
	print "INSERT INTO documentation_commands VALUES('%s', '%s', '%s', '%s');" % (quote(command['name']), quote(command['description']), quote(command['usage']), quote(command['notes']))

for event in events:
	print "INSERT INTO documentation_events VALUES('%s', '%s', '%s', '%s', '%s', '%s');" % (quote(event['name']), quote(event['prototype']), quote(event['parameters']), quote(event['returnvalue']), quote(event['callcondition']), quote(event['notes']))

for object in objects:
	# Copy methods and properties from inherited object types
	for method in objectsmethods:
	
		if method['object'] in object['inherit']:
			method = dict(method)
			method['object'] = object['object']
			objectsmethods.append(method)
			
	for property in objectsproperties:
		if property['object'] in object['inherit']:
			property = dict(property)
			property['object'] = object['object']
			objectsproperties.append(property)

	print "INSERT INTO documentation_objects VALUES('%s', '%s');" % (quote(object['object']), quote(object['description']))
	
for method in objectsmethods:
	print "INSERT INTO documentation_objects_methods VALUES('%s', '%s', '%s', '%s', '%s', '%s');" % (quote(method['object']), quote(method['method']), quote(method['prototype']), quote(method['parameters']), quote(method['returnvalue']), quote(method['description']))

for property in objectsproperties:
	print "INSERT INTO documentation_objects_properties VALUES('%s', '%s', '%s', '%s');" % (quote(property['object']), quote(property['property']), quote(property['description']), quote(property['readonly']))
