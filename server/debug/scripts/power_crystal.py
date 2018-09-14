from wolfpack import tr

def onUse(char, item):
	if not char.canreach(item, 3):
		char.message(1019045) # I can't reach that.
	else:
		char.socket.sysmessage( tr("This looks like part of a larger contraption.") )
	return True
