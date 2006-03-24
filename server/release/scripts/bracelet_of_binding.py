#
# ToDo: reload charges; testing
#

import wolfpack
import system.input
from wolfpack.consts import *

MaxCharges = 999
TRANSPORT_TIMER = 2000

def onUse(char, item):
	char.socket.sysmessage("A")
	bounded = bound(item)
	if not bounded:
		bind(char, item)
	else:
		activate(char, item)
	return True

def activate(char, item):
	bounded = bound(item)
	if not item or not bounded:
		return False

	if not item.getoutmostchar() == char:
		char.socket.clilocmessage(1042664) # You must have the object in your backpack to use it.
	elif item.hastag('timer'):
		char.socket.clilocmessage(1054013) # The bracelet is already attempting contact. You decide to wait a moment.
	else:
		char.soundeffect(0xF9)
		char.message("* You concentrate on the bracelet to summon its power *", 0x5D)

		char.frozen = True
		item.settag('timer', 0)

		wolfpack.addtimer( TRANSPORT_TIMER, transport_timer, [char.serial, item.serial, bounded] )

def transport_timer(object, args):
	char = wolfpack.findchar(args[0])
	item = wolfpack.finditem(args[1])
	bound = args[2]
	char.frozen = False
	if not item:
		return False

	item.deltag('timer')
	if CheckUse(char, item, False):
		boundRoot = bound.getoutmostchar().socket
		if boundRoot:
			item.settag('charges', charges(item) - 1)
			item.resendtooltip()

			#
			# ToDo: TeleportPets
			#

			char.soundeffect(0x1fc)
			char.moveto(boundRoot.player.pos)
			char.soundeffect(0x1fc)
			char.update()

def bound(item):
	bound = None
	if item.hastag('bound'):
		bound = wolfpack.finditem(item.gettag('bound'))
	return bound

def charges( item ):
	charge = 0
	if item.hastag( "charges" ):
		charge = int(item.gettag( "charges" ))
	return charge

def CheckUse(char, item, successMessage = True):
	bounded = bound(item)
	if not bounded:
		return False

	boundRoot = bounded.getoutmostchar()

	if charges(item) == 0:
		char.socket.clilocmessage(1054005) # The bracelet glows black. It must be charged before it can be used again.
		return False
	elif not item.layer:
		char.socket.clilocmessage(1054004) # You must equip the bracelet in order to use its power.
		return False
	elif not boundRoot or not boundRoot.socket or not bounded.layer == LAYER_BRACELET:
#	elif not bound.socket or bound.itemonlayer(LAYER_BRACELET) != bound:
		char.socket.clilocmessage(1054006) # The bracelet emits a red glow. The bracelet's twin is not available for transport.
		return False
	elif not AGEOFSHADOWS and char.pos.map != boundRoot.pos.map:
		char.socket.clilocmessage( 1054014 ) # The bracelet glows black. The bracelet's target is on another facet.
		return False

	#
	# some things are missing here...
	#
#	else if ( Factions.Sigil.ExistsOn( from ) )
#	{
#		from.SendLocalizedMessage( 1061632 ); // You can't do that while carrying the sigil.
#		return false;
#	}
#	else if ( !SpellHelper.CheckTravel( from, TravelCheckType.RecallFrom ) )
#	{
#		return false;
#	}
#	else if ( !SpellHelper.CheckTravel( from, boundRoot.Map, boundRoot.Location, TravelCheckType.RecallTo ) )
#	{
#		return false;
#	}
#	else if ( boundRoot.Map == Map.Felucca && from is PlayerMobile && ((PlayerMobile)from).Young )
#	{
#		from.SendLocalizedMessage( 1049543 ); // You decide against traveling to Felucca while you are still young.
#		return false;
#	}
#	else if ( from.Kills >= 5 && boundRoot.Map != Map.Felucca )
#	{
#		from.SendLocalizedMessage( 1019004 ); // You are not allowed to travel there.
#		return false;

	elif char.iscriminal():
		char.socket.clilocmessage(1005561) # Thou'rt a criminal and cannot escape so easily.
		return False
	elif char.getopponents():
		char.socket.clilocmessage(1005564) # Wouldst thou flee during the heat of battle??
		return False
	elif char.overloaded:
		char.socket.clilocmessage(502359) # Thou art too encumbered to move.
		return False
	elif char.jailed:
		char.socket.clilocmessage(1041530) # You'll need a better jailbreak plan then that!
		return False
	elif boundRoot.jailed:
		char.socket.clilocmessage(1019004) # You are not allowed to travel there.
		return False
	else:
		if successMessage:
			char.socket.clilocmessage( 1054015 ) # The bracelet's twin is available for transport.
		return True

def bind(char, item):
	if not item:
		return False

	if not item.getoutmostchar() == char:
		char.socket.clilocmessage(1042664) # You must have the object in your backpack to use it.
	else:
		char.socket.clilocmessage(1054001) # Target the bracelet of binding you wish to bind this bracelet to.
		char.socket.attachtarget( 'bracelet_of_binding.bind_target', [item.serial] )

def bind_target(char, args, target):
	item = wolfpack.finditem(args[0])
	if not item:
		return False

	if not item.getoutmostchar() == char:
		char.socket.clilocmessage(1042664) # You must have the object in your backpack to use it.
	elif target.item and target.item.hasscript('bracelet_of_binding'):
		if target.item == item:
			char.socket.clilocmessage(1054012) # You cannot bind a bracelet of binding to itself!
		elif not target.item.getoutmostchar() == char:
			char.socket.clilocmessage(1042664) # You must have the object in your backpack to use it.
		else:
			char.socket.clilocmessage(1054003) # You bind the bracelet to its counterpart. The bracelets glow with power.
			char.soundeffect(0x1fa)

			item.settag('bound', target.item.serial)
	else:
		char.socket.clilocmessage(1054002) # You can only bind this bracelet to another bracelet of binding!

def inscribe(char, item):
	if not item:
		return False
	if not item.getoutmostchar() == char:
		char.socket.clilocmessage(1042664) # You must have the object in your backpack to use it.
	else:
		char.socket.clilocmessage(1054009) # Enter the text to inscribe upon the bracelet :
		system.input.request(char, item, 2)

def search(char, item):
	bounded = bound(item)

	if not (item or bounded):
		return False

	if not item.getoutmostchar() == char:
		char.socket.clilocmessage(1042664) # You must have the object in your backpack to use it.
	else:
		CheckUse(char, item, True)

def onTextInputCancel(player, item, inputid):
	player.socket.clilocmessage(1054010) # You decide not to inscribe the bracelet at this time.

def onTextInput(player, item, inputid, text):
	if not item:
		return False
	if not item.getoutmostchar() == player:
		player.socket.clilocmessage(1042664) # You must have the object in your backpack to use it.
	else:
		player.socket.clilocmessage(1054011) # You mark the bracelet with your inscription.
		item.settag('inscription', text)
		item.resendtooltip()

def onContextEntry(char, item, entry):
	if entry == 1:
		activate(char, item)
	elif entry == 2:
		search
	elif entry in [3, 4]:
		bind(char, item)
	elif entry == 5:
		inscribe(char, item)
	return True

def onContextCheckVisible(player, object, tag):
	if tag == 1:
		if bound(object):
			return True
	if tag == 2:
		if bound(object):
			return True
	if tag == 3:
		if not bound(object):
			return True
	if tag == 4:
		if bound(object):
			return True
	if tag == 5:
		return True

	return False

def onShowTooltip(char, item, tooltip):
	inscription = '\t '
	if item.hastag('inscription'):
		inscription = ' :\t' + item.gettag('inscription')
	tooltip.add(1054000, "%s%s" % (str(charges(item)), unicode(inscription) )) # a bracelet of binding : ~1_val~ ~2_val~
