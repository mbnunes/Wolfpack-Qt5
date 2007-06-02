#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts
#  ((    _/{  "-;  | Created by: MagnusBr
#   )).-' {{ ;'`   | Revised by:
#  ( (  ;._ \\ ctr | Last Modification: Created
#===============================================================#
# Quest System - Chat System
#===============================================================#

# "Documentation"
#
# Tags:
# Socket: 'InChat' -> Flag to see if this socket is on Chat
# Socket: 'ChatChannel' -> Channel where this user is
# Socket: 'Moderator' -> Flag to see if this socket is Moderator of current channel

##############################################################################################
##########################  Imports  #########################################################
##############################################################################################

import wolfpack
import wolfpack.sockets
import wolfpack.accounts

##############################################################################################
##########################  Constants  #######################################################
##############################################################################################

ENABLECHAT = 1

DEFAULTCHANNEL = 'General'

###########################################################################################################
###########################################################################################################
######################							     ##############################
######################			Packet Handle			     ##############################
######################							     ##############################
###########################################################################################################
###########################################################################################################

##############################################################################################
##########################  Packet xB3  ######################################################
##############################################################################################

def handleplayermsg(socket, packet):

	length = packet.getshort(1)
	language = packet.getint(3)
	subcommand = packet.getbyte(8)

	# Change Password
	if subcommand == 0x41:
		process_changechannelpassword(socket, length, packet)

	# Close Chat
	elif subcommand == 0x58:
		closechat(socket)

	# Message
	elif subcommand == 0x61:
		process_enviamensagem(socket, length, packet, language)

	# Join Conference
	elif subcommand == 0x62:
		process_trytojoinchannel(socket, length, packet)

	# Create New Conference
	elif subcommand == 0x63:
		process_create_conference(socket, length, packet)

	# Rename Conference
	elif subcommand == 0x64:
		return

	# Send Private Message
	elif subcommand == 0x65:
		process_sendprivate(socket, length, packet, language)

	# Ignore
	elif subcommand == 0x66:
		return

	# Stop Ignoring
	elif subcommand == 0x67:
		return

	# Toggle Ignoring
	elif subcommand == 0x68:
		return

	# Granting Speak Privileges
	elif subcommand == 0x69:
		return

	# Removing Speak Privileges
	elif subcommand == 0x6A:
		return

	# Toggle Speak Privileges
	elif subcommand == 0x6B:
		return

	# Granting Moderator Status
	elif subcommand == 0x6C:
		return

	# Remove Moderator Status
	elif subcommand == 0x6D:
		return

	# Toggle Moderator Status
	elif subcommand == 0x6E:
		return

	# Not Receiving Messages
	elif subcommand == 0x6F:
		return

	# Receiving Messages
	elif subcommand == 0x70:
		return

	# Toggle Receiving Messages
	elif subcommand == 0x71:
		return

	# Show My Character Name
	elif subcommand == 0x72:
		return

	# Dont Show My Character Name
	elif subcommand == 0x73:
		return

	# Toggle Show My Character Name
	elif subcommand == 0x74:
		return

	# Who is this player?
	elif subcommand == 0x75:
		return

	# Kick out the conference
	elif subcommand == 0x76:
		return

	# Only moderators have speaking privileges by default
	elif subcommand == 0x77:
		return

	# Everyone have speaking privileges by default
	elif subcommand == 0x78:
		return

	# Toggle Default Speaking privileges
	elif subcommand == 0x79:
		return

	# Emote
	elif subcommand == 0x7A:
		return

	return

##############################################################################################
##########################  Packet xB5  ######################################################
##############################################################################################

def openchat(socket, packet):

	# Chat System is Enabled?
	if not ENABLECHAT:
		socket.sysmessage('The chat system has been disabled.')
		return

	# Process if we have a ChatName Sent
	chatname = ''

	if socket.account.chatname == '':
		for i in range(1, 64):
			byte = packet.getbyte(i)
			if byte != 0:
				chatname += chr(byte)
	else:
		chatname = socket.account.chatname
		
	
	# So... we have a chat name? If not, lets Open the "Ask Nickname" Panel
	if not chatname:
		send_askfornickname(socket)
	else:
		if socket.account.chatname == '':
			if not checkifnickisfree(chatname):
				socket.account.chatname = chatname
			else:
				socket.sysmessage('Someone already using this Name')
				return
		openchatmenu(socket, chatname)

###########################################################################################################
###########################################################################################################
######################							     ##############################
######################			Functions			     ##############################
######################							     ##############################
###########################################################################################################
###########################################################################################################

##############################################################################################
##########################  Open Chat Menu  ##################################################
##############################################################################################

def openchatmenu(socket, chatname):

	# Saving "InChat" tag on Socket and Chat Name
	socket.settag('InChat', 1)

	# Get Chatname to this user
	chatname = socket.account.chatname

	# Display the Chat Window
	send_displaychat(socket, chatname)

	# Send Default Conference
	send_conference(socket, DEFAULTCHANNEL, 0)

	# Listing Conferences
	listconferences(socket)

	# Enter Channel
	enterchannel(socket, DEFAULTCHANNEL)

##############################################################################################
########################  Trying to Join some Channel  #######################################
##############################################################################################

def trytojoinchannel(socket, channelname, password, havepass):

	# Actual Channel
	actualchannel = socket.gettag('ChatChannel')

	# We're already on this channel?
	if actualchannel == channelname:
		send_alreadyinconference(socket, channelname)
		return

	# Check if Channel Exist
	if channelname == DEFAULTCHANNEL:
		enterchannel(socket, channelname)
	elif checkchannelexist(channelname):
		if not havepass:
			enterchannel(socket, channelname)
		else:
			if checkpassword(socket, channelname, password):
				enterchannel(socket, channelname)
			else:
				send_incorrectpassword(socket)
				
	else:
		send_invalidconferencename(socket)

##############################################################################################
########################  Entering Specific Channel  #########################################
##############################################################################################

def enterchannel(socket, channel):

	# Old Channel
	oldchannel = 0

	# Leaving old Channel
	if socket.hastag('ChatChannel'):
		oldchannel = socket.gettag('ChatChannel')
		chatname = socket.account.chatname
		
		spam_exitchannel(oldchannel, socket)

		# We cant destroy Default Channel
		if oldchannel == DEFAULTCHANNEL:
			oldchannel = 0

	# Saving Channel
	socket.settag('ChatChannel', channel)

	# Old Channel is empty now? So we need to destroy it
	if oldchannel:
	
		poldchannel = 0

		worldsocket = wolfpack.sockets.first()
		while worldsocket:
			# Checking for tags
			if worldsocket.hastag('InChat'):
				if worldsocket.gettag('ChatChannel') == oldchannel:
					poldchannel = 1
					break

			worldsocket = wolfpack.sockets.next()

		if not poldchannel:
			destroychannel(oldchannel)

	# Clear Player List
	send_clearplayerlist(socket)

	# Join the Conference
	send_joinconference(socket, channel)

	# Listing users on this channel
	listusers(socket, channel)

	# Spam this user to all users
	spam_entered(channel, socket)

##############################################################################################
########################  Create New Conference  #############################################
##############################################################################################

def create_conference(socket, channelname, password, havepass):

	# Lets check if we already have a conference with this name
	if channelname == DEFAULTCHANNEL or checkchannelexist(channelname):
		send_alreadyconferencename(socket)
		return

	# Saving Channel
	conferences = int(wolfpack.getoption('channels', '0'))

	wolfpack.setoption('channel_name_' + str(conferences + 1), channelname)
	if havepass:
		wolfpack.setoption('channel_pass_' + str(conferences + 1), password)

	wolfpack.setoption('channels', str(conferences + 1))

	# Channel Spam
	spam_createchannel(channelname, havepass)

	# Getting Moderator Status
	if socket.account.acl == 'player':
		socket.settag('Moderator', 1)

	# Enter this channel
	enterchannel(socket, channelname)

##############################################################################################
########################  Destroy Channel  ###################################################
##############################################################################################

def destroychannel(channel):

	conferences = int(wolfpack.getoption('channels', '0'))

	for i in range(1, conferences+1):

		channelname = wolfpack.getoption('channel_name_%u' % i, '')
		channelpass = wolfpack.getoption('channel_pass_%u' % i, '0')

		if channelname == channel:
			
			for j in range(i + 1, conferences+1):

				channelname2 = wolfpack.getoption('channel_name_%u' % j, '')
				channelpass2 = wolfpack.getoption('channel_pass_%u' % j, '0')

				wolfpack.setoption('channel_name_' + str(j - 1), channelname2)
				if channelpass2:
					wolfpack.setoption('channel_pass_' + str(j - 1), channelpass2)

	wolfpack.setoption('channels', str(conferences - 1))

	# Spam Destroy
	spam_destroychannel(channel)

##############################################################################################
########################  Change Channel Password  ###########################################
##############################################################################################

def changechannelpassword(socket, password):

	# Get the Player, account and Channel
	player = socket.player
	account = socket.account
	channel = socket.gettag('ChatChannel')
	
	# Check if we're Moderators or Staffers
	if account.acl != 'player' or socket.hastag('Moderator'):
		
		conferences = int(wolfpack.getoption('channels', '0'))

		for i in range(1, conferences+1):

			channelname = wolfpack.getoption('channel_name_%u' % i, '')
			channelpass = wolfpack.getoption('channel_pass_%u' % i, '0')

			if channelname == channel:
				wolfpack.setoption('channel_pass_%u' % i, password)
				break

		send_msgpasschanged(socket)

	else:

		send_refuseaction(socket)

##############################################################################################
########################  Closing Chat  ######################################################
##############################################################################################

def closechat(socket):

	# Getting Channel
	channel = socket.gettag('ChatChannel')
	chatname = socket.account.chatname

	# Erase Socket tags
	socket.deltag('InChat')
	socket.deltag('ChatChannel')

	# Old Channel is empty now? So we need to destroy it
	if channel:

		if not channel == DEFAULTCHANNEL:
	
			poldchannel = 0

			worldsocket = wolfpack.sockets.first()
			while worldsocket:
				# Checking for tags
				if worldsocket.hastag('InChat'):
					if worldsocket.gettag('ChatChannel') == channel:
						poldchannel = 1
						break

				worldsocket = wolfpack.sockets.next()

			if not poldchannel:
				destroychannel(channel)

	# Spam: Exiting from Chat
	spam_exitchannel(channel, socket)

	# Set Moderator Status out
	if socket.hastag('Moderator'):
		socket.deltag('Moderator')

##############################################################################################
########################  Send Message  ######################################################
##############################################################################################

def enviamensagem(socket, language, message):

	# Getting Channel
	channel = socket.gettag('ChatChannel')

	# Send Spam
	spam_message(language, socket, channel, message)

##############################################################################################
########################  Send Private Message  ##############################################
##############################################################################################

def sendprivate(socket, language, targetnick, message):

	# Searching for target socket
	targetsocket = searchforsocket(targetnick)
	if targetsocket:
		# Send the Private Message
		send_privatemessage(socket, message, language, targetsocket)

###########################################################################################################
###########################################################################################################
######################							     ##############################
######################			Packets: Process		     ##############################
######################							     ##############################
###########################################################################################################
###########################################################################################################

##############################################################################################
########################  Change Channel Password  ###########################################
##############################################################################################

def process_changechannelpassword(socket, length, packet):

	# Get the Password
	password = ''
	for i in range(9, length - 2):
		byte = packet.getbyte(i)
		if byte != 0:
			password += chr(byte)

	# Send ChannelPassword
	changechannelpassword(socket, password)

##############################################################################################
########################  Try to join Channel  ###############################################
##############################################################################################

def process_trytojoinchannel(socket, length, packet):

	# Flag
	havepass = 0

	# Processing Channel name
	channelname = ''
	for i in range(11, length - 2):

		# We have password?
		tpass =  packet.getshort(i)
		if tpass == 0x0022:
			tpass = packet.getshort(i + 2)
			if tpass == 0x0020:
				havepass = i + 4
				
			break

		# Normal Process
		byte = packet.getbyte(i)
		if byte != 0:
			channelname += chr(byte)

	# Have Password?
	if havepass:
		# Process password
		password = ''
		for i in range(havepass, length - 2):

			# Normal Process
			byte = packet.getbyte(i)
			if byte != 0:
				password += chr(byte)

	if havepass:
		trytojoinchannel(socket, channelname, password, 1)
	else:
		trytojoinchannel(socket, channelname, 0, 0)

##############################################################################################
########################  Send Message  ######################################################
##############################################################################################

def process_enviamensagem(socket, length, packet, language):

	# Processing Message
	message = ''
	for i in range(9, length - 2):
		byte = packet.getbyte(i)
		if byte != 0:
			message += chr(byte)

	# Send Message
	enviamensagem(socket, language, message)

##############################################################################################
########################  Private Message  ###################################################
##############################################################################################

def process_sendprivate(socket, length, packet, language):

	# Processing Target
	targetnick = ''
	for i in range(9, length - 2):

		# We finished name?
		endname =  packet.getshort(i)
		if endname == 0x0020:
			endname = i + 2
			break

		byte = packet.getbyte(i)
		if byte != 0:
			targetnick += chr(byte)

	# Processing Message
	message = ''
	for i in range(endname, length - 2):
		byte = packet.getbyte(i)
		if byte != 0:
			message += chr(byte)

	# Send Private Message
	sendprivate(socket, language, targetnick, message)

##############################################################################################
########################  Create New Conference  #############################################
##############################################################################################

def process_create_conference(socket, length, packet):

	# Password Flag
	havepass = 0

	# Processing Channel name
	channelname = ''
	for i in range(9, length - 2):

		# We have password?
		tpass =  packet.getshort(i)
		if tpass == 0x007b:
			havepass = i + 2
			break

		# Normal Process
		byte = packet.getbyte(i)
		if byte != 0:
			channelname += chr(byte)

	# Have Password?
	if havepass:
		# Process password
		password = ''
		for i in range(havepass, length - 4):

			# Normal Process
			byte = packet.getbyte(i)
			if byte != 0:
				password += chr(byte)

	if havepass:
		create_conference(socket, channelname, password, 1)
	else:
		create_conference(socket, channelname, 0, 0)

###########################################################################################################
###########################################################################################################
######################							     ##############################
######################			Packets: Send			     ##############################
######################							     ##############################
###########################################################################################################
###########################################################################################################

##############################################################################################
##########################  Open "Ask Nickname" Menu  ########################################
##############################################################################################

def send_askfornickname(socket):

	askpacket = wolfpack.packet(0xB2, 13)
	askpacket.setshort(1, 13)
	askpacket.setshort(3, 0x03EB)
	askpacket.send(socket)

##############################################################################################
##########################  Display Chat  ####################################################
##############################################################################################

def send_displaychat(socket, chatname):

	# Data
	length = len(chatname) * 2
	extlength = length + 13

	# Packet
	chatpacket = wolfpack.packet(0xB2, extlength)
	chatpacket.setshort(1, extlength)
	chatpacket.setshort(3, 0x3ED)
	chatpacket.setunicode(7, ' ' + chatname + chatname + ' ')
	chatpacket.send(socket)

##############################################################################################
##########################  Send Conference  #################################################
##############################################################################################

def send_conference(socket, channel, havepass):

	# Data
	length = len(channel) * 2
	extlength = length + 15

	# Creating basic Conference
	chatpacket = wolfpack.packet(0xB2, extlength)
	chatpacket.setshort(1, extlength)
	chatpacket.setshort(3, 0x3E8)
	chatpacket.setunicode(9,channel + channel)
	if havepass:
		chatpacket.setshort((len(channel) * 2) + 11, 0x0031)
	else:
		chatpacket.setshort((len(channel) * 2) + 11, 0x0030)
	chatpacket.send(socket)

##############################################################################################
##########################  Clear Player List  ###############################################
##############################################################################################

def send_clearplayerlist(socket):
	
	chatpacket = wolfpack.packet(0xB2, 13)
	chatpacket.setshort(1, 13)
	chatpacket.setshort(3, 0x3F0)
	chatpacket.send(socket)

##############################################################################################
##########################  Join Channel  ####################################################
##############################################################################################

def send_joinconference(socket, channel):

	# Data
	length = len(channel) * 2
	extlength = length + 13

	# Send Packet
	chatpacket = wolfpack.packet(0xB2, extlength)
	chatpacket.setshort(1, extlength)
	chatpacket.setshort(3, 0x3F1)
	chatpacket.setunicode(7, ' ' + channel + channel + ' ')
	chatpacket.send(socket)

##############################################################################################
##########################  Send Private Message  ############################################
##############################################################################################

def send_privatemessage(socket, message, language, targetsocket):

	# Get Chatname of Sender
	nick = socket.account.chatname

	# Decoding Message
	message = unicode(message, 'latin-1')

	# Data
	length = len(nick) * 2
	length += len(message) * 2
	length += 25
		
	# Send Packet
	chatpacket = wolfpack.packet(0xB2, length)
	chatpacket.setshort(1, length)
	chatpacket.setint(4, language)
	chatpacket.setshort(3, 0x0025)

	if socket == targetsocket:
		chatpacket.setbyte(9, 0x0034)
	elif socket.account.acl != 'player' or socket.hastag('Moderator'):
		chatpacket.setbyte(9, 0x0031)
	else:
		chatpacket.setshort(9, 0x0030)
	
	chatpacket.setunicode(11, '[PM] ' + nick + nick + ' [PM]')
	chatpacket.setunicode(23 + (len(nick) * 2), ' ' + message + message + ' ')
	chatpacket.send(targetsocket)

##############################################################################################
##########################  Send Message  ####################################################
##############################################################################################

def send_message(worldsocket, socket, message, language):

	# Get Chatname of Sender
	chatname = socket.account.chatname

	# Decoding Message
	message = unicode(message, 'latin-1')
	
	# Data
	length = len(chatname) * 2
	length += len(message) * 2
	length += 17
		
	# Send Message
	chatpacket = wolfpack.packet(0xB2, length)
	chatpacket.setshort(1, length)
	chatpacket.setint(4, language)
	chatpacket.setshort(3, 0x0025)

	if socket == worldsocket:
		chatpacket.setbyte(9, 0x0034)
	elif socket.account.acl != 'player' or socket.hastag('Moderator'):
		chatpacket.setbyte(9, 0x0031)
	else:
		chatpacket.setshort(9, 0x0030)

	chatpacket.setunicode(11, ' ' + chatname + chatname + ' ')
	chatpacket.setunicode(15 + (len(chatname) * 2), ' ' + message + message + ' ')
	chatpacket.send(worldsocket)

##############################################################################################
##########################  Send AddUser  ####################################################
##############################################################################################

def send_adduser(socket, worldsocket):

	# Get Chatname
	chatname = socket.account.chatname

	# Data
	length = len(chatname) * 2
	extlength = length + 13
			
	# Sending
	chatpacket = wolfpack.packet(0xB2, extlength)
	chatpacket.setshort(1, extlength)
	chatpacket.setshort(3, 0x3EE)
	
	if socket == worldsocket:
		chatpacket.setbyte(8, 0x34)
	elif socket.account.acl != 'player' or socket.hastag('Moderator'):
		chatpacket.setbyte(8, 0x31)
	else:
		chatpacket.setshort(8, 0x30)
	
	chatpacket.setunicode(9, ' ' + chatname + chatname + ' ')
	chatpacket.send(worldsocket)

##############################################################################################
##########################  Remove User  #####################################################
##############################################################################################

def send_removeuser(socket, chatname):

	# Data
	length = len(chatname) * 2
	extlength = length + 9
	
	# Sending
	chatpacket = wolfpack.packet(0xB2, extlength)
	chatpacket.setshort(1, extlength)
	chatpacket.setshort(3, 0x3EF)
	chatpacket.setunicode(5, ' ' + chatname + chatname + ' ')
	chatpacket.send(socket)

##############################################################################################
##########################  Destroy Channel  #################################################
##############################################################################################

def send_destroychannel(socket, channel):

	# Sending
	chatpacket = wolfpack.packet(0xB2, (len(channel) * 2) + 15)
	chatpacket.setshort(1, (len(channel) * 2) + 15)
	chatpacket.setshort(3, 0x3E9)
	chatpacket.setunicode(9,channel + channel)
	chatpacket.send(socket)

##############################################################################################
##############  Send "The password to the conference has been changed"  ######################
##############################################################################################

def send_msgpasschanged(socket):

	chatpacket = wolfpack.packet(0xB2, 11)
	chatpacket.setshort(1, 11)
	chatpacket.setshort(3, 0x0028)
	chatpacket.send(socket)

##############################################################################################
##############  Send "You are already in the conference '%1'"  ###############################
##############################################################################################

def send_alreadyinconference(socket, channelname):

	chatpacket = wolfpack.packet(0xB2, 11 + (len(channelname) * 2))
	chatpacket.setshort(1, 11 + (len(channelname) * 2))
	chatpacket.setshort(3, 0x001A)
	chatpacket.setunicode(9,channelname + channelname)
	chatpacket.send(socket)

##############################################################################################
##############  Send "That is not the correct password."  ####################################
##############################################################################################

def send_incorrectpassword(socket):

	chatpacket = wolfpack.packet(0xB2, 11)
	chatpacket.setshort(1, 11)
	chatpacket.setshort(3, 0x000E)
	chatpacket.send(socket)

##############################################################################################
##############  Send "That is not a valid conference name."  #################################
##############################################################################################

def send_invalidconferencename(socket):

	chatpacket = wolfpack.packet(0xB2, 11)
	chatpacket.setshort(1, 11)
	chatpacket.setshort(3, 0x0007)
	chatpacket.send(socket)

##############################################################################################
##############  Send "There is already a conference of that name."  ##########################
##############################################################################################

def send_alreadyconferencename(socket):

	chatpacket = wolfpack.packet(0xB2, 11)
	chatpacket.setshort(1, 11)
	chatpacket.setshort(3, 0x0008)
	chatpacket.send(socket)	

##############################################################################################
##############  Send "You must have operator status to do this"  #############################
##############################################################################################

def send_refuseaction(socket):

	chatpacket = wolfpack.packet(0xB2, 11)
	chatpacket.setshort(1, 11)
	chatpacket.setshort(3, 0x0009)
	chatpacket.send(socket)

###########################################################################################################
###########################################################################################################
######################							     ##############################
######################			Packets: Spam			     ##############################
######################							     ##############################
###########################################################################################################
###########################################################################################################

##############################################################################################
##########################  Spam: Creating Channel  ##########################################
##############################################################################################

def spam_createchannel(channel, haspassword):

	# Loop
	worldsocket = wolfpack.sockets.first()
	while worldsocket:

		# Checking for tags
		if worldsocket.hastag('InChat'):
			
			send_conference(worldsocket, channel, haspassword)

		worldsocket = wolfpack.sockets.next()

##############################################################################################
##########################  Spam: Message  ###################################################
##############################################################################################

def spam_message(language, socket, channel, message):

	# Loop
	worldsocket = wolfpack.sockets.first()
	while worldsocket:

		# Checking for tags
		if worldsocket.hastag('InChat'):
			if worldsocket.gettag('ChatChannel') == channel:
				# Send Message
				send_message(worldsocket, socket, message, language)

		worldsocket = wolfpack.sockets.next()

##############################################################################################
##########################  Spam: New on Channel  ############################################
##############################################################################################

def spam_entered(channel, socket):

	# Loop
	worldsocket = wolfpack.sockets.first()
	while worldsocket:

		# Checking for tags
		if worldsocket.hastag('InChat'):
			if worldsocket.gettag('ChatChannel') == channel:
				
				# Send this user
				send_adduser(socket, worldsocket)

		worldsocket = wolfpack.sockets.next()

##############################################################################################
##########################  Spam: Exiting Channel  ###########################################
##############################################################################################

def spam_exitchannel(channel, socket):

	# Get Chatname
	chatname = socket.account.chatname

	# Loop
	worldsocket = wolfpack.sockets.first()
	while worldsocket:

		# Non-sense to send to ourselves
		if socket == worldsocket:
			continue

		# Checking for tags
		if worldsocket.hastag('InChat'):
			if worldsocket.gettag('ChatChannel') == channel:
				
				# Sending the Removed User
				send_removeuser(worldsocket, chatname)

		worldsocket = wolfpack.sockets.next()

##############################################################################################
##########################  Spam: Destroy Channel  ###########################################
##############################################################################################

def spam_destroychannel(channel):
	
	# Loop
	worldsocket = wolfpack.sockets.first()
	while worldsocket:

		# Checking for tags
		if worldsocket.hastag('InChat'):
			
			send_destroychannel(worldsocket, channel)

		worldsocket = wolfpack.sockets.next()

###########################################################################################################
###########################################################################################################
######################							     ##############################
######################			Chat Utils			     ##############################
######################							     ##############################
###########################################################################################################
###########################################################################################################

##############################################################################################
########################  Listing all Channels  ##############################################
##############################################################################################

def listconferences(socket):

	conferences = int(wolfpack.getoption('channels', '0'))
	
	if conferences:
		
		for i in range(1, conferences+1):
			
			channelname = wolfpack.getoption('channel_name_%u' % i, '')
			channelpass = wolfpack.getoption('channel_pass_%u' % i, '0')

			if len(channelname):

				send_conference(socket, channelname, channelpass)

##############################################################################################
########################  Check if Channel Name Exist  #######################################
##############################################################################################

def checkchannelexist(channel):
	
	# Flag
	exist = 0

	conferences = int(wolfpack.getoption('channels', '0'))

	for i in range(1, conferences+1):

		channelname = wolfpack.getoption('channel_name_%u' % i, '')
		channelpass = wolfpack.getoption('channel_pass_%u' % i, '0')

		if channelname == channel:
			exist = 1
			break

	return exist

##############################################################################################
########################  Check Password for a Channel  ######################################
##############################################################################################

def checkpassword(socket, channel, password):

	# Flag
	correctpass = 0

	conferences = int(wolfpack.getoption('channels', '0'))

	for i in range(1, conferences+1):

		channelname = wolfpack.getoption('channel_name_%u' % i, '')
		channelpass = wolfpack.getoption('channel_pass_%u' % i, '0')

		if channelname == channel:
			if channelpass == password:
				correctpass = 1
			break

	return correctpass

##############################################################################################
########################  Search for a Target Socket  ########################################
##############################################################################################

def searchforsocket(chatname):

	# Loop
	worldsocket = wolfpack.sockets.first()
	while worldsocket:

		# Checking for tags
		if worldsocket.hastag('InChat'):
			
			socketnick = worldsocket.account.chatname

			if socketnick == chatname:

				return worldsocket

		worldsocket = wolfpack.sockets.next()

##############################################################################################
########################  Listing all Users on a Channel  ####################################
##############################################################################################

def listusers(socket, channel):

	# Loop
	worldsocket = wolfpack.sockets.first()
	while worldsocket:

		# Checking for tags
		if worldsocket.hastag('InChat') and not worldsocket == socket:
			if worldsocket.gettag('ChatChannel') == channel:

				# Get Chatname
				chatname = worldsocket.account.chatname
				
				# Sending the message
				length = len(chatname) * 2
				extlength = length + 13
			
				chatpacket = wolfpack.packet(0xB2, extlength)
				chatpacket.setshort(1, extlength)
				chatpacket.setshort(3, 0x3EE)
				if worldsocket.account.acl != 'player' or worldsocket.hastag('Moderator'):
					chatpacket.setbyte(8, 0x31)
				else:
					chatpacket.setbyte(8, 0x30)
				chatpacket.setunicode(9, ' ' + chatname + chatname + ' ')
				chatpacket.send(socket)

		worldsocket = wolfpack.sockets.next()

##############################################################################################
########################  Check if a Nick is Avaliable  ######################################
##############################################################################################

def checkifnickisfree(chatname):

	acclist = wolfpack.accounts.list
	for i in acclist():
		record = wolfpack.accounts.find( i )
		if record.chatname == chatname:
			return True
	
	return False

###########################################################################################################
###########################################################################################################
######################							     ##############################
######################			    Handlers			     ##############################
######################							     ##############################
###########################################################################################################
###########################################################################################################

##############################################################################################
##########################  Packet Handlers  #################################################
##############################################################################################

def onLoad():
	wolfpack.registerpackethook(0xB3, handleplayermsg)
	wolfpack.registerpackethook(0xB5, openchat)