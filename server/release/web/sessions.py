
import random
import time
import web.template

sessions = {}
valid_keys = "abcdefghijklmnopqrstuvwxyz0123456789"

"""
	Creates a session and returns the session id
"""
def start_session( username ):
	global sessions

	usedkeys = []

	# Delete sessions for this username
	for key,value in sessions.items():
		#if value[ 'username' ] == username:
		#	del sessions[ key ]
		#else:
			usedkeys.append( key )
	
	while 1:
		session_key = ''
		for i in range( 1, 20 ):
			session_key += random.choice( valid_keys )
		if not session_key in usedkeys:
			break

	session = {
		'username': username,
		'expire': time.time() + 900
	}

	sessions[ session_key ] = session

	return session_key

"""
	Try to get the username for a given session-id
	If no session is registered it returns None
"""
def get_session( session_id ):
	global sessions

	if not sessions.has_key( session_id ):
		return None
	else:
		# Refresh expire time
		sessions[ session_id ][ 'expire' ] = time.time() + 900
		return sessions[ session_id ]

"""
	Destroys a session with the given session id
"""
def destroy_session( session_id ):
	global sessions

	if sessions.has_key( session_id ):
		del sessions[ session_id ]

"""
	Check for session timeouts
"""
def check_timeouts():
	global sessions

	for key, value in sessions.items():
		if value[ 'expire' ] <= time.time():
			del sessions[ key ]

""" 
	Display a page notifying the user that his session timed out
"""
def display_timeout():
	web.template.output( '&gt; <a href="login.py">Login</a>', '', 0, 'Your session timed out.<br />Click <a href="login.py">here</a> to return to the login page.', 0 )

