
# Detailed information about Accounts

import web.sessions
import web.template
import cgi
import sys
import wolfpack.accounts
from urllib import quote

letters = "#abcdefghijklmnopqrstuvwxyz"

form = cgi.FieldStorage()
session_id = form.getvalue( 'session', '' )
letter = form.getvalue( 'letter', '#' )
account = form.getvalue( 'username', '' )
try:
	save = int( form.getvalue( 'save', '0' ) )
except:
	pass
message = None

web.sessions.check_timeouts()
session = web.sessions.get_session( session_id )

if session == None:
	web.sessions.display_timeout()
	sys.exit()

username = session[ 'username' ]

"""
	Get the record for the account we want to have details about.
"""
record = wolfpack.accounts.find( account )

if not record:
	print "Content-type: text/html\n\n"
	print "Invalid account"
	sys.exit()

# Check if we should change any information
if save == 1:
	password1 = form.getvalue( 'password1', '' )
	password2 = form.getvalue( 'password2', '' )

	# Change Password
	if password1 != '':
		if password1 != password2:
			message = "The passwords don't match!"
		else:
			record.password = password1

	# Only continue if we didn't fail already
	if not message:
		record.acl = form.getvalue( 'acl', 'player' ) # If error = default to playor ;)
		if form.getvalue( 'block', 'false' ) == 'true': 
			record.block()
		else:
			record.unblock()
		
		if form.getvalue( 'blockuntil', '' ) != '':
			record.blockuntil = form.getvalue( 'blockuntil', '' )
			
		message = "The record has been updated successfully."

content = """
<p>
<h1>Account Information</h1>
"""

if message != None:
	content += message + '<br /><br />'

content += '<table width="350" border="0" cellspacing="0" cellpadding="3">'
content += '<form action="account.py" method="POST">'
content += '<tr><td colspan="2"><b>General Information</b></td></tr>'
content += '<tr><td colspan="2" height="1"><img src="line_green.png" height="1" width="340" /></td></tr>'

# Username
content += '<tr><td>Username</td><td>%s</td></tr>' % account

# Change Password
content += '<tr><td>Password</td><td><input type="password" name="password1" /></td></tr>'
content += '<tr><td>&nbsp;</td><td><input type="password" name="password2" /></td></tr>'

# ACL
content += '<tr><td>ACL</td><td><select name="acl">'

for acl in wolfpack.accounts.acls():
	if acl != record.acl:
		content += '<option>%s</option>' % acl
	else:
		content += '<option selected>%s</option>' % acl


content += '</select></td></tr>'

content += '<tr><td>Last Login</td><td>%s</td></tr>' % record.lastlogin

select0 = 'selected'
select1 = ''

if record.flags & 0x00000001:
	select0 = ''
	select1 = 'selected'
	
content += '<tr><td>Blocked</td><td><select name="block"><option '+select0+'>false</option><option '+select1+'>true</option></select></td></tr>'
content += '<tr><td>until</td><td><input type="text" name="blockuntil" value="%s"</td></tr>' % record.blockuntil

# Submit Button
content += '<tr><td colspan="2"><br />'
content += '<input type="hidden" name="save" value="1" />'
content += '<input type="hidden" name="session" value="%s" />' % session_id
content += '<input type="hidden" name="username" value="%s" />' % quote( account )
content += '<input type="submit" value="Save" /></td></tr>'

content += "</form></table><br /><br />\n"

# Character list 
content += '<table width="350" border="0" cellspacing="0" cellpadding="3">'
content += '<tr><td colspan="2"><b>Characters</b></td></tr>'
content += '<tr><td colspan="2" height="1"><img src="line_green.png" height="1" width="340" /></td></tr>'

for char in record.characters:
	content += '<tr><td colspan="2"><b>%s</b> (Serial: 0x%x)</td></tr>' % ( char.name, char.serial )

content += '</table><br/><br/><br/>'


# Back Link
content += '<input type="button" onClick="window.location.href=\'accounts.py?session=%s&letter=%s\';" value="Back" /> ' % ( session_id, quote( letter ) )

# Deleting your own account is not possible!
if account != username:
	content += '<input type="button" onClick="window.location.href=\'accounts.py?session=%s&letter=%s&delete=%s\';" value="Delete Account" /> ' % ( session_id, quote( letter ), quote( account ) )

web.template.output( 	'&gt; <a href="accounts.py?session=%(session)s&letter=%(letter)s" class="header">Accounts</a> &gt; <a href="account.py?username=%(accountq)s&letter=%(letter)s&session=%(session)s" class="header">Account Information (%(account)s)</a>' % { 'session': session_id, 'letter': quote( letter ), 'account': account, 'accountq': quote( account ) }, 
						'<i>Logged in as:</i> %(username)s<br /><a href="logout.py?session=%(session)s" class="header">Logout</a>' % { 'username': username, 'session': session_id }, 
						1, 
						content,
						session_id
					)

