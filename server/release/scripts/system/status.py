
import random
import wolfpack
import re
from StringIO import StringIO
import sys
from wolfpack.consts import *
from wolfpack import settings, console, accounts, sockets
import wolfpack.time # Already imported
import htmlentitydefs
from threading import Thread, Event, Lock
import time

magic = 0.0
interval = 60000
htmltemplate = 'web/status_template.html'
outputfile = 'web/status.html'
processthread = None
templatemodule = None
		
#
# The processing thread
#
class ProcessThread(Thread):
	# Initialize this thread
	def __init__(self):
		Thread.__init__(self)
		self.stopped = Event() # Cancel Event
		self.mutex = Lock()
		self.data = None
	
	# Threaded code
	def run(self):
		while not self.stopped.isSet():
			# Check if status data is available and process it
			data = None
			self.mutex.acquire()
			if self.data:
				data = self.data
				self.data = None
			self.mutex.release()
			
			if data:
				global outputfile
				try:
					fp = file(outputfile, 'wu')
					fp.write(data.encode('utf-8'))
					fp.close()
				except Exception, e:
					console.log(LOG_PYTHON, "Couldn't write status to '%s': %s.\n" % (outputfile, str(e)))
			
			self.stopped.wait(0.5)

#
# Status generation routine
#
def generate(object, arguments):
	global magic, interval

	# The script has been reloaded
	if arguments[0] != magic:
		return
	
	global templatemodule
	
	if not templatemodule:
		global htmltemplate
		try:
			fp = file(htmltemplate, 'ru')
			template = fp.read()
			fp.close()
			console.log(LOG_MESSAGE, "Loaded status template from '%s'.\n" % htmltemplate)
		except Exception, e:
			console.log(LOG_PYTHON, "Couldn't load status template from '%s': %s.\n" % (htmltemplate, str(e)))
			return
	
		# Search for python codeblocks <%= %>
		length = len(template)
		blocks = ''
		current = ''
		inlineprint = 0
		
		# PROLOG
		try:
			for i in range(0, length):
				if template[i:i + 3] == "<%\n":
					# Dump previous string					
					if current != '':
						if inlineprint:
							blocks += '+ ' + repr(current)
						else:
							blocks += 'print ' + repr(current)
						current = ''
					inlineprint = 0
					continue
				elif template[i - 1:i+1] == "%>":
					# Gather code
					current = current[1:-1].strip("%")
					
					if inlineprint:
						blocks += " + unicode(%s)" % current[1:]
					else:
						blocks += "\n%s" % current
						inlineprint = 0
					current = ''
					continue
				elif template[i:i + 3] == "<%=":
					# Dump previous string
					if current != '':
						if inlineprint:
							blocks += '+ ' + repr(current)
						else:
							blocks += 'print ' + repr(current)
						current = ''
					inlineprint = 1
					continue
				current += template[i]
			if inlineprint:
				blocks += '+ ' + repr(current)
			else:
				blocks += 'print ' + repr(current)
			blocks += "\n"
			
		except Exception, e:
			console.log(LOG_PYTHON, "Unable to parse python template file: %s\n" % str(e))
			return		

		# Try to compile the file
		try:
			templatemodule = compile(blocks, htmltemplate, 'exec')
		except Exception, e:
			console.log(LOG_PYTHON, "Unable to compile python template file: %s\n" % str(e))
			templatemodule = None
			return			
			
	# Try to execute the code
	savedstdout = sys.stdout
	sys.stdout = StringIO()
	output = sys.stdout
	exec templatemodule
	sys.stdout = savedstdout
	
	text = output.getvalue()
	output.close()

	# Collect data and pass it to the processing thread
	global processthread
	
	if processthread:
		processthread.mutex.acquire()
		processthread.data = text
		processthread.mutex.release()

	# Re-execute after interval miliseconds
	wolfpack.addtimer(interval, "system.status.generate", [magic])

#
# Initialization
#
def onLoad():
	global magic
	magic = random.random()
	loadSettings()

	global processthread
	processthread = ProcessThread()
	processthread.start()

	wolfpack.addtimer(interval, "system.status.generate", [magic])
		
#
# Stop the thread
#
def onUnload():
	global processthread
	if processthread:
		processthread.stopped.set()
		time.sleep(0.01) # Sleep a little to give the thread time to exit
		if processthread:
			processthread.join()

#
# Load settings
#
def loadSettings():
	global interval, htmltemplate, outputfile
	interval = settings.getnumber("HTML Status", "Interval in Seconds", 60, True) * 1000
	htmltemplate = settings.getstring("HTML Status", "HTML Template", "web/status_template.html", True)
	outputfile = settings.getstring("HTML Status", "Output Path", "web/status.html", True)
