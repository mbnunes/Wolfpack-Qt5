
# Displays the Console log!

import web.sessions
import web.template
import wolfpack.console
import _wolfpack.console
from wolfpack.consts import *
import sys, cgi, re, string

# print "pretty" buttons
def pretty_button( action_val, caption ):
	rvalue =  '<form action="console.py" method="get">'
	rvalue += '<input type="hidden" name="session" value="%s" />' % session_id
	rvalue += '<input type="hidden" name="action" value="%s" />' % action_val
	rvalue += '<input type="submit" value="%s" />' % caption
	rvalue += '</form>'
	return rvalue

form = cgi.FieldStorage()
session_id = form.getvalue( 'session', '' )

web.sessions.check_timeouts()
session = web.sessions.get_session( session_id )

if session == None:
	web.sessions.display_timeout()
	sys.exit()

try:
	action = int( form.getvalue( 'action', '0' ) )
except:
	pass

username = session[ 'username' ]

if action == 1:
	print "Content-type: text/html\n\n"
	print "Server is reloading, you will have to re-login after reloading is done."
	wolfpack.queueaction( RELOAD_SCRIPTS )
	sys.exit()
elif action == 2:
        print "Content-type: text/html\n\n"
        print "Server is reloading Python scripts, you will have to re-login after reloading is done."
        wolfpack.queueaction( RELOAD_PYTHON )
        sys.exit()
elif action == 3:
        wolfpack.queueaction( RELOAD_ACCOUNTS )
elif action == 4:
        wolfpack.queueaction( RELOAD_CONFIGURATION )
elif action == 5:
        wolfpack.queueaction( SAVE_WORLD )
elif action == 6:
        print "Content-type: text/html\n\n"
        print "Server is shutting down, remote admin stopped."
        _wolfpack.console.shutdown()
        sys.exit()

content = """
<p>
<h1>Console</h1>
Below you see the Wolfpack console output. Use your browsers refresh button in order to see the current output.<br>
<textarea cols="70" rows="25" readonly="readonly" style="background-color: #004025; color: #EFEFEF">
"""

for line in wolfpack.console.getbuffer():
	# Replace \b characters
	i = line.find( "\b" )
	while i >= 0:
		# Take it out
		line = line[:i-1] + line[i+1:]
		i = line.find( "\b" )

	line = string.replace( line, "", "" )
	line = string.replace( line, "[1;37m", "" )
	line = string.replace( line, "[1;32m", "" )
	line = string.replace( line, "[0m", "" )
	
	content += line + "\n"

content += """
</textarea>
</p>
<p>
"""
content += pretty_button( 1, "Reload Scripts" )
content += pretty_button( 2, "Reload Python" )
content += pretty_button( 3, "Reload Accounts" )
content += pretty_button( 4, "Reload Configuration" )
content += pretty_button( 5, "Save World" )
content += pretty_button( 6, "Shutdown" )


web.template.output( 	'&gt; <a href="console.py?session=%(session)s" class="header">Console</a>' % { 'session': session_id }, 
						'<i>Logged in as:</i> %(username)s<br /><a href="logout.py?session=%(session)s" class="header">Logout</a>' % { 'username': username, 'session': session_id }, 
						1, 
						content,
						session_id
					)

