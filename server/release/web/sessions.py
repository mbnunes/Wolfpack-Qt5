
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
		os.mkdir( 'sessions', 0o700 )

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
		os.mkdir( 'sessions', 0o700 )

	files = os.listdir( 'sessions/' )
	for file in files:
		if file.endswith( '.dat' ):
			os.remove( 'sessions/' + file )	

""" 
	Display a page notifying the user that his session timed out
"""
def display_timeout():
    content = """<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="utf-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1" />
  <title>Sessão Expirada - Wolfpack Administration</title>
  
  <!-- Bootstrap 5 CSS -->
  <link rel="stylesheet" href="css/bootstrap.min.css" />
  <link rel="stylesheet" href="css/custom.css" />

  <style>
    body {
      background-color: #f8fafc;
      min-height: 100vh;
      display: flex;
      flex-direction: column;
    }
    .wp-header {
      background-color: #004026;
      border-bottom: 2px solid #002b1a;
      height: 60px;
    }
  </style>
</head>
<body class="d-flex flex-column min-vh-100">

  <!-- Topbar com Logo Oficial -->
  <header class="wp-header shadow-sm">
    <div class="container-fluid px-3 px-md-4 d-flex align-items-center h-100">
      <a href="login.py" class="d-flex align-items-center text-decoration-none">
        <img src="images/top_logo.jpg" alt="Wolfpack Logo" class="img-fluid" style="max-height: 48px;" />
      </a>
    </div>
  </header>

  <!-- Card Centralizado na Tela -->
  <main class="container d-flex flex-grow-1 align-items-center justify-content-center py-4">
    <div class="col-12 col-sm-10 col-md-7 col-lg-5 col-xl-4">
      <div class="card border border-light-subtle shadow-sm p-4 text-center bg-white rounded-3">
        
        <!-- Ícone de Atenção / Sessão Expirada -->
        <div class="mb-3">
          <span class="d-inline-flex align-items-center justify-content-center rounded-circle bg-warning-subtle text-warning-emphasis" style="width: 56px; height: 56px; font-size: 1.5rem;">
            &#9200;
          </span>
        </div>

        <h4 class="fw-bold text-dark mb-2">Sessão Expirada</h4>
        <p class="text-secondary small mb-4">
          Sua sessão de administração remota expirou por inatividade. Por motivos de segurança, autentique-se novamente.
        </p>

        <!-- Botão de Retorno -->
        <a href="login.py" class="btn text-white w-100 py-2 fw-semibold shadow-sm" style="background-color: #004026;">
          Retornar ao Login
        </a>
      </div>
    </div>
  </main>

  <!-- Rodapé -->
  <footer class="py-3 bg-white border-top text-center text-secondary small">
    Wolfpack-Qt5 &bull; Remote Administration Console
  </footer>

  <!-- Bootstrap 5 JS -->
  <script src="js/bootstrap.bundle.min.js"></script>
</body>
</html>
"""
    print("Content-type: text/html; charset=utf-8\n\n")
    print(content)
