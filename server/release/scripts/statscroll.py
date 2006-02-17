import wolfpack.gumps
import wolfpack.console
from wolfpack.consts import LOG_WARNING
from wolfpack import tr

def onCreate(object, id):
	if not object.hastag("value"):
		wolfpack.console.log(LOG_WARNING, "Stat Scroll of Power without value has been created and was automatically deleted. Please check your scripts.")
		object.delete()

def onUse(char, item):
	doUse(char, item, True)
	return True

def doUse(char, item, firstStage):
	if item.getoutmostchar() != char:
		char.socket.clilocmessage( 1042001 ) # That must be in your pack for you to use it.
		return True
	try:
		value = int(item.gettag("value"))
	except:
		char.socket.sysmessage("tt")
		return False

	if char.statcap >= value:
		char.socket.clilocmessage( 1049510 ) # Your stats are too high for this power scroll.
	else:
		if firstStage:
			# close statcapscroll gump
			# close powerscroll gump
			InternalGump(char, item)
		else:
			char.socket.clilocmessage( 1049512 ) # You feel a surge of magic as the scroll enhances your powers!

			char.statcap = value

			#Effects.SendLocationParticles( EffectItem.Create( from.Location, from.Map, EffectItem.DefaultDuration ), 0, 0, 0, 0, 0, 5060, 0 );

			char.pos.soundeffect(0x243)

			#Effects.SendMovingParticles( new Entity( Serial.Zero, new Point3D( from.X - 6, from.Y - 6, from.Z + 15 ), from.Map ), from, 0x36D4, 7, 0, false, true, 0x497, 0, 9502, 1, 0, (EffectLayer)255, 0x100 );
			#Effects.SendMovingParticles( new Entity( Serial.Zero, new Point3D( from.X - 4, from.Y - 6, from.Z + 15 ), from.Map ), from, 0x36D4, 7, 0, false, true, 0x497, 0, 9502, 1, 0, (EffectLayer)255, 0x100 );
			#Effects.SendMovingParticles( new Entity( Serial.Zero, new Point3D( from.X - 6, from.Y - 4, from.Z + 15 ), from.Map ), from, 0x36D4, 7, 0, false, true, 0x497, 0, 9502, 1, 0, (EffectLayer)255, 0x100 );

			char.effect(0x375A, 35, 90)

			item.delete()
	return True

def InternalGump(char, item):
	gump = wolfpack.gumps.cGump()
	gump.setCallback(callback)
	gump.setArgs([item.serial])

	gump.addResizeGump(25, 10, 5054, 420, 200)
	#AddBackground( 25, 10, 420, 200, 5054 );

	gump.addTiledGump(33, 20, 401, 181, 2624)
	gump.addCheckerTrans(33, 20, 401, 181)
	#AddImageTiled( 33, 20, 401, 181, 2624 );
	#AddAlphaRegion( 33, 20, 401, 181 );

	gump.addXmfHtmlGump(40, 48, 387, 100, 1049469, 1,1 ) # Using a scroll increases the maximum amount of a specific skill or your maximum statistics.
								# When used, the effect is not immediately seen without a gain of points with that skill or statistics.
								# You can view your maximum skill values in your skills window.
								# You can view your maximum statistic value in your statistics window.


	gump.addXmfHtmlGump(125, 148, 200, 20, 1049478, 0,0, 0xFFFFFF ) # Do you wish to use this scroll?

	gump.addButton(100, 172, 4005, 4007, 1)
	gump.addXmfHtmlGump(135, 172, 120, 20, 1046362, 0,0, 0xFFFFFF ) # Yes

	gump.addButton(275, 172, 4005, 4007, 0)
	gump.addXmfHtmlGump(310, 172, 120, 20, 1046363, 0,0, 0xFFFFFF ) # No

	try:
		value = int(item.gettag("value"))
	except:
		return False

	if value == 230:
		gump.addXmfHtmlGump(40, 20, 260, 20, 1049458, 0,0, 0xFFFFFF ) # Wonderous Scroll (+5 Maximum Stats):
	elif value == 235:
		gump.addXmfHtmlGump(40, 20, 260, 20, 1049459, 0,0, 0xFFFFFF ) # Exalted Scroll (+10 Maximum Stats):
	elif value == 240:
		gump.addXmfHtmlGump(40, 20, 260, 20, 1049460, 0,0, 0xFFFFFF ) # Mythical Scroll (+15 Maximum Stats):
	elif value == 245:
		gump.addXmfHtmlGump(40, 20, 260, 20, 1049461, 0,0, 0xFFFFFF ) # Legendary Scroll (+20 Maximum Stats):
	elif value == 250:
		gump.addXmfHtmlGump(40, 20, 260, 20, 1049462, 0,0, 0xFFFFFF ) # Ultimate Scroll (+25 Maximum Stats):
	else:
		pre = ""
		if value - 225 >= 0:
			pre = "+"
		name = 	tr("<basefont color=#FFFFFF>Power Scroll (%s%s Maximum Stats):</basefont>" % (pre, str(value - 225)))

		gump.addHtmlGump(40, 20, 260, 20, name, 0, 0)

	gump.addXmfHtmlGump(310, 20, 120, 20, 1038019, 0,0, 0xFFFFFF ) # Power
	gump.send(char.socket)

def callback(char, args, response):
	if response.button == 0:
		return False
	item = wolfpack.finditem(args[0])
	if item:
		doUse(char, item, False)
	return

def onShowTooltip(char, item, tooltip):
	if not item.hastag('value'):
		return False
	tooltip.reset()
	value = int(item.gettag('value'))
	if value == 230:
		tooltip.add( 1049463, "#1049476" ) # a wonderous scroll of ~1_type~ (+5 Maximum Stats)
	elif value == 235:
		tooltip.add( 1049464, "#1049476" ) # an exalted scroll of ~1_type~ (+10 Maximum Stats)
	elif value == 240:
		tooltip.add( 1049465, "#1049476" ) # a mythical scroll of ~1_type~ (+15 Maximum Stats)
	elif value == 245:
		tooltip.add( 1049466, "#1049476" ) # a legendary scroll of ~1_type~ (+20 Maximum Stats)
	elif value == 250:
		tooltip.add( 1049467, "#1049476" ) # an ultimate scroll of ~1_type~ (+25 Maximum Stats)
	else:
		pre = ""
		if value - 225 >= 0:
			pre = "+"
		tooltip.add(1042971, tr("a scroll of power (%s%s Maximum Stats)" % (pre, str(value - 225))))
