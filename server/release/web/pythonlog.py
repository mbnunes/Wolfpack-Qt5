
# Displays the Python log

import web.sessions
import web.template
import cgi
import sys
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
<h1>Python Log</h1>
Below you see the Python Logfile. Use your browsers refresh button in order to see the current output.<br>
<textarea cols="70" rows="25" readonly="readonly" style="background-color: #004025; color: #EFEFEF">
"""

try:
	file = open( 'python.log', 'r' )
	content += file.read()
	file.close()
except:
	pass

content += """
</textarea>
</p>
"""

web.template.output( 	'&gt; <a href="pythonlog.py?session=%(session)s" class="header">Python Log</a>' % { 'session': session_id }, 
						'<i>Logged in as:</i> %(username)s<br /><a href="logout.py?session=%(session)s" class="header">Logout</a>' % { 'username': username, 'session': session_id }, 
						1, 
						content,
						session_id
					)

