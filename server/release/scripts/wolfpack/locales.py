
# Import Locale Constants Here
from wolfpack.consts import SYSLOCALE, LOCALE_EN, LOCALE_DE

"""
This function's purpose is to allow people to edit common messages for their
own languages. The message meanings should match up with others.
This will be expanded in time.
"""
def localemsg( msg, locale=SYSLOCALE ):
	# Locale Index
	LOCALE_MESSAGE = {
		LOCALE_EN: {
			0: "Sorry but this feature is not implemented yet!",
			1: "Where do you want to create the item ",
			2: "Where do you want to create the NPC ",
			3: "The item must be in your backpack to use it!",
			4: "You can not pick cotton here yet.",
			5: "You reach down and pick some cotton.",
			6: "You are unable to reach this!",
			7: "What do you want to use this on?",
			8: "You must target an item.",
			9: "You must target a character.",
			10: "This needs to be in your belongings to use it.",
			11: "You cannot dye this!",
			12: "That is an invalid color.",
			13: "You have to choose a hair color.",
			14: "You fail to create the sextant",
			15: " and break the parts.",
			16: ".",
			17: "You put the sextant into your backpack",
			18: "Please select the object you wish to dye."
		},
		LOCALE_DE: {
			0: "",
			1: "",
			2: "",
			3: ""
		}
	}
	if LOCALE_MESSAGE[locale][msg]:
		# Return the message
		return unicode( LOCALE_MESSAGE[locale][msg] )
	else:
		return unicode( "ERROR: BAD LOCALE DEFINED" )
