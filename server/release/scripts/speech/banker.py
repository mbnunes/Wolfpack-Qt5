#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: DarkStorm                      #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#
# Speech Script for Bankers                                     #
#===============================================================#

from wolfpack.speech import addKeyword,setCatchAll
from wolfpack.consts import *
import wolfpack
import re

#0x0000 "*withdraw*"
#0x0000 "*withdrawl*"
#0x0001 "*balance*"
#0x0001 "*statement*"
#0x0002 "*bank*"
#0x0003 "*check*"

amountre = re.compile( '(\d+)' )

def onLoad():
	setCatchAll( 'speech_banker', 0 )
	addKeyword( 'speech_banker', 0x0 )
	addKeyword( 'speech_banker', 0x1 )
	addKeyword( 'speech_banker', 0x2 )
	addKeyword( 'speech_banker', 0x3 )	

def onSpeech( listener, speaker, text, keywords ):
	for keyword in keywords:

		# withdraw
		if keyword == 0x0:
			# Search for the amount we want to withdraw (should be the only digits in there)
			result = amountre.search( text )
			amount = 0

			if result:
				amount = int( result.group( 1 ) )

			# Invalid Withdraw Amount
			if not amount:
				listener.say( "How much do you want to withdraw?" )

			# Withraw
			else:
				# Check if the player has enough gold on his bank
				bank = speaker.getbankbox()
				gold = 0

				if bank:
					gold = bank.countresource( 0xEED, 0x0 )

				if amount > gold:
					listener.say( "You don't have enough money in your bank box." )

				else:
					# We have enough money, so let's withdraw it
					listener.say( "Here are your %i gold." % amount )
					bank.useresource( amount, 0xEED, 0x0 )
					backpack = speaker.getbackpack()

					while amount > 0:
						item = wolfpack.additem( "eed" )
						item.amount = min( [ amount, 65535 ] )
						item.container = backpack

						amount -= min( [ amount, 65535 ] )				

					speaker.soundeffect( 0x37, 0 )

			break

		# balance (count all gold)
		elif keyword == 0x1:
			bank = speaker.getbankbox()
			if bank:
				amount = bank.countresource( 0xEED, 0x0 )
				if not amount: 
					listener.say( "Alas you don't have any money on your bank." )
				else:
					listener.say( "You have %i gold on your bank." % amount )
			break

		# bank
		elif keyword == 0x2:
			bank = speaker.getbankbox()
			
			if bank:
				listener.say( "Here is your bank box, %s." % speaker.name )
				speaker.socket.sendcontainer( bank )

			break

		# check
		elif keyword == 0x3:
			result = amountre.search( text )
			amount = 0

			if result:
				amount = int( result.group( 1 ) )

			# Invalid Withdraw Amount
			if amount < 5000:
				listener.say( "A check at least needs to have 5000 gold value." )

			# Withraw
			else:
				# Check if the player has enough gold on his bank
				bank = speaker.getbankbox()
				gold = 0

				if bank:
					gold = bank.countresource( 0xEED, 0x0 )

				if amount > gold:
					listener.say( "You don't have enough money in your bank box." )

				else:
					# We have enough money, so let's withdraw it
					listener.say( "I've put a check for %i gold on your bank." % amount )
					bank.useresource( amount, 0xEED, 0x0 )
					check = wolfpack.additem( "bank_check" )
					check.settag( 'value', amount )
					check.container = bank
					check.update()
			break
			
	return 1
