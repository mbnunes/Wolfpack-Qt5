
#
# Answer the banker context menu.
#
def onContextEntry(char, target, tag):
	if tag == 1:
		if char.dead:
			target.say(500895) # That sounded spooky.
			return 1

		if char.iscriminal():
			target.say(500378) # Thou art a criminal and cannot access thy bank box.
			return 1

		bank = char.getbankbox()
		target.turnto(char)
		target.say(tr("Here is your bank box, %s.") % char.name)
		char.socket.sendcontainer(bank)

	elif tag == 2:
		target.turnto(char)
		bank = char.getbankbox()
		amount = bank.countresource(0xEED, 0x0)
		if not amount:
			target.say(tr("Alas you don't have any money in your bank."))
		else:
			target.say(tr("You have %i gold in your bank.") % amount)

	return 1
