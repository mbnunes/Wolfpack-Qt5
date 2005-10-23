
import sys
import os
import glob

# These may be globbed
FILENAMES = ('openal32.dll', )
ARCHIVE = 'updates\\openal.7z'
SOURCEPATH = "../"
COMPRESSOR = 'updates\\7za.exe'

os.chdir(SOURCEPATH)

# "Press any key to continue..."
def pause():
	os.system("pause")
	
def compileFileList(pattern):
	result = []
	
	# Glob the pattern and apply it to all subdirs if it contains a * or ?
	if not ('?' in pattern or '*' in pattern):
		result.append(pattern)
	else:
		filelist = glob.glob(pattern)
		
		for name in filelist:
			if os.path.isfile(name):
				result.append(name)
			elif os.path.isdir(name):
				if name != '.' and name != '..':
					# Split it into a pattern and a dir part
					p = pattern.rfind('/')
					newpattern = pattern[:p+1] + name + '/' + pattern[p+1:]					
					result += compileFileList(newpattern)
				
	return result

if os.name != 'nt':
	print "This program can only be run on Windows based systems."	
	sys.exit()

print "---------------------------------------------"
print "This update will include the following files:"
print "---------------------------------------------"

fileList = []

# Compile file list recursively
for pattern in FILENAMES:
	fileList += compileFileList(pattern)

for name in fileList:
	print name

pause()

# Build arguments
syscall = COMPRESSOR + ' a -r ' + ARCHIVE

for name in fileList:
	syscall += ' "' + name + '"'

if os.path.isfile(ARCHIVE):
	try:
		os.remove(ARCHIVE)
	except Exception, message:
		print str(Exception) + ':' + str(message)
		pause()
		sys.exit()
os.system(syscall)

pause()
