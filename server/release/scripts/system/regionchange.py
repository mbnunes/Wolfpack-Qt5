###########################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts
#  ((    _/{  "-;  # Created by: Dreoth
#   )).-' {{ ;'`   # Revised by:
#  ( (  ;._ \\ ctr # Last Modification: Created
###########################################################

import wolfpack
import string

def onChangeRegion( char, oldregion, newregion ):
	socket = char.socket
	
	if not socket:
		return False
	
	"""
		 0. Both messages enabled == Show Messages Leave Old/Enter New *
		 1. Both messages disabled == No Messages *
		 2. New enabled, Old disabled == Show Message Enter New *
		 3. New disabled, Old enabled == Show Message Leave Old *
		 4. New disabled, Old enabled. New is a child of Old == No Message *
		 5. New enabled, Old enabled. New is a child of Old == Show Message Enter New *
		 6. New disabled, Old enabled. New is a parent of Old == No Message *
		 7. New enabled, Old enabled. New is a parent of Old == Show Message Leave Old *
		 8. New disabled, Old disabled. New is a child of Old == No Message *
		 9. New enabled, Old disabled. New is a child of Old == Show Message Enter New *
		10. New disabled, Old disabled. New is a parent of Old. == No Message *
		11. New enabled, Old disabled. New is a parent of Old. == No Message *
	"""
	# Cases 0 5 7
	# Both Enabled
	if not newregion.noentermessage and not oldregion.noentermessage:
		# Case 0
		if newregion.parent == None or oldregion.parent == None or newregion.parent != oldregion or oldregion.parent != newregion:
			areaLeaveMessage( socket, oldregion.name )
			areaEnterMessage( socket, newregion.name )
		# Case 5
		elif oldregion.parent == newregion or oldregion.name in newregion.children:
			areaEnterMessage( socket, newregion.name )
		# Case 7
		elif newregion.parent == oldregion or newregion.name in oldregion.children:
			areaLeaveMessage( socket, oldregion.name )
	# Cases 1 8 10
	# Both Disabled
	elif newregion.noentermessage and oldregion.noentermessage:
		# Case 1
		if newregion.parent == None or oldregion.parent == None or newregion.parent != oldregion or oldregion.parent != newregion:
			pass
		# Case 8
		elif oldregion.parent == newregion or oldregion.name in newregion.children:
			pass
		# Case 10
		elif newregion.parent == oldregion or newregion.name in oldregion.children:
			pass
	# Cases 2 9 11
	# New Enabled, Old Disabled
	elif not newregion.noentermessage and oldregion.noentermessage:
		# Case 2
		if newregion.parent == None or oldregion.parent == None or newregion.parent != oldregion or oldregion.parent != newregion:
			areaEnterMessage( socket, newregion.name )
		# Case 9
		elif oldregion.parent == newregion or oldregion.name in newregion.children:
			pass
		# Case 11
		elif newregion.parent == oldregion or newregion.name in oldregion.children:
			pass
	# Cases 3 4 6
	# New Disabled, Old Enabled
	elif not newregion.noentermessage and oldregion.noentermessage:
		# Case 3
		if newregion.parent == None or oldregion.parent == None or newregion.parent != oldregion or oldregion.parent != newregion:
			areaLeaveMessage( socket, oldregion.name )
		# Case 4
		elif oldregion.parent == newregion or oldregion.name in newregion.children:
			pass
		# Case 6
		elif newregion.parent == oldregion or newregion.name in oldregion.children:
			pass

	"""
		Possible Guard Cases:
		0. Both are unguarded. Show Message Disabled == No Message
		1. Both are unguarded. Show Message Enabled == No Message
		2. New is guarded, Old is not. Show Message Enabled. == Show Message Enter
		3. New is guarded, Old is not. Show Message Disabled. == No Message
		4. New is not guarded, Old is. Show Message Enabled. == Show Message Leave
		5. New is not guarded, Old is. Show Message Disabled == Show Message Leave
		6. Both are guarded. Owners are the same. Show Message Enabled == No Message
		7. Both are guaded. Owners are the same. Show Message Disabled == No Message
		8. Both are guarded. Owners are different. Show Message Disabled == Show Message Leave
		9. Both are guarded. Owners are different. Show Message Enabled == Show Message Leave/Join
	"""
	# Cases 6 7 8 9
	if newregion.guarded and oldregion.guarded:
		# Case 6
		if newregion.guardowner == oldregion.guardowner and not newregion.noguardmessage:
			pass
		# Case 7
		elif newregion.guardowner == oldregion.guardowner and newregion.noguardmessage:
			pass
		# Case 8
		elif newregion.guardowner != oldregion.guardowner and newregion.noguardmessage:
			guardLeaveMessage( socket, oldregion.guardowner )
		# Case 9
		elif newregion.guardowner != oldregion.guardowner and not newregion.noguardmessage:
			guardLeaveMessage( socket, oldregion.guardowner )
			guardEnterMessage( socket, newregion.guardowner )
	# Cases 2 3
	elif newregion.guarded and not oldregion.guarded:
		# Case 2
		if not newregion.noguardmessage:
			guardEnterMessage( socket, newregion.guardowner )
		# Case 3
		else:
			pass
	# Cases 4 5
	elif not newregion.guarded and oldregion.guarded:
		# Case 4 and 5
		guardLeaveMessage( socket, oldregion.guardowner )
	# Cases 0 1
	elif not newregion.guarded and not oldregion.guarded:
		pass

	return True

# Region Enter Message
def areaEnterMessage( socket, name ):
	name = name.strip()
	if name == None or len( name ) == 0 or name == "":
		return True
	else:
		socket.sysmessage( "You have entered %s." % name )
		return True

# Region Leave Message
def areaLeaveMessage( socket, name ):
	name = name.strip()
	if name == None or len( name ) == 0 or name == "":
		return True
	else:
		socket.sysmessage( "You have left %s." % name )
		return True

# Left Guard's Protection Message
def guardLeaveMessage( socket, owner ):
	if owner == None or owner == "the town" or len( owner ) == 0:
		socket.clilocmessage( 500113 )
		return True
	else:
		socket.sysmessage( "You have left the protection of %s guards." % owner )
		return True

# Enter Guard's Protection Message
def guardEnterMessage( socket, owner ):
	if owner == None or owner == "the town" or len( owner ) == 0:
		socket.clilocmessage( 500112 )
		return True
	else:
		socket.sysmessage( "You are now under protection by %s guards." % owner )
		return True
