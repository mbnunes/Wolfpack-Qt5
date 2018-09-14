
import web.sessions
import web.template
import cgi
import sys

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
<h1>Main</h1>
Welcome to the Wolfpack remote administration console.
</p>
"""

web.template.output( 	'&gt; <a href="main.py?session=%(session)s" class="header">Main</a>' % { 'session': session_id }, 
						'<i>Logged in as:</i> %(username)s<br /><a href="logout.py?session=%(session)s" class="header">Logout</a>' % { 'username': username, 'session': session_id }, 
						1, 
						content,
						session_id
					)

