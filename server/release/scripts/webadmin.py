#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: DarkStorm                      #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#
# Remote Administration Webserver                               #
#===============================================================#

from threading import Thread, Event
import time
from whrandom import random
from CGIHTTPServer import CGIHTTPRequestHandler
from BaseHTTPServer import HTTPServer
import os
import posixpath
import urllib
import sys
import socket
import cStringIO
import traceback
import web.sessions

# Just override handle_error for nicer error handling
class Webserver( HTTPServer ):
    def handle_error(self, request, client_address):
		# Ignore Socket Errors && IOErrors
		if sys.exc_type == socket.error or sys.exc_type == IOError:
			dummy = cStringIO.StringIO()
			traceback.print_exc( None, dummy )
			dummy.close()
			return

		print  '-'*40
		print  'Exception happened during processing of request from ' + str( client_address )
		
		traceback.print_exc() # XXX But this goes to stderr!
		print '-'*40

# Custom Request Handler
class WebserverHandler( CGIHTTPRequestHandler ):
	basepath = 'web/'

	# We ONLY want execfile
	have_fork = 0
	have_popen2 = 0
	have_popen3 = 0

	# CGIs are allowed everywhere
	def is_cgi( self ):
		path = self.path

		# Extract directory and stuff behind query sign (?)
		i = path.rfind( '?' )
		       
		if i >= 0:
			scriptpath, query = path[:i], path[i+1:]
		else:
			scriptpath = path
			query = ''

		if self.is_python( scriptpath ):
			path = posixpath.dirname( scriptpath )
			script = posixpath.basename( scriptpath ) + '?' + query
			self.cgi_info = path, script
			return 1

		return 0

	def translate_path(self, path):
		path = posixpath.normpath(urllib.unquote(path))
		words = path.split('/')
		words = filter(None, words)
		path = self.basepath
		for word in words:
			drive, word = os.path.splitdrive(word)
			head, word = os.path.split(word)
			if word in (os.curdir, os.pardir): continue
			path = os.path.join(path, word)
		return path

	def log_message( self, format, *args ):
		try:
			file = open( 'web.log', 'a' )
			file.write( "%s - - [%s] %s\n" % ( self.address_string(), self.log_date_time_string(), format%args ) )
			file.close()
		except:
			print "Could not write to logfile: web.log\n"

class WebserverThread(Thread):
	def __init__( self, port=2594 ):
		Thread.__init__( self )
		self.finished = Event()
		self.port = port

	def cancel( self ):
		self.finished.set()

	def run( self ):
		# Wait with binding the webserver for 5 Seconds
		server_address = ( '', self.port )
		time.sleep( 5 )	

		try:
			httpd = Webserver( server_address, WebserverHandler )
		except:
			print "error occured"

		while not self.finished.isSet():
			httpd.handle_request()
			time.sleep( 0.001 )

		self.finished.clear()

		httpd.server_close()

thread = None

def onLoad():
	web.sessions.clear_sessions()

	# Start the Thread
	global thread
	thread = WebserverThread( 2594 )
	thread.start()

def onUnload():
	# Stop the Thread
	global thread
	if thread:
		thread.cancel()

	web.sessions.clear_sessions()
