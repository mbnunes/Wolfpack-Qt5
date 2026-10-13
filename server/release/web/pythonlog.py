# Displays the Python log

import web.sessions
import web.template
import cgi
import sys
import os

form = cgi.FieldStorage()
session_id = form.getvalue( 'session', '' )

web.sessions.check_timeouts()
session = web.sessions.get_session( session_id )

if session is None:
	web.sessions.display_timeout()
	sys.exit()

username = session[ 'username' ]

# Leitura segura do arquivo de log
raw_log = ""
log_exists = False
log_paths = ['python.log', '../python.log', 'server/python.log']

for path in log_paths:
	if os.path.exists(path):
		try:
			with open(path, 'r', encoding='utf-8', errors='replace') as f:
				raw_log = f.read()
				log_exists = True
				break
		except Exception as e:
			raw_log = f"Erro ao ler log: {e}"
			log_exists = True
			break

if not log_exists and not raw_log:
	raw_log = "Logfile python.log não encontrado ou nenhum registro gerado até o momento."

# Sanitização para evitar injeção de tags acidentais
escaped_log = raw_log.replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;")

content = f"""
<!-- Cabeçalho do Python Log -->
<div class="d-flex flex-wrap justify-content-between align-items-center pb-3 mb-3 border-bottom gap-2">
	<div>
		<h4 class="fw-bold text-dark mb-0">Logs de Scripts Python</h4>
		<p class="text-secondary small mb-0">Rastreamento de execução, carregamento de módulos e exceções do interpretador Python.</p>
	</div>
	<div class="d-flex gap-2">
		<a href="pythonlog.py?session={session_id}" class="btn btn-outline-success btn-sm">
			Atualizar Log
		</a>
	</div>
</div>

<!-- Janela de Terminal de Logs -->
<div class="card bg-dark border-0 shadow-sm">
	<div class="card-header bg-black text-light-50 py-2 d-flex justify-content-between align-items-center small border-secondary">
		<span class="font-monospace text-secondary">python.log</span>
		<span class="badge bg-secondary-subtle text-dark border">Python Runtime</span>
	</div>
	<div class="card-body p-0">
		<pre id="python-log-output" class="text-light font-monospace m-0 p-3 small overflow-auto" 
			 style="height: 520px; background-color: #0d1117; color: #c9d1d9; white-space: pre-wrap; word-break: break-all;">{escaped_log}</pre>
	</div>
</div>

<script>
	// Rola automaticamente para o fim do log
	window.addEventListener('DOMContentLoaded', () => {{
		const terminal = document.getElementById('python-log-output');
		if (terminal) {{
			terminal.scrollTop = terminal.scrollHeight;
		}}
	}});
</script>
"""

web.template.output( 	
	f'&gt; <a href="pythonlog.py?session={session_id}" class="text-decoration-none text-success fw-semibold">Python Log</a>', 
	username, 
	1, 
	content,
	session_id
)