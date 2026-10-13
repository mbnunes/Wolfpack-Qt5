# Displays the Console log!

import web.sessions
import web.template
import wolfpack.console
import _wolfpack.console
from wolfpack.consts import *
import sys, cgi, re, string

# Gera botões estilizados com Bootstrap 5
def pretty_button(action_val, caption, btn_class="btn-outline-secondary", confirm_msg=""):
	onclick_attr = f'onsubmit="return confirm(\'{confirm_msg}\');"' if confirm_msg else ''
	return f"""
	<form action="console.py" method="get" class="d-inline" {onclick_attr}>
		<input type="hidden" name="session" value="{session_id}" />
		<input type="hidden" name="action" value="{action_val}" />
		<button type="submit" class="btn {btn_class} btn-sm fw-semibold shadow-sm">
			{caption}
		</button>
	</form>
	"""

form = cgi.FieldStorage()
session_id = form.getvalue('session', '')

web.sessions.check_timeouts()
session = web.sessions.get_session(session_id)

if session is None:
	web.sessions.display_timeout()
	sys.exit()

action = 0
try:
	action = int(form.getvalue('action', '0'))
except:
	pass

username = session['username']

if action == 1:
	print("Content-type: text/html; charset=utf-8\n\n")
	print("""
	<div style="font-family: sans-serif; padding: 2rem; text-align: center;">
		<h3>Recarregando scripts do servidor...</h3>
		<p>Será necessário realizar o login novamente após o término.</p>
		<p><a href="login.py">Voltar ao Login</a></p>
	</div>
	""")
	wolfpack.queueaction(RELOAD_SCRIPTS)
	sys.exit()
elif action == 2:
	print("Content-type: text/html; charset=utf-8\n\n")
	print("""
	<div style="font-family: sans-serif; padding: 2rem; text-align: center;">
		<h3>Recarregando scripts Python...</h3>
		<p>Será necessário realizar o login novamente após o término.</p>
		<p><a href="login.py">Voltar ao Login</a></p>
	</div>
	""")
	wolfpack.queueaction(RELOAD_PYTHON)
	sys.exit()
elif action == 3:
	wolfpack.queueaction(RELOAD_ACCOUNTS)
elif action == 4:
	wolfpack.queueaction(RELOAD_CONFIGURATION)
elif action == 5:
	wolfpack.queueaction(SAVE_WORLD)
elif action == 6:
	print("Content-type: text/html; charset=utf-8\n\n")
	print("""
	<div style="font-family: sans-serif; padding: 2rem; text-align: center; color: #dc3545;">
		<h3>O servidor está sendo desligado.</h3>
		<p>Administração remota encerrada.</p>
	</div>
	""")
	_wolfpack.console.shutdown()
	sys.exit()

# Obtenção e limpeza do buffer de logs
raw_line = wolfpack.console.getbuffer()

# Remove códigos ANSI de escape e caracteres de retrocesso (\b)
raw_line = raw_line.replace("\b", "")
# Regex para remover códigos ANSI de cor/controle
ansi_escape = re.compile(r'\x1B(?:[@-Z\\-_]|\[[0-?]*[ -/]*[@-~])')
clean_line = ansi_escape.sub('', raw_line)
clean_line = clean_line.replace("[1;33m", "").replace("[1;37m", "").replace("[1;32m", "").replace("[0m", "")
clean_line = clean_line.replace("", "")

# Escape de tags HTML no log para evitar injeção
escaped_log = clean_line.replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;")

content = f"""
<!-- Cabeçalho do Console -->
<div class="d-flex flex-wrap justify-content-between align-items-center pb-3 mb-3 border-bottom gap-2">
	<div>
		<h4 class="fw-bold text-dark mb-0">Console do Servidor</h4>
		<p class="text-secondary small mb-0">Saída de terminal do Wolfpack em tempo real.</p>
	</div>
	<div>
		<a href="console.py?session={session_id}" class="btn btn-outline-success btn-sm">
			Atualizar Saída
		</a>
	</div>
</div>

<!-- Janela do Terminal -->
<div class="card bg-dark border-0 shadow-sm mb-3">
	<div class="card-header bg-black text-light-50 py-2 d-flex justify-content-between align-items-center small border-secondary">
		<span class="font-monospace text-secondary">stdout / stderr</span>
		<span class="badge bg-secondary-subtle text-dark border">Wolfpack Core</span>
	</div>
	<div class="card-body p-0">
		<pre id="terminal-output" class="text-light font-monospace m-0 p-3 small overflow-auto" 
			 style="height: 480px; background-color: #0d1117; color: #c9d1d9; white-space: pre-wrap; word-break: break-all;">{escaped_log}</pre>
	</div>
</div>

<!-- Barra de Ações Operacionais -->
<div class="card border border-light-subtle shadow-sm">
	<div class="card-header bg-white py-2">
		<span class="fw-semibold text-secondary small text-uppercase">Ações do Servidor</span>
	</div>
	<div class="card-body">
		<div class="d-flex flex-wrap gap-2 align-items-center">
			{pretty_button(1, "Reload Scripts", "btn-outline-primary")}
			{pretty_button(2, "Reload Python", "btn-outline-primary")}
			{pretty_button(3, "Reload Accounts", "btn-outline-secondary")}
			{pretty_button(4, "Reload Configuration", "btn-outline-secondary")}
			{pretty_button(5, "Save World", "btn-success")}
			<div class="ms-auto">
				{pretty_button(6, "Shutdown", "btn-outline-danger", "ATENÇÃO: Você tem certeza de que deseja DESLIGAR o servidor? O serviço será encerrado imediatamente.")}
			</div>
		</div>
	</div>
</div>

<script>
	// Faz o scroll automático até a última linha do log ao carregar
	window.addEventListener('DOMContentLoaded', () => {{
		const terminal = document.getElementById('terminal-output');
		if (terminal) {{
			terminal.scrollTop = terminal.scrollHeight;
		}}
	}});
</script>
"""

web.template.output( 	
	f'&gt; <a href="console.py?session={session_id}" class="text-decoration-none text-success fw-semibold">Console</a>', 
	username, 
	1, 
	content,
	session_id
)