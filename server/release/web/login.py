
import wolfpack.accounts
import cgi
import web.sessions
import sys

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
	elif !account.checkpassword(password):
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

	print "Content-type: text/html\n\n"
	print """
<?xml version="1.0" encoding="iso-8859-1" ?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/strict.dtd">
<html>
<head>
	<title>You were logged in succesfully</title>
	<meta http-equiv="refresh" content="1; URL=main.py?session=""" + session + """" />
</head>
<body>
<h1>You were logged in successfully</h1><br />
Click <a href="main.py?session=""" + session + """">here</a> to continue.
</body>
</html>
"""
