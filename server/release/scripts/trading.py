import wolfpack
from wolfpack.consts import *
from wolfpack.utilities import rusmsg

def onLoad():
	wolfpack.registerglobal( EVENT_TRADESTART, 'trading' )
	wolfpack.registerglobal( EVENT_TRADE, 'trading' )

def onTradeStart( player1, player2 ):
	rusmsg( player1, 'Вызвано создание окна' )

	#Sending trade window to both players etc
	action = 0

	box1 = player1.itemonlayer( LAYER_TRADING )
	if not box1:
		box1 = wolfpack.additem("e75")
		box1.layer = LAYER_TRADING
		box1.owner = player1.serial

	box2 = player2.itemonlayer( LAYER_TRADING )
	if not box2:
		box2 = wolfpack.additem("e75")
		box2.layer = LAYER_TRADING
		box2.owner = player2.serial
		
	
	packet1 = wolfpack.packet( 0x6F, 17 )
	packet2 = wolfpack.packet( 0x6F, 17 )
	
	#set partners
	packet1.setint( 4, player2.serial )	
	packet2.setint( 4, player1.serial )
	packet1.setascii( 17, player1.name )
	packet2.setascii( 17, player2.name )
	
	#box to us
	packet1.setint( 8, box1.serial )
	packet1.setint( 12, box2.serial )
	
	#box to partner
	packet2.setint( 8, box2.serial )
	packet2.setint( 8, box1.serial )
	
	packet1.send( player1.socket )
	packet2.send( player2.socket )

	#Partner
	packet1.setint( 4, box1.serial )
	#Box1
	packet1.setint( 8, 0 )
	#Box2
	packet1.setint( 12, 0 )
	#Name
	packet1.setascii( 17, '')
	packet1.send( player1.socket )
	
	#Partner
	packet2.setint( 4, box2.serial )
	#Box1
	packet2.setint( 8, 0 )
	#Box2
	packet2.setint( 12, 0 )
	#Name
	packet2.setascii( 17, '')
	packet2.send( player2.socket )


	return OK

def onTrade( player, type, buttonstate, itemserial ):
	#Receiving button state etc
	rusmsg( player, 'Торговля' )
	return OK
