# Detailed information about Accounts

import web.sessions
import web.template
import cgi
import sys
import wolfpack.accounts
from urllib.parse import quote

letters = "#abcdefghijklmnopqrstuvwxyz"

form = cgi.FieldStorage()
session_id = form.getvalue( 'session', '' )
letter = form.getvalue( 'letter', '#' )
account = form.getvalue( 'username', '' )

save = 0
try:
	save = int( form.getvalue( 'save', '0' ) )
except:
	pass
	
message = None
is_error = False

web.sessions.check_timeouts()
session = web.sessions.get_session( session_id )

if session == None:
	web.sessions.display_timeout()
	sys.exit()

username = session[ 'username' ]

record = wolfpack.accounts.find( account )

if not record:
	print("Content-type: text/html; charset=utf-8\n\n")
	print("<div class='alert alert-danger m-4'>Invalid account.</div>")
	sys.exit()

# Check if we should change any information
if save == 1:
	password1 = form.getvalue( 'password1', '' )
	password2 = form.getvalue( 'password2', '' )

	# Change Password
	if password1 != '':
		if password1 != password2:
			message = "The passwords don't match!"
			is_error = True
		else:
			record.password = password1

	# Only continue if we didn't fail already
	if not is_error:
		record.acl = form.getvalue( 'acl', 'player' )
		if form.getvalue( 'block', 'false' ) == 'true': 
			record.block()
		else:
			record.unblock()

		if form.getvalue( 'blockuntil', '' ) != '':
			record.blockuntil = form.getvalue( 'blockuntil', '' )

		if form.getvalue( 'email', '' ) != '':
			record.email = form.getvalue( 'email', '' )

		message = "The account details have been updated successfully."

alert_html = ""
if message:
	alert_type = "danger" if is_error else "success"
	alert_html = f"""
	<div class="alert alert-{alert_type} alert-dismissible fade show" role="alert">
		{message}
	</div>
	"""

# Opções do Select de ACL
acl_options = []
for acl_item in wolfpack.accounts.acls():
	selected = 'selected' if acl_item == record.acl else ''
	acl_options.append(f'<option value="{acl_item}" {selected}>{acl_item}</option>')

# Status Blocked
is_blocked = bool(record.flags & 0x00000001)
sel_block_false = '' if is_blocked else 'selected'
sel_block_true = 'selected' if is_blocked else ''

# Personagens vinculados à conta
chars_html = ""
if len(record.characters) > 0:
	char_items = []
	for char in record.characters:
		char_items.append(f"""
		<li class="list-group-item d-flex justify-content-between align-items-center">
			<div>
				<strong class="text-dark">{char.name}</strong>
			</div>
			<span class="badge bg-secondary-subtle text-dark border font-monospace">Serial: 0x{char.serial:x}</span>
		</li>
		""")

	chars_html = f"""
	<div class="card border border-light-subtle shadow-sm mb-4">
		<div class="card-header bg-white py-3">
			<h6 class="fw-bold text-dark mb-0">Personagens Vinculados ({len(record.characters)})</h6>
		</div>
		<ul class="list-group list-group-flush">
			{''.join(char_items)}
		</ul>
	</div>
	"""

# Botão Deletar (proibido para a própria conta)
delete_btn = ""
if account != username:
	delete_btn = f"""
	<a href="accounts.py?session={session_id}&letter={quote(letter)}&delete={quote(account)}" 
	   class="btn btn-outline-danger btn-sm"
	   onclick="return confirm('Tem certeza absoluta de que deseja excluir a conta {account}? Esta ação não pode ser desfeita.');">
		Excluir Conta
	</a>
	"""

content = f"""
<div class="d-flex justify-content-between align-items-center pb-2 mb-3 border-bottom">
	<div>
		<h4 class="fw-bold text-dark mb-0">Detalhes da Conta: <span class="text-success">{account}</span></h4>
		<p class="text-secondary small mb-0">Último Login: {record.lastlogin}</p>
	</div>
	<div>
		<a href="accounts.py?session={session_id}&letter={quote(letter)}" class="btn btn-outline-secondary btn-sm">
			&larr; Voltar para Contas
		</a>
	</div>
</div>

{alert_html}

<!-- Formulário de Informações Gerais -->
<div class="card border border-light-subtle shadow-sm mb-4">
	<div class="card-header bg-white py-3">
		<h6 class="fw-bold text-dark mb-0">Informações Gerais</h6>
	</div>
	<div class="card-body">
		<form action="account.py" method="POST" class="row g-3">
			<input type="hidden" name="save" value="1" />
			<input type="hidden" name="session" value="{session_id}" />
			<input type="hidden" name="username" value="{quote(account)}" />
			<input type="hidden" name="letter" value="{quote(letter)}" />

			<div class="col-12 col-md-6">
				<label class="form-label small fw-semibold text-secondary">Username</label>
				<input type="text" class="form-control form-control-sm bg-light" value="{account}" readonly />
			</div>

			<div class="col-12 col-md-6">
				<label class="form-label small fw-semibold text-secondary">E-Mail</label>
				<input type="email" class="form-control form-control-sm" name="email" value="{record.email}" />
			</div>

			<div class="col-12 col-md-6">
				<label class="form-label small fw-semibold text-secondary">Nova Senha (opcional)</label>
				<input type="password" class="form-control form-control-sm" name="password1" placeholder="Deixe em branco para manter" />
			</div>

			<div class="col-12 col-md-6">
				<label class="form-label small fw-semibold text-secondary">Confirmar Nova Senha</label>
				<input type="password" class="form-control form-control-sm" name="password2" placeholder="Repita a nova senha" />
			</div>

			<div class="col-12 col-md-4">
				<label class="form-label small fw-semibold text-secondary">Nível de Acesso (ACL)</label>
				<select name="acl" class="form-select form-select-sm">
					{''.join(acl_options)}
				</select>
			</div>

			<div class="col-12 col-md-4">
				<label class="form-label small fw-semibold text-secondary">Bloqueio de Conta</label>
				<select name="block" class="form-select form-select-sm">
					<option value="false" {sel_block_false}>Ativa (False)</option>
					<option value="true" {sel_block_true}>Bloqueada (True)</option>
				</select>
			</div>

			<div class="col-12 col-md-4">
				<label class="form-label small fw-semibold text-secondary">Bloqueado Até (Data/Timestamp)</label>
				<input type="text" class="form-control form-control-sm" name="blockuntil" value="{record.blockuntil}" />
			</div>

			<div class="col-12 d-flex justify-content-between align-items-center mt-4 pt-3 border-top">
				<button type="submit" class="btn text-white btn-sm px-4 fw-semibold" style="background-color: #004026;">
					Salvar Alterações
				</button>
				{delete_btn}
			</div>
		</form>
	</div>
</div>

{chars_html}
"""

user_badge = f"""
<div class="d-flex justify-content-between align-items-center w-100">
	<div>
		<span class="text-muted">Conectado como:</span> 
		<span class="badge bg-secondary-subtle text-dark border border-secondary-subtle fw-semibold px-2 py-1">{username}</span>
	</div>
	<div>
		<a href="logout.py?session={session_id}" class="text-danger text-decoration-none fw-semibold small">Sair (Logout)</a>
	</div>
</div>
"""

web.template.output( 	
	f'&gt; <a href="accounts.py?session={session_id}&letter={quote(letter)}" class="text-decoration-none text-success">Accounts</a> &gt; <a href="account.py?username={quote(account)}&letter={quote(letter)}&session={session_id}" class="text-decoration-none text-success fw-semibold">Account ({account})</a>', 
	user_badge, 
	1, 
	content,
	session_id
)