#######################################################
#
# Wrapper for wolfpack.server
#
#######################################################

import _wolfpack

clients = _wolfpack.clients()

def shutdown():
	_wolfpack.server_shutdown()
	
def save():
	_wolfpack.server_save()
