#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: DarkStorm                      #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification:                         #
#################################################################

# Displays all Accounts

import web.sessions
import web.template
import cgi
import sys
import wolfpack.accounts
import re
from urllib.parse import quote

validletters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789._-"
letters = "#abcdefghijklmnopqrstuvwxyz"
message = None
is_error = False

form = cgi.FieldStorage()
session_id = form.getvalue( 'session', '' )
letter = form.getvalue( 'letter', '#' ).lower()

web.sessions.check_timeouts()
session = web.sessions.get_session( session_id )

if session == None:
	web.sessions.display_timeout()
	sys.exit()

username = session[ 'username' ]
user_acc = wolfpack.accounts.find(username)
if not user_acc or not user_acc.authorized('RemoteAccess', 'login'):
	content = f"""
	<div class="alert alert-danger border shadow-sm my-4">
		<h5 class="alert-heading fw-bold">&#128683; Acesso Negado</h5>
		<p class="mb-0">Esta área de gerenciamento de contas é restrita a administradores do servidor.</p>
		<div class="mt-3">
			<a href="main.py?session={session_id}&view=player" class="btn btn-outline-danger btn-sm">&larr; Voltar ao Meu Painel</a>
		</div>
	</div>
	"""
	web.template.output("Acesso Negado", username, 1, content, session_id)
	sys.exit()

# Try to add an account
if 'add' in form:
	name = form.getvalue( 'add', '' ).strip()
	password1 = form.getvalue( 'password1', '' )
	password2 = form.getvalue( 'password2', '' )
	acl = form.getvalue( 'acl', '' )

	record = wolfpack.accounts.find( name )
	if record:
		message = "A conta '%s' já existe." % name
		is_error = True
	elif ( len( name ) < 1 ) or ( len( name ) > 30 ):
		message = "O nome da conta '%s' é muito curto (mín. 1 caractere) ou muito longo (máx. 30 caracteres)." % name
		is_error = True
	elif ( len( password1 ) < 1 ) or ( len( password1 ) > 30 ):
		message = "A senha informada é muito curta (mín. 1 caractere) ou muito longa (máx. 30 caracteres)."
		is_error = True
	elif( password1 != password2 ):
		message = "As senhas não coincidem. Por favor digite a senha desejada duas vezes."
		is_error = True
	else:
		for l in name:
			if not l in validletters:
				message = "O nome da conta '%s' contém caracteres inválidos (%s)." % ( name, l )
				is_error = True
				break

		if not message:
			record = wolfpack.accounts.add( name, password1 )
			if record:
				if acl:
					record.acl = acl
				wolfpack.accounts.save()
				message = "A conta '%s' foi criada com sucesso." % name
			else:
				message = "Não foi possível criar a conta '%s'." % name
				is_error = True

# Try to delete an account
if 'delete' in form:
	del_name = form.getvalue( 'delete', '' ).strip()
	record = wolfpack.accounts.find( del_name )

	if del_name.lower() == username.lower():
		message = "Você não pode excluir sua própria conta."
		is_error = True
	elif not record:
		message = "A conta não existe."
		is_error = True
	elif record.inuse:
		message = "A conta está atualmente em uso no jogo."
		is_error = True
	else:
		record.delete()
		wolfpack.accounts.save()
		message = "A conta foi excluída com sucesso."

# Alertas
alert_html = ""
if message:
	alert_type = "danger" if is_error else "success"
	alert_html = f"""
	<div class="alert alert-{alert_type} alert-dismissible fade show" role="alert">
		{message}
	</div>
	"""

# Paginação Alfabética A-Z
pagination_items = []
all_active = 'active' if letter == 'all' else ''
pagination_items.append(f'<li class="page-item {all_active}"><a class="page-link" href="accounts.py?session={session_id}&letter=all">All</a></li>')

for l in letters:
	active_cls = 'active' if l == letter else ''
	pagination_items.append(f'<li class="page-item {active_cls}"><a class="page-link" href="accounts.py?session={session_id}&letter={quote(l.lower())}">{l.upper()}</a></li>')

alpha_nav = f"""
<nav aria-label="Alphabetical Filter" class="mb-4">
	<ul class="pagination pagination-sm flex-wrap mb-0 shadow-sm">
		{''.join(pagination_items)}
	</ul>
</nav>
"""

# Lista de contas filtrada
accounts = []
for account in wolfpack.accounts.list():
	if len( account ) > 0:
		first = account.lower()[0]

		if letter == 'all':
			accounts.append( account )
		elif letter == '#' and first not in letters[1:]:
			accounts.append( account )
		elif first == letter:
			accounts.append( account )

accounts.sort( key=lambda a: a.lower() )

# Linhas da tabela de contas
table_rows = []
for account in accounts:
	record = wolfpack.accounts.find( account )
	if record is None:
		continue

	is_blocked = bool(record.flags & 0x00000001)
	blocked_badge = '<span class="badge bg-danger-subtle text-danger border border-danger">Blocked</span>' if is_blocked else '<span class="badge bg-success-subtle text-success border border-success">Active</span>'
	until_info = record.blockuntil if record.blockuntil else '<span class="text-muted">-</span>'
	acl_badge = f'<span class="badge bg-secondary-subtle text-dark border">{record.acl}</span>'

	table_rows.append(f"""
	<tr>
		<td class="ps-3 fw-bold">
			<a href="account.py?session={session_id}&username={quote(account)}&letter={quote(letter)}" class="text-decoration-none text-success">
				{account}
			</a>
		</td>
		<td>{acl_badge}</td>
		<td>{blocked_badge}</td>
		<td class="text-secondary small">{until_info}</td>
		<td class="text-end pe-3">
			<a href="account.py?session={session_id}&username={quote(account)}&letter={quote(letter)}" class="btn btn-outline-secondary btn-sm py-0">Editar</a>
		</td>
	</tr>
	""")

if not table_rows:
	table_rows.append('<tr><td colspan="5" class="text-center py-4 text-muted">No accounts found with this filter.</td></tr>')

# Opções de ACL para o select
acl_options = "".join([f'<option value="{a}">{a}</option>' for a in wolfpack.accounts.acls()])

content = f"""
<div class="d-flex justify-content-between align-items-center pb-2 mb-3 border-bottom">
	<div>
		<h4 class="fw-bold text-dark mb-0">Gerenciamento de Contas</h4>
		<p class="text-secondary small mb-0">Visualize, filtre, adicione e edite contas do servidor.</p>
	</div>
</div>

{alert_html}
{alpha_nav}

<!-- Tabela de Contas -->
<div class="card border border-light-subtle shadow-sm mb-4">
	<div class="card-header bg-white py-3 d-flex justify-content-between align-items-center">
		<h6 class="fw-bold text-dark mb-0">Contas Registradas ({len(accounts)})</h6>
	</div>
	<div class="table-responsive">
		<table class="table table-hover table-striped align-middle mb-0">
			<thead class="table-light small text-uppercase text-secondary">
				<tr>
					<th scope="col" class="ps-3">Username</th>
					<th scope="col">ACL</th>
					<th scope="col">Status</th>
					<th scope="col">Blocked Until</th>
					<th scope="col" class="text-end pe-3">Ações</th>
				</tr>
			</thead>
			<tbody>
				{''.join(table_rows)}
			</tbody>
		</table>
	</div>
</div>

<!-- Card Formulário: Nova Conta -->
<div class="card border border-light-subtle shadow-sm">
	<div class="card-header bg-white py-3">
		<h6 class="fw-bold text-dark mb-0">Criar Nova Conta</h6>
	</div>
	<div class="card-body">
		<form action="accounts.py" method="POST" class="row g-3">
			<input type="hidden" name="session" value="{session_id}" />
			<input type="hidden" name="letter" value="{quote(letter)}" />

			<div class="col-12 col-md-4">
				<label class="form-label small fw-semibold text-secondary">Username</label>
				<input type="text" class="form-control form-control-sm" name="add" required />
			</div>

			<div class="col-12 col-md-4">
				<label class="form-label small fw-semibold text-secondary">Password</label>
				<input type="password" class="form-control form-control-sm" name="password1" required />
			</div>

			<div class="col-12 col-md-4">
				<label class="form-label small fw-semibold text-secondary">Confirm Password</label>
				<input type="password" class="form-control form-control-sm" name="password2" required />
			</div>

			<div class="col-12 col-md-4">
				<label class="form-label small fw-semibold text-secondary">Nível de Acesso (ACL)</label>
				<select name="acl" class="form-select form-select-sm">
					{acl_options}
				</select>
			</div>

			<div class="col-12 d-flex align-items-end">
				<button type="submit" class="btn text-white btn-sm px-4 fw-semibold" style="background-color: #004026;">
					+ Adicionar Conta
				</button>
			</div>
		</form>
	</div>
</div>
"""

web.template.output( 	
	f'&gt; <a href="accounts.py?session={session_id}&letter={quote(letter)}" class="text-decoration-none text-success fw-semibold">Accounts</a>', 
	username, 
	1, 
	content,
	session_id
)