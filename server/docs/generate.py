
import time
import os
import os.path
import sys
from parse import parsepython, parsecpp
from glob import glob

paths = sys.argv[2:]
commands = []

if len(paths) == 0:
	print "Usage: python generate.py path1[,path2,...]"
	sys.exit()

def examine(path):
	global commands
	#print "Examining %s..." % path
	files = glob(path + '/*.py')
	
	for file in files:
		if os.path.isfile(file):
			commands += parsepython(file)
			
	files = glob(path + '/*.cpp')
	
	for file in files:
		if os.path.isfile(file):
			commands += parsecpp(file)
			
	files = glob(path + '/*.h')
	
	for file in files:
		if os.path.isfile(file):
			commands += parsecpp(file)
	
	# Get subdirectories and process them
	entries = glob(path + '/*')
	for entry in entries:
		if os.path.isdir(entry):
			examine(entry)
			
for path in paths:
	examine(path)

print "DELETE FROM documentation_commands;"
print "DELETE FROM documentation_settings;"

# Time
print "INSERT INTO documentation_settings VALUES('generated', %u);" % int(time.time())
print "INSERT INTO documentation_settings VALUES('version', '12.9.4');"

def quote(text):
	return text.replace("'", "\\'")

for command in commands:
	print "INSERT INTO documentation_commands VALUES('%s', '%s', '%s', '%s');" % (quote(command['name']), quote(command['description']), quote(command['usage']), quote(command['notes']))
