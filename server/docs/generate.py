
import time
import os
import os.path
import sys
from parse import parsepython, parsecpp, getVersion
from glob import glob

paths = sys.argv[2:]
commands = []
events = []

if len(paths) == 0:
	print "Usage: python generate.py path1[,path2,...]"
	sys.exit()

def examine(path):
	global commands
	global events
	#print "Examining %s..." % path
	files = glob(path + '/*.py')
	
	for file in files:
		if os.path.isfile(file):
			(newcommands, newevents) = parsepython(file)
			commands += newcommands
			events += newevents
			
	files = glob(path + '/*.cpp')
	
	for file in files:
		if os.path.isfile(file):
			(newcommands, newevents) = parsecpp(file)
			commands += newcommands
			events += newevents
			
	files = glob(path + '/*.h')
	
	for file in files:
		if os.path.isfile(file):
			(newcommands, newevents) = parsecpp(file)
			commands += newcommands
			events += newevents
	
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

# Time
print "INSERT INTO documentation_settings VALUES('generated', %u);" % int(time.time())
print "INSERT INTO documentation_settings VALUES('version', '%s');" % (quote(getVersion()))

for command in commands:
	print "INSERT INTO documentation_commands VALUES('%s', '%s', '%s', '%s');" % (quote(command['name']), quote(command['description']), quote(command['usage']), quote(command['notes']))

for event in events:
	print "INSERT INTO documentation_events VALUES('%s', '%s', '%s', '%s', '%s', '%s');" % (quote(event['name']), quote(event['prototype']), quote(event['parameters']), quote(event['returnvalue']), quote(event['callcondition']), quote(event['notes']))
