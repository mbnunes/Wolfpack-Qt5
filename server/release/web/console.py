
# Displays the Console log!

import web.sessions
import web.template
import cgi
import sys
import wolfpack.console
import re

form = cgi.FieldStorage()
session_id = form.getvalue( 'session', '' )

web.sessions.check_timeouts()
session = web.sessions.get_session( session_id )

if session == None:
	web.sessions.display_timeout()
	sys.exit()

username = session[ 'username' ]

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
	
	content += line + "\n"

content += """
</textarea>
</p>
"""

web.template.output( 	'&gt; <a href="console.py?session=%(session)s" class="header">Console</a>' % { 'session': session_id }, 
						'<i>Logged in as:</i> %(username)s<br /><a href="logout.py?session=%(session)s" class="header">Logout</a>' % { 'username': username, 'session': session_id }, 
						1, 
						content,
						session_id
					)

