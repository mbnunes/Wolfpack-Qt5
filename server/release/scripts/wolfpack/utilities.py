#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: DarkStorm                      #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#
# Utility functions used in Python scripts                      #
#===============================================================#

# Shows Fizzle Animation + Sound
def fizzle( char ):
	char.effect( 0x3735, 1, 30 )
	char.soundeffect( 0x5c )

# STAT MODIFIER (Bless, Curse, etc.)

def statmodifier_dispel( char, args, source, dispelargs ):
	# Normally reduce "if not 'silent' in dispelargs"
	char.str -= args[0]
	char.dex -= args[1]
	char.int -= args[2]

	if not "silent" in dispelargs:
		# If Str changed, resend to others
		if args[0] != 0:
			char.updatehealth()
		char.updatestats()

	return

def statmodifier_expire( char, args ):
	# The stat modifier normally expired, so just reduce the stats and resend
	char.str -= args[0]
	char.dex -= args[1]
	char.int -= args[2]

	# If Str changed, resend to others
	if args[0] != 0:
		char.updatehealth()
	char.updatestats()

	return

def statmodifier( char, time, mStr, mDex, mInt ):
	# Dispel any old stat modifiers on this character
	# And readd a new one (remove the old ones silently)
	char.dispel( char, 0, "statmodifier", [ "silent" ] )

	char.str += mStr
	char.dex += mDex
	char.int += mInt

	if mStr != 0:
		char.updatehealth()

	char.updatestats()

	# Save the values in a tempeffect to remove them later
	char.addtimer( time, "wolfpack.utilities.statmodifier_expire", [ mStr, mDex, mInt ], 1, 1, "statmodifier", "wolfpack.utilities.statmodifier_dispel" )
