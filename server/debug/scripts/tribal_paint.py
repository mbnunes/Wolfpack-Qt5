
from wolfpack.consts import LAYER_HELM
from magic import necromancy

duration = 1000 * 60 * 60 * 24 * 7 # one week

def onUse( char, item ):
	if not item.getoutmostchar() == char:
		char.socket.clilocmessage( 1042001 ) # That must be in your pack for you to use it.
		return True

	if char.incognito:
		char.socket.clilocmessage( 501698 ) # You cannot disguise yourself while incognitoed.
	elif char.polymorph:
		char.socket.clilocmessage( 501699 ) # You cannot disguise yourself while polymorphed.
	elif necromancy.transformed(char):
		char.socket.clilocmessage( 501699 ) # You cannot disguise yourself while polymorphed.
	# wearing an orkish kin mask
	elif char.itemonlayer(LAYER_HELM) and char.itemonlayer(LAYER_HELM).baseid == '141b':
		char.socket.clilocmessage( 501605 ) # You are already disguised.
	else:
		if char.gender:
			char.id = 184
		else:
			char.id = 183
		char.update()
		char.addtimer( duration, paint_expire, [], True, False, 'tribal_paint', dispel_callback  )
		char.socket.clilocmessage( 1042537 ) # You now bear the markings of the savage tribe.  Your body paint will last about a week or you can remove it with an oil cloth.
		item.delete()
	return True

def paint_expire( char, args ):
	char.id = char.orgid
	char.update()

def dispel_callback(char, args, source, dispelargs):
	char.id = char.orgid
	char.update()
