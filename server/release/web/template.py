
def output( navigation, username, navbar, content, session_id ):

	try:
		file = open( 'web/template.html', 'r' )
		template = file.read()
		file.close()
	except:
		template = "<b>ERROR:</b> Couldn't read template.html."

	template = template.replace( '%NAVIGATION%', navigation )

	if navbar:
		navigation = """&nbsp;&nbsp;&nbsp; <a href="main.py?session=%(session)s">Start Page</a><br />
		&nbsp;&nbsp;&nbsp; <a href="console.py?session=%(session)s">Console</a><br />
		&nbsp;&nbsp;&nbsp; <a href="accounts.py?session=%(session)s">Accounts</a><br />
		&nbsp;&nbsp;&nbsp; <a href="pythonlog.py?session=%(session)s">Python Log</a><br />
		<br />
		&nbsp;&nbsp;&nbsp; <a href="logout.py?session=%(session)s">Logout</a>""" % { 'session': session_id }
	else:
		navigation = ''

	template = template.replace( '%USERNAME%', username )
	template = template.replace( '%NAVBAR%', navigation )
	template = template.replace( '%CONTENT%', content )
	
	print "Content-type: text/html\n\n"
	print template
