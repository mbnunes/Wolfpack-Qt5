
import wolfpack.accounts
import cgi
import web.sessions
import sys
import os
import web.template

reload( web.sessions )

form = cgi.FieldStorage()

username = form.getvalue( 'username', '' )
password = form.getvalue( 'password', '' )
message = None
authenticated = 0

# Try to log the user in
if username != '' and password != '':
	account = wolfpack.accounts.find( username )
	
	# Wrong Account
	if not account:
		message = "Unable to comply, authentication failed."

	# Wrong Password
	elif not account.checkpassword(password):
		message = "Unable to comply, authentication failed."

	# Authorized for Remote Admin?
	elif not account.authorized( 'RemoteAccess', 'login' ):
		message = "Unable to comply, authentication failed."

	# Authentication succeeded
	else:
		authenticated = 1

if not authenticated:
	content = "<h1>Welcome to Wolfpack</h1>"
	
	if message:
		content += "<h4>%s</h4><br>" % message
	
	content += """<!-- Login -->
		<form action="login.py" method="POST">
			Username:<br />
			<input type="text" name="username" /><br /><br />
			Password:<br />
			<input type="password" name="password" /><br /><br />
			<input type="submit" value="Login" />
		</form>"""
	
	try:
		file = open( 'web/template.html', 'r' )
		template = file.read()
		file.close()
	except:
		template = "<b>ERROR:</b> Couldn't read template.html."
	
	template = template.replace( '%NAVIGATION%', '&gt; <a href="login.py" class="header">Login</a>' )
	template = template.replace( '%USERNAME%', '' )
	template = template.replace( '%NAVBAR%', '' )
	template = template.replace( '%CONTENT%', content )
	
	print "Content-type: text/html\n\n"
	print template
else:
	# Create a session
	session = web.sessions.start_session( username )

	web.template.output( '', '', 0, """You have been logged in.<br />Click <a href="main.py?session=%(session)s">here</a> if you are not automatically redirected.
	<meta http-equiv="refresh" content="0;URL=main.py?session=%(session)s" />""" % {'session': session}, '' )
