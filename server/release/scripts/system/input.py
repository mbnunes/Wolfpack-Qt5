"""
	This file is used to request the next speech input from the client to be 
	sent back in a special packet. This can be used for very simple renaming
	of runes, runebooks, etc. 
	
	The user can also cancel the input by using his Escape key. If the user
	successfully enters a text, the onTextInput event will be called for the
	originating item.
	
	If the user cancels the request, onTextInputCancel will be called.
"""

import wolfpack

#
# Send an input line request to the client
#
def request(player, item, id):
	packet = wolfpack.packet(0x9a, 16)
	packet.setshort(1, 16)
	packet.setint(3, item.serial)
	packet.setint(7, id)
	packet.setint(11, 0) # Request/Reply
	packet.setbyte(15, 0)
	packet.send(player.socket)

def callEvent(item, event, args):
	scripts = item.scripts + item.basescripts.split(',') # Build the scriptlist
	for script in scripts:
		if not wolfpack.hasnamedevent(script, event):
			continue
	
		result = wolfpack.callnamedevent(script, event, args)
		
		if result:
			return True

	return False	

#
# Procses an input line response
#
def responseHandler(socket, packet):
	if packet.size < 16:
		return True
	
	item = wolfpack.finditem(packet.getint(3))
	
	if not item:
		socket.sysmessage(tr('Unexpected input response.'))
		return True # Item went out of scope
					
	if packet.getint(11) != 1:
		args = (socket.player, item, packet.getint(7))
		callEvent(item, 'onTextInputCancel', args)
		return True # Cancelled

	# Last byte has to be nullbyte
	if packet.getbyte(packet.size - 1) != 0:
		socket.log(LOG_ERROR, "Received invalid input response packet. String is not null terminated.")
		return True
		
	text = packet.getascii(15, packet.size - 15)
	
	# Build argument string for onTextInput event
	args = (socket.player, item, packet.getint(7), text)
	callEvent(item, 'onTextInput', args)

	return True

# Register the packet hook for the response
def onLoad():
	wolfpack.registerpackethook(0x9a, responseHandler)
