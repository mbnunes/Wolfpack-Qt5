#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: codex                          #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: 25-11-03                #
#################################################################

import wolfpack
from wolfpack.consts import *
from wolfpack.utilities import *

def onLoad():
	wolfpack.registerglobal( EVENT_TRADESTART, 'system.trading' )
	wolfpack.registerglobal( EVENT_TRADE, 'system.trading' )

def onLogout( player ):
	if player.hastag( 'trade_partner' ):
		partner = wolfpack.findchar( player.gettag( 'trade_partner' ) )
		box2 = partner.itemonlayer( LAYER_TRADING )
		box1 = player.itemonlayer( LAYER_TRADING )
		closetrade( player, partner, box1, box2 )

	return True
	
def onLogin( player ):
	player.deltag('trade_partner')
	player.deltag('trade_button')
	player.removescript('system.trading')

#
# Disallow using items in trade containers
#
def onUse(char, item):
	if not item.isitem():
		return False
	
	if item.container and item.container.isitem() and item.container.container == char and item.container.layer == LAYER_TRADING:
		return True
	else:
		return False

def onPickup(player1, item):
	box1 = player1.itemonlayer(LAYER_TRADING)
	if not box1:
		return False
		
	if item.container != box1:
		return False
		
	if not player1.hastag('trade_partner'):
		return False
	
	player2 = wolfpack.findchar(int(player1.gettag('trade_partner')))
	if not player2:
		return False
	box2 = player2.itemonlayer(LAYER_TRADING)
	
	if not box2:
		return False
		
	player1.settag( 'trade_button', 0 )
	player2.settag( 'trade_button', 0 )
		
	# Reset the trading status and resend it
	# To me
	sendtradepacket( player1.socket, 2, box1.serial, 0, 0, "" )
	
	# To partner
	sendtradepacket( player2.socket, 2, box2.serial, 0, 0, "" )
	
	item.removefromview(True)
	
	return False

def onTradeAdd( player, item ):
	#not implemented yet in code
	#Refuse:
	#Recently stolen items
	#Trapped items
	#If recipient could not carry that
	#Using of added items ( tag )
	#Refuse stealing of added items ( tag )
	#Refuse pet's transfering while trading ( tag )
	#Refuse trading with someone who dragging something ( tag )

	return True

def onTradeRemove( player, item ):
	#not implemented yet in code

	return True
	
def onDropOnItem( item, box1 ):
	player1 = box1.container
	if not player1.hastag('trade_partner'):
		return False
	player2 = wolfpack.findchar(int(player1.gettag('trade_partner')))
	if not player2:
		return False
	box2 = player2.itemonlayer(LAYER_TRADING)
	
	if not box2:
		return False
		
	player1.settag( 'trade_button', 0 )
	player2.settag( 'trade_button', 0 )
		
	# Reset the trading status and resend it
	# To me
	sendtradepacket( player1.socket, 2, box1.serial, 0, 0, "" )
	
	# To partner
	sendtradepacket( player2.socket, 2, box2.serial, 0, 0, "" )	
	
	return False

#
# This event is called if an item is dropped on another character and
# he is a logged in player.
#
def onTradeStart( player1, player2, firstitem ):
	# If we are trading with someone else right now,
	# trading should be denied
	if player2.hastag('trade_partner'):
		partner = wolfpack.findchar(int(player2.gettag('trade_partner')))
		if partner and partner != player1:
			player1.socket.sysmessage('Your trading partner is currently busy.')
			if not wolfpack.utilities.tobackpack(firstitem, player1):
				firstitem.update()
			return False
			
	if player1.hastag('trade_partner'):
		partner = wolfpack.findchar(int(player1.gettag('trade_partner')))
		if partner and partner != player2:
			player1.socket.sysmessage('You are trading with someone else right now.')
			if not wolfpack.utilities.tobackpack(firstitem, player1):
				firstitem.update()
			return False
	
	#player1 : I am
	#player2 : Partner

	# Creating trade container for me
	box1 = player1.itemonlayer( LAYER_TRADING )
	if not box1:
		box1 = wolfpack.additem( "e75" )
		box1.owner = player1
		player1.additem( LAYER_TRADING, box1 )
		box1.update()
		box1new = 1
	else:
		box1new = 0

	# Same operation for partner
	box2 = player2.itemonlayer( LAYER_TRADING )
	if not box2:
		box2 = wolfpack.additem("e75")
		box2.owner = player2
		player2.additem( LAYER_TRADING, box2 )
		box2.update()
		box2new = 1
	else:
		box2new = 0

	# Unable to create trade containers
	if not box1 or not box2:
		if not wolfpack.utilities.tobackpack(firstitem, player1):
			firstitem.update()
		return False

	# onLogout event should be executed for tradewindow disposing
	player1.addscript( 'system.trading' )
	player2.addscript( 'system.trading' )

	# We want to know serial of partner in future
	player1.settag( 'trade_partner', player2.serial )
	player2.settag( 'trade_partner', player1.serial )

	# We need to store button state of each player
	player1.settag( 'trade_button', 0 )
	player2.settag( 'trade_button', 0 )

	#Send trade window to both players
	#To me
	if box1new:
		sendtradepacket( player1.socket, 0, player2.serial, box1.serial, box2.serial, player2.name )
		
	#To partner
	if box2new:
		sendtradepacket( player2.socket, 0, player1.serial, box2.serial, box1.serial, player1.name )

	#Send buttons state
	#To me
	sendtradepacket( player1.socket, 2, box1.serial, 0, 0, "" )	
	#To partner
	sendtradepacket( player2.socket, 2, box2.serial, 0, 0, "" )	

	box1.additem( firstitem )
	firstitem.update()
	
	box1.addscript('system.trading')
	box2.addscript('system.trading')

	return True

#
# This is called on trade events
#
def onTrade( player, type, buttonstate, itemserial ):
	#Receiving button state, close request etc...
	#itemserial == tradecontainer.serial of player
	#buttonstate == 1 : user confirm deal
	#buttonstate == 0 : user reject deal
	#type == 1 : user want to close trade
	#type == 2 : button pressed
	handler = { 1 : closetrade, 2 : pressbutton }

	#Check if we have a partner
	if player.hastag( 'trade_partner' ):
		partner = wolfpack.findchar( player.gettag( 'trade_partner' ) )
		if not partner:
			return False
	else:
		return False

	#Get tradecontainers
	box2 = partner.itemonlayer( LAYER_TRADING )
	box1 = player.itemonlayer( LAYER_TRADING )

	#Something lost ?
	if not box1 or not box2:
		closetrade( player, partner, box1, box2 )

	#Button pressed ?
	if buttonstate == 1:
		player.settag( 'trade_button', 1 )

	#Execute handler of specified type
	handler[type]( player, partner, box1, box2 )


	return True

def closetrade( player, partner, box1, box2 ):
	#Closing tradewindows, deleting tradecontainers
	#copying items from tradecontainers to chars backpacks
	#Close partner tradewindow
	for p in [player, partner]:
		p.deltag( 'trade_partner' )
		p.deltag( 'trade_button' )

	if box1:
		back1 = player.getbackpack()
		cont2cont( box1, back1 )
		sendclosetrade( player.socket, box1.serial )
		box1.delete()

	if box2:
		back2 = partner.getbackpack()
		cont2cont( box2, back2 )
		sendclosetrade( partner.socket, box2.serial )
		box2.delete()

	return True

def pressbutton( player, partner, box1, box2 ):
	#Switch buttons on trade gump
	button1 = player.gettag( 'trade_button' )
	button2 = partner.gettag( 'trade_button' )
	sendtradepacket( player.socket, 2, box1.serial, button1, button2, "" )
	sendtradepacket( partner.socket, 2, box2.serial, button2, button1, "" )

	#To far away for trading ?
	if player.distanceto( partner ) > 2:
		player.socket.sysmessage('You are too far away to do that.')
		partner.socket.sysmessage('You are too far away to do that.')
		closetrade( player, partner, box1, box2 )
		return True

	if button1 == 1 and button2 == 1:
		back1 = player.getbackpack()
		back2 = partner.getbackpack()
		cont2cont( box2, back1 )
		cont2cont( box1, back2 )
		closetrade( player, partner, box1, box2 )

	return True

def sendtradepacket( socket, action, partnerserial, box1serial, box2serial, playername ):
	#Sending 0x6F packet. Length may vary when playername is defined
	packetlength = 0x10
	trade = wolfpack.packet( 0x6F, packetlength )

	if playername != "":
		packetlength += len( playername ) + 2
		trade.resize( packetlength )
		trade.setbyte( 16, 1 )
		trade.setascii( 17, playername )

	trade.setshort( 1, packetlength )
	trade.setbyte( 3, action )
	trade.setint( 4, partnerserial )
	trade.setint( 8, box1serial )
	trade.setint( 12, box2serial )
	trade.send( socket )

	return True

def sendclosetrade( socket, boxserial ):
	# 2 - close action
	action = 1
	packetlength = 8
	trade = wolfpack.packet( 0x6F, packetlength )
	trade.setshort( 1, packetlength )
	trade.setbyte( 3, action )
	trade.setint( 4, boxserial )
	trade.send( socket )
	return True
