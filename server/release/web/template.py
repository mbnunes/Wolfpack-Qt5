
def output( navigation, username, navbar, content, session_id ):

	try:
		file = open( 'web/template.html', 'r' )
		template = file.read()
		file.close()
	except:
		template = "<b>ERROR:</b> Couldn't read template.html."

	template = template.replace( '%NAVIGATION%', navigation )

	if navbar:
		navigation = '<br /><br />'
		navigation += '- <a href="main.py?session=%s">Main</a><br />' % session_id
		navigation += '- <a href="console.py?session=%s">Console</a><br />' % session_id
		navigation += '- <a href="pythonlog.py?session=%s">Python Log</a><br />' % session_id
		navigation += '- <a href="accounts.py?session=%s">Accounts</a><br />' % session_id
	else:
		navigation = ''

	template = template.replace( '%USERNAME%', username )
	template = template.replace( '%NAVBAR%', navigation )
	template = template.replace( '%CONTENT%', content )
	
	print "Content-type: text/html\n\n"
	print template
