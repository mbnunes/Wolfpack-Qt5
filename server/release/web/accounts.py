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
from urllib import quote

validletters = "abcdefghijklmnopqrstuvwxyz0123456789._"
letters = "#abcdefghijklmnopqrstuvwxyz"
message = None

form = cgi.FieldStorage()
session_id = form.getvalue( 'session', '' )
letter = form.getvalue( 'letter', '#' )

web.sessions.check_timeouts()
session = web.sessions.get_session( session_id )

if session == None:
	web.sessions.display_timeout()
	sys.exit()

username = session[ 'username' ]

# Try to add an account
if form.has_key( 'add' ):
	name = form.getvalue( 'add', '' )
	password1 = form.getvalue( 'password1', '' )
	password2 = form.getvalue( 'password2', '' )
	acl = form.getvalue( 'acl', '' )

	record = wolfpack.accounts.find( name )
	if record:
		message = "The account '%s' already exists." % name
	elif ( len( name ) < 1 ) or ( len( name ) > 30 ):
		message = "The account name '%s' is too short (min. 1 character) or too long (max. 30 characters)." % name
	elif ( len( password1 ) < 1 ) or ( len( password1 ) > 30 ):
		message = "The given password is too short (min 1. character) or too long (max. 30 characters)."
	elif( password1 != password2 ):
		message = "The passwords don't match. Please enter the desired password twice."
	else:
		for l in name:
			if not l in validletters:
				message = "The account name '%s' contains invalid characters (%s)." % ( name, l )
				break

		if not message:
			record = wolfpack.accounts.add( name, password1 )
			record.acl = acl
			message = "The account has been added."

# Try to delete an account
if form.has_key( 'delete' ):
	record = wolfpack.accounts.find( form.getvalue( 'delete', '' ) )

	if form[ 'delete' ] == username:
		message = "You can't delete your own account."
	elif not record:
		message = "The account does not exist."
	elif record.inuse:
		message = "The account is currently in use."
	else:
		record.delete()
		wolfpack.accounts.save()
		message = "The account has been deleted."	

content = """
<p>
<h1>Accounts</h1>
"""

if message != None:
	content += message + "<br /><br />"

if letter != 'all':
	content += '<a href="accounts.py?session=%s&letter=all">All</a> | ' % session_id
else:
	content += 'All | '

for l in letters:
	if l != letter:
		# Note: the # character is interpreted as the end of the url, thats ok because we're using a default value when receiving the parameter
		content += '<a href="accounts.py?session=%(session)s&letter=%(linkletter)s">%(letter)s</a> ' % { 'linkletter': quote( l.lower() ), 'letter': l.upper(), 'session': session_id }
	else:
		content += '%s ' % l.upper()

content += """
</p>
"""

accounts = []

# Display a filtered list of accounts
for account in wolfpack.accounts.list():
	if len( account ) > 0:
		first = account.lower()[0]

		if letter == 'all':
			accounts.append( account )
		elif letter == '#' and first not in letters[1:]:
			accounts.append( account )
		elif first == letter:
			accounts.append( account )

# Sort the list
accounts.sort( lambda a, b: cmp( a.lower(), b.lower() ) )

content += '<table width="350" border="0" cellspacing="0" cellpadding="3">'
content += '<tr><td><b>Username</b></td><td>ACL</td><td>blocked</td><td>until</td></tr>'
content += '<tr><td colspan="4" height="1"><img src="line_green.png" height="1" width="340"/></td></tr>'

for account in accounts:
	record = wolfpack.accounts.find( account )

	if record == None:
		continue

	blocked = 'false'
	if record.flags & 0x00000001: blocked = 'true'
	content += '<tr>'
	content += '<td><a href="account.py?session=%(session)s&username=%(username)s&letter=%(letter)s">%(account)s</a></td>' % { 'username': quote( account ), 'account': account, 'session': session_id, 'letter': quote( letter ) }
	content += '<td>%(acl)s</td>' % { 'acl': record.acl }
	content += '<td>%s</td>' % blocked
	content += '<td>%s</td>' % record.blockuntil
	content += '</tr>'

content += '</table><br /><br />'

# Table for New Accounts
content += '<table width="350" border="0" cellspacing="0" cellpadding="3">'
content += '<form action="accounts.py" method="GET">'
content += '<tr><td colspan="2"><b>Add Account</b></td></tr>'
content += '<tr><td colspan="2" height="1"><img src="line_green.png" height="1" width="340"/></td></tr>'

# Username
content += '<tr><td>Username</td><td><input type="text" name="add" /></td></tr>'

# Change Password
content += '<tr><td>Password</td><td><input type="password" name="password1" /></td></tr>'
content += '<tr><td>&nbsp;</td><td><input type="password" name="password2" /></td></tr>'

# ACL
content += '<tr><td>ACL</td><td><select name="acl">'

for acl in wolfpack.accounts.acls():
		content += '<option>%s</option>' % acl

content += '</select></td></tr>'

# Submit Button
content += '<tr><td colspan="2"><br />'
content += '<input type="hidden" name="session" value="%s" />' % session_id
content += '<input type="submit" value="Add" /></td></tr>'

content += '</form>'
content += '</table>'

web.template.output( 	'&gt; <a href="accounts.py?session=%(session)s&letter=%(letter)s" class="header">Accounts</a>' % { 'session': session_id, 'letter': quote( letter ) }, 
						'<i>Logged in as:</i> %(username)s<br /><a href="logout.py?session=%(session)s" class="header">Logout</a>' % { 'username': username, 'session': session_id }, 
						1, 
						content,
						session_id
					)

