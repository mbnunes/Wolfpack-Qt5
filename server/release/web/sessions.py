
import random
import time
import web.template
import os.path
import os

sessions = {}
valid_keys = "abcdefghijklmnopqrstuvwxyz0123456789"

"""
	Saves a session on disk
"""
def save_session( id, data ):
	if not os.path.exists( 'sessions' ):
		os.mkdir( 'sessions', 0700 )

	try:
		session = open( 'sessions/' + id + '.dat', 'w' )
		session.write( data[ 'username' ] + "\n" )
		session.write( str( data[ 'expire' ] ) + "\n" )
		session.close()
	except:
		return 0

	return 1

"""
	Tries to load a session from disk, returns None if there is none
"""
def load_session( id ):
	if not os.path.exists( 'sessions/' + id + '.dat' ):
		return None

	try:
		session = open( 'sessions/' + id + '.dat', 'r' )
		username = session.readline()
		expire = session.readline()
		session.close()

		# Strip Newlines
		username = username.replace( "\n", "" )
		expire = int( expire.replace( "\n", "" ) )

		# Whenever loading a session, reset the expiretime
		session = open( 'sessions/' + id + '.dat', 'w' )
		session.write( username + "\n" )
		session.write( str( expire + 900 ) + "\n" )
		session.close()

		return { 'username': username, 'expire': expire }
	except:
		return None

"""
	Creates a session and returns the session id
"""
def start_session( username ):
	usedkeys = []

	while 1:
		session_key = ''
		for i in range( 1, 20 ):
			session_key += random.choice( valid_keys )
		if not os.path.exists( 'sessions/' + session_key + '.dat' ):
			break

	session = {
		'username': username,
		'expire': int( time.time() ) + 900
	}

	save_session( session_key, session )

	return session_key

"""
	Try to get the username for a given session-id
	If no session is registered it returns None
"""
def get_session( session_id ):
	return load_session( session_id )

"""
	Destroys a session with the given session id
"""
def destroy_session( session_id ):
	os.remove( 'sessions/' + session_id + '.dat' )

"""
	Check for session timeouts
"""
def check_timeouts():
	files = os.listdir( 'sessions/' )
	for file in files:
		if file.endswith( '.dat' ):
			session = open( 'sessions/' + file, 'r' )
			session.readline()
			expire = int( session.readline() )
			session.close()
			if expire <= time.time():
				os.remove( 'sessions/' + file )

"""
	Clears all currently opened sessions
"""
def clear_sessions():
	if not os.path.exists( 'sessions' ):
		os.mkdir( 'sessions', 0700 )

	files = os.listdir( 'sessions/' )
	for file in files:
		if file.endswith( '.dat' ):
			os.remove( 'sessions/' + file )	

""" 
	Display a page notifying the user that his session timed out
"""
def display_timeout():
	web.template.output( '&gt; <a href="login.py">Login</a>', '', 0, 'Your session timed out.<br />Click <a href="login.py">here</a> to return to the login page.', 0 )

