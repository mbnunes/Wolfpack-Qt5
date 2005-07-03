import wolfpack

recharges = {
"citrine" : [['f15', 'f23', 'f24', 'f2c'], 500],
"amber" : [['f25'], 500],
"tourmaline" : [['f18', 'f1e', 'f20', 'f2d'], 750],
"emerald" : [['f10', 'f2f'], 1000],
"sapphire" : [['f11', 'f12', 'f19', 'f1f'], 1000],
"amethyst" : [['f16', 'f17', 'f22', 'f2e'], 1000],
"starsapphire" : [['f0f', 'f1b', 'f21'], 1250],
"diamond" : [['f26','f27','f28','f29','f30'], 2000]
}

MaxCharges = 2000

def getrechargeinfo(gem):
	new_charge = recharges.get(gem)
	return new_charge[1]

def getname( baseid ):
	for gem in recharges.keys():
		name = recharges[gem][0]
		if baseid in name:
			return gem
	return False

def active( item ):
	if item.hastag( "active" ):
		return True
	return False

def charges( item ):
	charge = 0
	if item.hastag( "charges" ):
		charge = int(item.gettag( "charges" ))
	return charge

def receivers( item ):
	amount = 0
	for tag in item.tags:
		if tag.startswith( "receiver_" ):
			amount += 1
	return amount

def activate_sender( item ):
	item.settag( "active", 0 )
	item.id = 0x1ecd
	item.update()

def activate_receiver( item ):
	item.settag( "active", 0 )
	item.id = 0x1ed1
	item.update()

def deactivate( item ):
	item.deltag( "active" )
	item.id = 0x1ed0
	item.update()

def deltags_receiver( item ):
	try:
		for i in item.tags:
			if i.startswith( "receiver_" ):
				item.deltag(i)
				receiver = wolfpack.finditem( int(i.split("_")[1]) )
				receiver.deltag( "sender" )
	except:
		pass
	return

def deltags_sender( item ):
	if item.hastag( "sender" ):
		sender = wolfpack.finditem( int(item.gettag("sender")) )
		sender.deltag( "receiver_%s" % item.serial )
		sender.resendtooltip()
	return

def onDelete( item ):
	if item.baseid == "comcrystal_sender":
		deltags_receiver( item )
	else:
		deltags_sender( item )
	return

def onUse( char, item ):
	if not char.canreach( item, 2 ):
		char.socket.clilocmessage( 1019045 ) # I can't reach that.
		return False
	if item.baseid == "comcrystal_sender":
		char.socket.attachtarget( "comcrystals.response_sender", [item.serial] )
	else:
		char.socket.attachtarget( "comcrystals.response_receiver", [item.serial] )
	return True

def response_sender( char, args, target ):
	item = wolfpack.finditem( args[0] )
	if not item:
		return False
	if not item.getoutmostchar() == char and not char.canreach( target.pos, 2 ):
		char.socket.clilocmessage( 1019045 ) # I can't reach that.
		return False

	if target.item and target.item.serial == item.serial:
		if active(item):
			deactivate( item )
			char.socket.clilocmessage( 500672 ) # You turn the crystal off.
		else:
			if int(charges(item)) > 0:
				activate_sender( item )
				char.socket.clilocmessage( 500673 ) # You turn the crystal on.
			else:
				char.socket.clilocmessage( 500676 ) # This crystal is out of charges.
		#item.resendtooltip()

	elif target.item and target.item.baseid == "comcrystal_receiver":
		if receivers(item) >= 10:
			char.socket.cliloc( 1010042 ) # This broadcast crystal is already linked to 10 receivers.
			return False
		elif target.item.gettag( "sender" ) == item.serial:
			char.socket.clilocmessage( 500674 ) # This crystal is already linked with that crystal.
		elif target.item.hastag( "sender" ):
			char.socket.clilocmessage( 1010043 ) # That receiver crystal is already linked to another broadcast crystal.
		else:
			item.settag( "receiver_%s" % target.item.serial, 0 )
			target.item.settag( "sender", item.serial )
			char.socket.clilocmessage( 500675 ) # That crystal has been linked to this crystal.
			item.resendtooltip()

	elif target.char:
		for i in item.tags:
			if i.startswith( "receiver_" ):
				item.deltag(i)
				receiver = wolfpack.finditem( int(i.split("_")[1]) )
				receiver.deltag( "sender" )
		char.socket.clilocmessage( 1010046 ) # You unlink the broadcast crystal from all of its receivers.
		item.resendtooltip()

	else:
		if getname(target.item.baseid) in recharges.keys():
			if charges(item) >= MaxCharges:
				char.socket.clilocmessage( 500678 ) # This crystal is already fully charged.
			else:
				char.socket.sysmessage( "baseid: " + str(getname(target.item.baseid)))
				gem = getname( target.item.baseid )
				new_charges = getrechargeinfo(gem)
				target.item.delete()
				if ( charges(item) + new_charges ) >= MaxCharges:
					item.settag( "charges", MaxCharges )
					char.socket.clilocmessage( 500679 ) # You completely recharge the crystal.
				else:
					item.settag( "charges", charges(item) + new_charges )
					char.socket.clilocmessage( 500680 ) # You recharge the crystal.
		char.socket.clilocmessage( 500681 ) # You cannot use this crystal on that.
	return True

def response_receiver( char, args, target ):
	item = wolfpack.finditem( args[0] )
	if not item:
		return False
	if not item.getoutmostchar() == char and not char.canreach( target.pos, 2 ):
		char.socket.clilocmessage( 1019045 ) # I can't reach that.
		return False

	if target.item and target.item.serial == item.serial:
		if active(item):
			deactivate( item )
			char.socket.clilocmessage( 500672 ) # You turn the crystal off.
		else:
			activate_receiver( item )
			char.socket.clilocmessage( 500673 ) # You turn the crystal on.

	elif target.char:
		if item.hastag( "sender" ):
			deltags_sender( item )
			item.deltag( "sender" )
			char.socket.clilocmessage( 1010044 ) # You unlink the receiver crystal.
		else:
			char.socket.clilocmessage( 1010045 ) # That receiver crystal is not linked.
		#item.resendtooltip()

	else:
		if getname(target.item.baseid) in recharges.keys():
			char.socket.clilocmessage( 500677 ) # This crystal cannot be recharged.
			return True
		char.socket.clilocmessage( 1010045 ) # That receiver crystal is not linked.
	return True

def onShowTooltip(player, object, tooltip):
	if active(object):
		tooltip.add(1060742,"")
	else:
		tooltip.add(1060743,"")
	if object.baseid == "comcrystal_sender":
		tooltip.add( 1060745,"" )
	else:
		tooltip.add( 1060744,"" )
	if object.baseid == "comcrystal_sender":
		tooltip.add( 1060741, str(charges(object)) )
	if receivers(object) > 0:
		tooltip.add(1060746, str(receivers(object)) )
	return

def onContextEntry(player, object, entry):
	if entry == 400:
		object.settag( "active", 0 )
	if entry == 401:
		object.deltag( "active" )
	object.resendtooltip()
	return True

def onContextCheckVisible(player, object, tag):
	if tag == 400:
		if not object.hastag( "active" ):
			return True
	if tag == 401:
		if object.hastag( "active" ):
			return True
	return False
