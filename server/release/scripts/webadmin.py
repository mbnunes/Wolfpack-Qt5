#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: DarkStorm                      #
#   )).-' {{ ;'`   | Revised by: Correa                         #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#
# Remote Administration Webserver                               #
#===============================================================#

from wolfpack.consts import *
from threading import Thread, Event
import os, sys, time, string, socket, urllib
from CGIHTTPServer import CGIHTTPRequestHandler
from BaseHTTPServer import HTTPServer
from SocketServer import BaseServer
import posixpath
import atexit
import cStringIO
import traceback
import web.sessions
import wolfpack

# Just override handle_error for nicer error handling
class Webserver( HTTPServer ):
    def __init__( self, addr, htdocs ):
	BaseServer.__init__( self, addr, WebserverHandler )
	self.socket = socket.socket( self.address_family, self.socket_type )
	self.socket.setblocking( 0 )
	self.server_bind()
	self.server_activate()

	self.htdocs = htdocs
	

    def handle_request(self):
	try:
	    request, client_address = self.get_request()
	    request.setblocking( 1 )
	except socket.error:
	    return
	if self.verify_request(request, client_address):
	    try:
		self.process_request(request, client_address)
	    except:
		self.handle_error(request, client_address)
		self.close_request(request)

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

    def translate_path(self, uri):
        """ Translate a /-separated PATH to the local filename syntax.

            Components that mean special things to the local file system
            (e.g. drive or directory names) are ignored.

        """
        file = urllib.unquote(uri)
        file.replace("\\", '/')
        words = file.split('/')
        words = filter(None, words)

        path = self.server.htdocs
        bad_uri = 0
        for word in words:
            drive, word = os.path.splitdrive(word)
            if drive:
                bad_uri = 1
            head, word = os.path.split(word)
            if word in (os.curdir, os.pardir):
                bad_uri = 1
                continue
            path = os.path.join(path, word)

        if bad_uri:
            self.log_error("Detected bad request URI '%s', translated to '%s'" % (uri, path,))
            
        return path

    def run_cgi(self):
        """Execute a CGI script."""
        dir, rest = self.cgi_info
        i = rest.rfind('?')
        if i >= 0:
            rest, query = rest[:i], rest[i+1:]
        else:
            query = ''
        i = rest.find('/')
        if i >= 0:
            script, rest = rest[:i], rest[i:]
        else:
            script, rest = rest, ''
        scriptname = dir + '/' + script
        scriptfile = self.translate_path(scriptname)
	scriptfile = os.path.normpath( scriptfile )

        if not os.path.exists(scriptfile):
            self.send_error(404, "No such CGI script (%s)" % `scriptname`)
            return
        if not os.path.isfile(scriptfile):
            self.send_error(403, "CGI script is not a plain file (%s)" %
                            `scriptname`)
            return
        ispy = self.is_python(scriptname)
        if not ispy:
	    self.send_error(403, "CGI script is not a Python script (%s)" %
                                `scriptname`)
            return

        # Reference: http://hoohoo.ncsa.uiuc.edu/cgi/env.html
        # XXX Much of the following could be prepared ahead of time!
        env = {}
        env['SERVER_SOFTWARE'] = self.version_string()
        env['SERVER_NAME'] = self.server.server_name
        env['GATEWAY_INTERFACE'] = 'CGI/1.1'
        env['SERVER_PROTOCOL'] = self.protocol_version
        env['SERVER_PORT'] = str(self.server.server_port)
        env['REQUEST_METHOD'] = self.command
        uqrest = urllib.unquote(rest)
        env['PATH_INFO'] = uqrest
        env['PATH_TRANSLATED'] = self.translate_path(uqrest)
        env['SCRIPT_NAME'] = scriptname
        if query:
            env['QUERY_STRING'] = query
        host = self.address_string()
        if host != self.client_address[0]:
            env['REMOTE_HOST'] = host
        env['REMOTE_ADDR'] = self.client_address[0]
        # XXX AUTH_TYPE
        # XXX REMOTE_USER
        # XXX REMOTE_IDENT
        if self.headers.typeheader is None:
            env['CONTENT_TYPE'] = self.headers.type
        else:
            env['CONTENT_TYPE'] = self.headers.typeheader
        length = self.headers.getheader('content-length')
        if length:
            env['CONTENT_LENGTH'] = length
        accept = []
        for line in self.headers.getallmatchingheaders('accept'):
            if line[:1] in "\t\n\r ":
                accept.append(line.strip())
            else:
                accept = accept + line[7:].split(',')
        env['HTTP_ACCEPT'] = ','.join(accept)
        ua = self.headers.getheader('user-agent')
        env['HTTP_USER_AGENT'] = ua or ''
        co = filter(None, self.headers.getheaders('cookie'))
	env['HTTP_COOKIE'] = string.join(co, ', ') or ''
        self.send_response(200, "Script output follows")
        decoded_query = query.replace('+', ' ')

	# execute script in this process
	save_env = os.environ
	save_argv = sys.argv
	save_stdin = sys.stdin
	save_stdout = sys.stdout
	save_stderr = sys.stderr
	try:
	    try:
	        os.environ.update(env)
		sys.argv = [scriptfile]
		if '=' not in decoded_query:
		    sys.argv.append(decoded_query)
		sys.stdout = self.wfile
		sys.stdin = self.rfile
		execfile(scriptfile, {"__name__": "__main__"})
		sys.stdout.flush()
	    finally:
		os.environ = save_env
		sys.argv = save_argv
		sys.stdin = save_stdin
		sys.stdout = save_stdout
		sys.stderr = save_stderr
	except SystemExit, sts:
	    self.log_error("CGI script exit status %s", str(sts))
	else:
	    self.log_message("CGI script exited OK")


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
	self.port = port
	self.stopped = Event()
	self.httpd = None

    def cancel( self ):
	self.stopped.set()
	self.httpd.server_close()

    def run( self ):
	# Wait with binding the webserver for 5 Seconds
	server_address = ( '', self.port )
	time.sleep( 5 )	

	# Starting up
	print "Remote Admin running on port %u\n" % self.port

	try:
	    filepath = os.path.normpath( os.path.abspath( 'web/' ) )
	    self.httpd = Webserver( ( '', self.port ), filepath )
	except:
	    traceback.print_exc()
	    return

	while 1:
	    self.httpd.handle_request()

	    self.stopped.wait( 0.05 )
	    if self.stopped.isSet():
		break

	print "Shutting down the Remote Admin.\n"

thread = None

def onServerStart():
	web.sessions.clear_sessions()

	global thread
	thread = WebserverThread( REMOTEADMIN_PORT )
	thread.start()

def onLoad():
	# Not on ServerStart
	if not wolfpack.isstarting():
		web.sessions.clear_sessions()

		# Start the Thread
		global thread
		thread = WebserverThread( REMOTEADMIN_PORT )
		thread.start()

def onUnload():
	# Stop the Thread
	global thread
	if thread:
		thread.cancel()
		time.sleep( 1 )	# This is needed to allow the thread to sync (Remember => global interpreter lock)
		thread.join() # Join with the thread

	web.sessions.clear_sessions()
