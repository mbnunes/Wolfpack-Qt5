
# All these imports are available to the python scriptlets as well
# Keep that in mind

import wolfpack

def evalcommand(socket, command, arguments):
	script = "import wolfpack\nfrom wolfpack.consts import *\n%s\n\n" % arguments
	try:
		code = compile(script, '<SOCKET>', 'exec')
		eval(code, globals(), locals())
	except Exception, e:
		socket.sysmessage('Python Error: ' + str(e))
		return

	socket.sysmessage('Executed the python scriptlet.')

def onLoad():
	registercommand('eval', evalcommand)
