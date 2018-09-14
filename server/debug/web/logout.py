
# Let's timeout a specific session

import web.sessions
import web.template
import cgi

form = cgi.FieldStorage()
session_id = form.getvalue( 'session', '' )

web.sessions.destroy_session( session_id )

web.template.output( '&gt; <a href="login.py" class="header">Login</a>', '', 0, 'You have been logged out.<br />Click <a href="login.py">here</a> to return to the login page.', '' )
