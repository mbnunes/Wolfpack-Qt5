#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: DarkStorm                      #
#   )).-' {{ ;'`   | Revised by: Correa                         #
#  ( (  ;._ \\ ctr | Last Modification: Python 3 Compatibility  #
#===============================================================#
# Remote Administration Webserver                               #
#===============================================================#

from wolfpack.consts import *
from threading import Thread, Event
import os, sys, time, socket

for _p in [
	'/usr/lib/python3/dist-packages',
	'/usr/local/lib/python3.14/dist-packages',
	'/usr/lib/python3.14/dist-packages',
	'/usr/local/lib/python3/dist-packages',
	'/usr/lib/python3.13/dist-packages',
	'/usr/lib/python3.12/dist-packages',
]:
	if os.path.isdir(_p) and _p not in sys.path:
		sys.path.append(_p)

import urllib.parse
from http.server import CGIHTTPRequestHandler, HTTPServer
from socketserver import BaseServer
import posixpath
import atexit
import traceback
import web.sessions
import wolfpack
import wolfpack.console
from wolfpack import settings

WEBADMIN_LOGGING = 1

# Stream wrappers for CGI script execution
class StreamWriter:
	def __init__(self, raw):
		self.raw = raw
		self.buffer = raw
	def write(self, s):
		if isinstance(s, str):
			s = s.encode('utf-8', 'surrogateescape')
		return self.raw.write(s)
	def writelines(self, lines):
		for line in lines:
			self.write(line)
	def flush(self):
		if hasattr(self.raw, 'flush'):
			self.raw.flush()

class StreamReader:
	def __init__(self, raw, length=None):
		self.raw = raw
		self.buffer = raw
		self.remaining = length
	def read(self, n=-1):
		if self.remaining is not None:
			if self.remaining <= 0:
				return ''
			if n < 0 or n > self.remaining:
				n = self.remaining
			data = self.raw.read(n)
			if isinstance(data, bytes):
				s = data.decode('utf-8', 'surrogateescape')
			else:
				s = data
			self.remaining -= len(data) if isinstance(data, (bytes, str)) else 0
			return s
		data = self.raw.read(n) if n >= 0 else self.raw.read()
		if isinstance(data, bytes):
			return data.decode('utf-8', 'surrogateescape')
		return data
	def readline(self, limit=-1):
		if self.remaining is not None:
			if self.remaining <= 0:
				return ''
			if limit < 0 or limit > self.remaining:
				limit = self.remaining
			data = self.raw.readline(limit)
			if isinstance(data, bytes):
				s = data.decode('utf-8', 'surrogateescape')
			else:
				s = data
			self.remaining -= len(data) if isinstance(data, (bytes, str)) else 0
			return s
		data = self.raw.readline(limit)
		if isinstance(data, bytes):
			return data.decode('utf-8', 'surrogateescape')
		return data

# Just override handle_error for nicer error handling
class Webserver( HTTPServer ):
	def __init__( self, addr, htdocs ):
		BaseServer.__init__( self, addr, WebserverHandler )
		self.socket = socket.socket( self.address_family, self.socket_type )
		self.socket.setsockopt( socket.SOL_SOCKET, socket.SO_REUSEADDR, 1 )
		self.socket.setblocking( 0 )
		self.server_bind()
		self.server_activate()

		self.htdocs = htdocs

	def handle_request(self):
		try:
			request, client_address = self.get_request()
			request.setblocking( 1 )
		except (socket.error, OSError):
			return
		if self.verify_request(request, client_address):
			try:
				self.process_request(request, client_address)
			except Exception:
				self.handle_error(request, client_address)
				self.close_request(request)

	def handle_error(self, request, client_address):
		# Ignore Socket Errors && IOErrors
		ex_type, ex_val, tb = sys.exc_info()
		if ex_type and issubclass(ex_type, (socket.error, IOError, OSError, ConnectionResetError, BrokenPipeError)):
			return

		print( '-'*40 )
		print( 'Exception happened during processing of request from ' + str( client_address ) )

		traceback.print_exc()
		print( '-'*40 )

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
		file = urllib.parse.unquote(uri)
		file = file.replace("\\", '/')
		words = file.split('/')
		words = list(filter(None, words))

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

		if bad_uri and WEBADMIN_LOGGING == 1:
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
			self.send_error(404, "No such CGI script (%s)" % repr(scriptname))
			return
		if not os.path.isfile(scriptfile):
			self.send_error(403, "CGI script is not a plain file (%s)" % repr(scriptname))
			return

		ispy = self.is_python(scriptname)
		if not ispy:
			self.send_error(403, "CGI script is not a Python script (%s)" % repr(scriptname))
			return

		# Reference: http://hoohoo.ncsa.uiuc.edu/cgi/env.html
		env = {}
		env['SERVER_SOFTWARE'] = self.version_string()
		env['SERVER_NAME'] = self.server.server_name
		env['GATEWAY_INTERFACE'] = 'CGI/1.1'
		env['SERVER_PROTOCOL'] = self.protocol_version
		env['SERVER_PORT'] = str(self.server.server_port)
		env['REQUEST_METHOD'] = self.command
		uqrest = urllib.parse.unquote(rest)
		env['PATH_INFO'] = uqrest
		env['PATH_TRANSLATED'] = self.translate_path(uqrest)
		env['SCRIPT_NAME'] = scriptname
		if query:
			env['QUERY_STRING'] = query
		host = self.address_string()
		if host != self.client_address[0]:
			env['REMOTE_HOST'] = host
		env['REMOTE_ADDR'] = self.client_address[0]

		content_type = self.headers.get('content-type', '')
		if content_type:
			env['CONTENT_TYPE'] = content_type
		length = self.headers.get('content-length')
		if length:
			env['CONTENT_LENGTH'] = length
		accept = self.headers.get_all('accept')
		if accept:
			env['HTTP_ACCEPT'] = ','.join(accept)
		ua = self.headers.get('user-agent')
		env['HTTP_USER_AGENT'] = ua or ''
		co = self.headers.get_all('cookie')
		if co:
			env['HTTP_COOKIE'] = ', '.join(co)
		else:
			env['HTTP_COOKIE'] = ''

		self.send_response(200, "Script output follows")
		self.flush_headers()
		decoded_query = query.replace('+', ' ')

		# execute script in this process
		save_env = os.environ
		save_argv = sys.argv
		save_stdin = sys.stdin
		save_stdout = sys.stdout
		save_stderr = sys.stderr
		try:
			try:
				try:
					nbytes = int(length)
				except (TypeError, ValueError):
					nbytes = 0
				if hasattr(self.rfile, '_rbufsize') and self.command.lower() == "post" and nbytes > 0:
					self.rfile._rbufsize = nbytes + 2
				os.environ.update(env)
				sys.argv = [scriptfile]
				if '=' not in decoded_query:
					sys.argv.append(decoded_query)
				sys.stdout = StreamWriter(self.wfile)
				sys.stdin = StreamReader(self.rfile, length=nbytes if nbytes > 0 else None)
				with open(scriptfile, 'r', encoding='utf-8', errors='replace') as sf:
					code = sf.read()
				exec(compile(code, scriptfile, 'exec'), {"__name__": "__main__", "__file__": scriptfile})
				sys.stdout.flush()
			finally:
				os.environ = save_env
				sys.argv = save_argv
				sys.stdin = save_stdin
				sys.stdout = save_stdout
				sys.stderr = save_stderr
		except SystemExit as sts:
			if WEBADMIN_LOGGING == 1:
				self.log_error("CGI script exit status %s", str(sts))
		except Exception as e:
			traceback.print_exc()
			if WEBADMIN_LOGGING == 1:
				self.log_error("CGI script exception: %s", str(e))
		else:
			if WEBADMIN_LOGGING == 1:
				self.log_message("CGI script exited OK")

	def log_message( self, format, *args ):
		try:
			os.makedirs('logs', exist_ok=True)
			with open( 'logs/web.log', 'a' ) as file:
				file.write( "%s - - [%s] %s\n" % ( self.address_string(), self.log_date_time_string(), format%args ) )
		except Exception:
			pass

class WebserverThread(Thread):
	def __init__( self, port ):
		Thread.__init__( self )
		self.port = port
		self.stopped = Event()
		self.httpd = None

	def cancel( self ):
		self.stopped.set()

	def run( self ):
		server_address = ('', self.port)

		# Starting up
		if not wolfpack.isreloading():
			wolfpack.console.log(LOG_MESSAGE, "Starting remote admin on port %u.\n" % self.port)

		try:
			filepath = os.path.normpath(os.path.abspath('web/'))
			self.httpd = Webserver(('', self.port), filepath)
		except Exception:
			traceback.print_exc()
			return

		while not self.stopped.is_set():
			try:
				self.httpd.handle_request()
			except Exception:
				traceback.print_exc()
			self.stopped.wait(0.05)

		if wolfpack.isreloading():
			wolfpack.console.log(LOG_MESSAGE, "Restarting remote admin.\n")
		else:
			wolfpack.console.log(LOG_MESSAGE, "Stopping remote admin.\n")

thread = None

def onServerStart():
	web.sessions.clear_sessions()

	# Get the remote admin port
	port = settings.getnumber('Network', 'Remoteadmin Port', 2594, True)

	global thread
	thread = WebserverThread( port )
	thread.start()

def onLoad():
	# Not on ServerStart
	if not wolfpack.isstarting():
		web.sessions.clear_sessions()
		
		# Get the remote admin port
		port = settings.getnumber('Network', 'Remoteadmin Port', 2594, True)
		
		# Start the Thread
		global thread
		thread = WebserverThread( port )
		thread.start()

def onUnload():
	# Stop the Thread
	global thread
	if thread:
		thread.cancel()
		time.sleep(0.3) # This is needed to allow the thread to sync (Remember => global interpreter lock)
		thread.join() # Join with the thread

	web.sessions.clear_sessions()
