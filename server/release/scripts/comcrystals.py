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

def onUse( char, item ):
	if not char.canreach( item, 2 ):
		char.socket.clilocmessage( 1019045 ) # I can't reach that.
		return False
	char.socket.attachtarget( "comcrystals.response", [item.serial] )
	return True

def response( char, args, target ):
	item = wolfpack.finditem( args[0] )
	if not item:
		return False
	if not item.getoutmostchar() == char and not char.canreach( target.pos, 2 ):
		char.socket.clilocmessage( 1019045 ) # I can't reach that.
		return False

	if target.item and target.item.serial == item.serial:
		if active(item):
			item.deltag( "active" )
			char.socket.clilocmessage( 500672 ) # You turn the crystal off.
		else:
			if int(charges(item)) > 0:
				item.settag( "active", 0 )
				char.socket.clilocmessage( 500673 ) # You turn the crystal on.
			else:
				char.socket.clilocmessage( 500676 ) # This crystal is out of charges.
		item.resendtooltip()

	elif target.item and target.item.baseid == "receiver_crystal":
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


def onShowTooltip(player, object, tooltip):
	tooltip.reset()
	tooltip.add(0xF9060 + object.id, '')
	if active(object):
		tooltip.add(1060742,"")
	else:
		tooltip.add(1060743,"")
	tooltip.add( 1060745,"" )
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
