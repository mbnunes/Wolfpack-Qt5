
# All these imports are available to the python scriptlets as well
# Keep that in mind

import wolfpack
from wolfpack.consts import *
from wolfpack import *

def evalcommand(socket, command, arguments):
	#script = "import wolfpack\nfrom wolfpack.consts import *\n%s\n\n" % arguments
	script = arguments
	result = None
	try:
		#code = compile(script, '<SOCKET>', 'exec')
		#result = eval(script, globals(), locals())
		result = eval(script)
	except Exception, e:
		socket.sysmessage('Python Error: ' + str(e))
		return

	socket.sysmessage('Executed the python scriptlet.')
	socket.sysmessage('Result: ' + str(result))

def onLoad():
	wolfpack.registercommand('eval', evalcommand)
