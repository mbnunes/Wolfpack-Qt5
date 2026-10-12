import wolfpack.accounts
import cgi
import web.sessions
import sys
import os
import web.template
import importlib
importlib.reload( web.sessions )

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
	# Mensagem de erro elegante com Alert do Bootstrap
	alert_html = ""
	if message:
		alert_html = f"""
		<div class="alert alert-danger d-flex align-items-center mb-4" role="alert">
			<div>{message}</div>
		</div>
		"""

	# Formulário estilizado com Bootstrap 5
	# Formulário estilizado sem card duplicado
	content = f"""
	<div class="py-3 px-2 mx-auto" style="max-width: 420px;">
		<div class="text-center mb-4">
			<h3 class="fw-bold mb-1" style="color: #004026;">Welcome to Wolfpack</h3>
			<p class="text-muted small">Please enter your administrative credentials.</p>
		</div>
		
		{alert_html}

		<form action="login.py" method="POST">
			<div class="mb-3 text-start">
				<label for="username" class="form-label fw-semibold text-secondary small">Username</label>
				<input type="text" class="form-control" id="username" name="username" required autofocus />
			</div>
			<div class="mb-4 text-start">
				<label for="password" class="form-label fw-semibold text-secondary small">Password</label>
				<input type="password" class="form-control" id="password" name="password" required />
			</div>
			<button type="submit" class="btn btn-wolfpack w-100 py-2 fw-semibold shadow-sm">
				Login
			</button>
		</form>
	</div>
	"""
	
	try:
		# Se sua pasta atual de testes for web_new, aponte para 'web_new/template.html' ou mantenha 'web/template.html'
		file = open( 'web_new/template.html' if os.path.exists('web_new/template.html') else 'web/template.html', 'r', encoding='utf-8' )
		template = file.read()
		file.close()
	except:
		template = "<b>ERROR:</b> Couldn't read template.html."
	
	template = template.replace( '%NAVIGATION%', '&gt; <a href="login.py">Login</a>' )
	template = template.replace( '%USERNAME%', '' )
	template = template.replace( '%NAVBAR%', '' )
	template = template.replace( '%CONTENT_COL%', 'col-12 col-md-8 col-lg-6' )
	template = template.replace( '%CONTENT%', content )
	
	print("Content-type: text/html; charset=utf-8\n\n")
	print(template)
else:
	# Create a session
	session = web.sessions.start_session( username )

	success_content = """
	<div class="alert alert-success d-flex align-items-center" role="alert">
		<div>
			<strong>Success!</strong> You have been logged in. Redirecting...<br>
			Click <a href="main.py?session=%(session)s" class="alert-link">here</a> if you are not automatically redirected.
		</div>
	</div>
	<meta http-equiv="refresh" content="0;URL=main.py?session=%(session)s" />
	""" % {'session': session}

	web.template.output( '', '', 0, success_content, '' )