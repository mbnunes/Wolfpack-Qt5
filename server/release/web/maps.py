
# Displays Players on a Map

import web.sessions
import web.template
import cgi
import sys
import re
import wolfpack
import wolfpack.sockets
import wolfpack.accounts
import wolfpack.time

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
<h1>Players on a Map</h1>
Below you see the Players on the Map. Use your browsers refresh button to see the current output.<br>
<table border="0">
  <tr><td>
"""

content += """
<br />
<a href="
"""
content +=	'map_trammel.py?session=%(session)s">Trammel</a><br /><br />'% { 'session': session_id }

content +=	"""
<a href="
"""
content += 'map_malas.py?session=%(session)s">Malas</a><br /><br />'% { 'session': session_id }

content += """
<a href="
"""

content += 'map_ilshenar.py?session=%(session)s">Ilshenar</a><br /><br />'% { 'session': session_id }

content += """
<a href="
"""
content += 'map_tokuno.py?session=%(session)s">Tokuno</a><br /><br />' % { 'session': session_id }

content += """
	</td>
  </tr>
</table>
</p>
"""

web.template.output( 	'&gt; <a href="map.py?session=%(session)s" class="header">Players on a Map</a>' % { 'session': session_id }, 
						'<i>Logged in as:</i> %(username)s<br /><a href="logout.py?session=%(session)s" class="header">Logout</a>' % { 'username': username, 'session': session_id }, 
						1, 
						content,
						session_id
					)

