import SimpleHTTPServer
import os
import posixpath
import BaseHTTPServer
import urllib
import cgi
import shutil
import mimetypes
import string
from StringIO import StringIO
import thread
import struct

import wolfpack.console
import wolfpack.server
import wolfpack

class HTMLTemplate:
   def __init__(self, template):
      f_in = open(template, "r")
      self.htmlText = f_in.read()
      f_in.close

   def substitute(self, **params):
      for arg in params:
         replaceString = "<% " + arg + " %>"
         self.htmlText = string.replace(self.htmlText,
                  replaceString, params[arg])

   def writeHTML(self):
      f = StringIO()
      f.write(self.htmlText)
      f.seek( 0 )
      return f

class WPHTTPServer( SimpleHTTPServer.SimpleHTTPRequestHandler ):

	# Serves a request
	def do_GET(self):
		if (self.path == "/SAVE" ):
			wolfpack.server.save()
	        	self.send_response(200)
       			self.send_header("Content-type", "text/html")
        		self.end_headers()
        		self.wfile.write( "World has been saved" )
			return
	
		if( self.path == "/CLIENTS" ):
			self.send_response( 200 )
			self.send_header( "Content-type", "text/html" )
			self.end_headers()
			self.wfile.write( str( len( wolfpack.server.clients ) ) )
			
			gump = wolfpack.gump()
			gump.nomove = 1
			gump.noclose = 1
			
			gump.addPage( 0 )
			gump.addBackground( 0, 0, 5120, 320, 340 )
			gump.addText( 10, 10, 0, "my Text" )

			gump.send( wolfpack.server.clients[ 0 ] )
			
			#speech = struct.pack( "!BB", 0x4F, 0x09 )
			#wolfpack.server.clients[ 0 ].send( speech )
			
			#wolfpack.server.clients[ 0 ].char.message( "It works, damnit!" )
			return
	
		f = self.send_head()
		if f:
			self.copyfile(f, self.wfile)
			f.close()

	# Serves a head request
    	def do_HEAD(self):
		f = self.send_head()
        	if f:
            		f.close()	
            			
    	def send_head(self):    
        	self.send_response(200)
        	self.send_header("Content-type", "text/html")
        	self.end_headers()

		Template = HTMLTemplate( "test.html" )
		Template.substitute( name = "Wolfpack" )
		return Template.writeHTML()

def runServer():
	port = 8000
	server_address = ('', port)
	HandlerClass = WPHTTPServer
	
	httpd = BaseHTTPServer.HTTPServer(server_address, HandlerClass)
	httpd.serve_forever()

def onServerStart():
	wolfpack.console.progress( "Starting HTTP Server on Port 8000" )
	thread.start_new_thread( runServer, () )
	wolfpack.console.printDone()

