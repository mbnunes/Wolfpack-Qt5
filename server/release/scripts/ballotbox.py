
import housing
import deeds.carpentry_deed
import wolfpack.gumps
import system.input

MaxTopicLines = 6

def isOwner( char ):
	house = housing.findHouse( char )
	if char.gm or deeds.carpentry_deed.checkmulti( char ):
		return True
	return False

def onDropOnItem(target, item):
	target.say( 500369 ) # I'm a ballot box, not a container!
	return True

def onUse( char, item ):
	if not char.canreach( item, 2 ):
		char.socket.clilocmessage( 1019045 ) # I can't reach that.
		return False
	sendgump(char, item)
	return True

def topic_length(item):
	length = 0
	for tag in item.tags:
		if tag.startswith( "topic_" ):
			length += 1
	return length

def sendgump( char, item ):
	gump = wolfpack.gumps.cGump()
	gump.setCallback(callback_maingump)
	gump.setArgs([item.serial])

	gump.addBackground(0xA28, 400, 350)
	if isOwner( char ):
		gump.addXmfHtmlGump(0, 15, 400, 35, 1011000) # <center>Ballot Box Owner's Menu</center>
	else:
		gump.addXmfHtmlGump(0, 15, 400, 35, 1011001) # <center>Ballot Box -- Vote Here!</center>
	gump.addXmfHtmlGump(0, 50, 400, 35, 1011002) # <center>Topic</center>
	lineCount = topic_length(item)
	gump.addResizeGump(25, 90, 5120, 350, max( 20 * lineCount, 20))
	
	# TODO
	offset = 0
	for i in range( 1, topic_length(item) + 1 ):
		line = item.gettag( "topic_%s" % i )
		gump.addCroppedText(30, 90 + offset * 20, 340, 20, line, 0x3e3)
		offset += 1

	yes_count = 0
	no_count = 0
	if item.hastag( "yes" ):
		yes_count = item.gettag( "yes" )
	if item.hastag( "no" ):
		no_count = item.gettag( "no" )
	total_votes = no_count + yes_count

	gump.addXmfHtmlGump(0, 215, 400, 35, 1011003) # <center>votes</center>
	if not isOwner( char ):
		gump.addButton( 20, 240, 0xFA5, 0xFA7, 1 )
	gump.addXmfHtmlGump( 55, 242, 25, 35, 1011004 ) # aye:
	gump.addText( 78, 242, "[%s]" % yes_count )
	if not isOwner( char ):
		gump.addButton( 20, 275, 0xFA5, 0xFA7, 2 )
	gump.addXmfHtmlGump( 55, 277, 25, 35, 1011005 ) # nay:
	gump.addText( 78, 277, "[%s]" % no_count )
	if total_votes > 0:
		gump.addTiledGump(130, 242, ( yes_count * 5 ), 10, 214 )
		gump.addTiledGump(130, 277, ( no_count * 5 ), 10, 214 )
	gump.addButton( 45, 305, 0xFA5, 0xFA7, 0 );
	gump.addXmfHtmlGump( 80, 308, 40, 35, 1011008 ) # done

	if isOwner( char ):
		gump.addButton( 120, 305, 0xFA5, 0xFA7, 3 );
		gump.addXmfHtmlGump( 155, 308, 100, 35, 1011006 ) # change topic

		gump.addButton( 240, 305, 0xFA5, 0xFA7, 4 );
		gump.addXmfHtmlGump( 275, 308, 300, 100, 1011007 ) # reset votes


	gump.send(char)
	return False

def clearVotes( item ):
	for tag in item.tags:
		item.deltag( tag )
	return

def clearTopic( item ):
	for i in range( 1, topic_length(item) + 1 ):
		item.deltag( "topic_%s" % i )
	return

def addVote( char, item, decision=None ):
	if decision == "yes":
		if not item.hastag( "yes" ):
			item.settag( "yes", 1 )
		else:
			item.settag( "yes", item.gettag( "yes" ) + 1 )
	else:
		if not item.hastag( "no" ):
			item.settag( "no", 1 )
		else:
			item.settag( "no", item.gettag( "no" ) + 1 )
	item.settag( "voted_%s" % char.serial, 0 )
	return

def callback_maingump( char, args, response ):
	box = wolfpack.finditem( args[0] )
	if not box or response.button == 0:
		return False
	if not char.canreach( box, 2 ):
		char.socket.clilocmessage( 1019045 ) # I can't reach that.
		return False
	if response.button == 4: # reset votes
		clearVotes( box )
		char.socket.clilocmessage( 500371 ) # Votes zeroed out.
		sendgump( char, box )
	if response.button == 3: # set topic
		if isOwner( char ):
			clearTopic( box )
			char.socket.clilocmessage( 500370, "", 0x35 ) # Enter a line of text for your ballot, and hit ENTER. Hit ESC after the last line is entered.
			system.input.request(char, box, 1)
		return True
	if response.button == 1: # aye
		if not isOwner( char ):
			if box.hastag( "voted_%s" % char.serial ):
				char.socket.clilocmessage( 500374 ) # You have already voted on this ballot.
			else:
				addVote(char, box, "yes")
				char.socket.clilocmessage( 500373 ) # Your vote has been registered.
		sendgump( char, box )
	if response.button == 2: # nay
		if not isOwner( char ):
			if box.hastag( "voted_%s" % char.serial ):
				char.socket.clilocmessage( 500374 ) # You have already voted on this ballot.
			else:
				addVote(char, box, "no")
				char.socket.clilocmessage( 500373 ) # Your vote has been registered.
		sendgump( char, box )
	return

def onTextInput(player, item, inputid, text):
	topicline = text
	item.settag( "topic_%s" % inputid, text )
	if topic_length(item) < 6:
		player.socket.clilocmessage( 500377, "", 0x35 ) # Next line or ESC to finish
		system.input.request(player, item, topic_length(item) + 1)
	else:
		 player.socket.clilocmessage( 500376, "", 0x35 ) # Ballot entry complete.
	return
