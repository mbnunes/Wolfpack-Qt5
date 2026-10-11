# -*- coding: cp1252 -*-

import random
import wolfpack
import re
from io import StringIO
import sys
from wolfpack.consts import *
from wolfpack import settings, console, accounts, sockets
import wolfpack.time # Already imported
import html.entities as htmlentitydefs
from threading import Thread, Event, Lock
import time

magic = 0.0
interval = 6000
htmltemplate = u'web/status_template.html'
outputfile = u'web/status.html'
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
					fp = open(outputfile, 'wb')
					fp.write(data.encode('utf-8'))
					fp.close()
				except Exception as e:
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
			fp = open(htmltemplate, 'r', encoding='utf-8', errors='ignore')
			template = fp.read()
			fp.close()
			console.log(LOG_MESSAGE, "Loaded status template from '%s'.\n" % htmltemplate)
		except Exception as e:
			console.log(LOG_PYTHON, "Couldn't load status template from '%s': %s.\n" % (htmltemplate, str(e)))
			return

		# Search for python codeblocks <%= %>
		length = len(template)
		blocks = ''
		current = ''
		i = 0

		# PROLOG
		try:
			while i < length:
				if template[i:i + 3] == "<%=":
					if current:
						blocks += 'sys.stdout.write(' + repr(current) + ')\n'
						current = ''
					end = template.find("%>", i + 3)
					if end != -1:
						code = template[i + 3:end].strip()
						blocks += 'sys.stdout.write(str(' + code + '))\n'
						i = end + 2
					else:
						current += template[i]
						i += 1
				elif template[i:i + 2] == "<%":
					if current:
						blocks += 'sys.stdout.write(' + repr(current) + ')\n'
						current = ''
					end = template.find("%>", i + 2)
					if end != -1:
						code = template[i + 2:end]
						blocks += '\n' + code + '\n'
						i = end + 2
					else:
						current += template[i]
						i += 1
				else:
					current += template[i]
					i += 1

			if current:
				blocks += 'sys.stdout.write(' + repr(current) + ')\n'

		except Exception as e:
			console.log(LOG_PYTHON, "Unable to parse python template file: %s\n" % str(e))
			return

		# Try to compile the file
		try:
			templatemodule = compile(blocks, htmltemplate, 'exec')
		except Exception as e:
			console.log(LOG_PYTHON, "Unable to compile python template file: %s\n" % str(e))
			templatemodule = None
			return

	# Re-execute after interval miliseconds
	wolfpack.addtimer(interval, generate, [magic])

	# Try to execute the code
	savedstdout = sys.stdout
	sys.stdout = StringIO()
	output = sys.stdout
	exec( templatemodule )
	sys.stdout = savedstdout

	text = output.getvalue()
	output.close()

	# Collect data and pass it to the processing thread
	global processthread

	if processthread:
		processthread.mutex.acquire()
		processthread.data = text
		processthread.mutex.release()

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

	wolfpack.addtimer(interval, generate, [magic])

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
