#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: DarkStorm                      #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#
# Speech Script for Bankers                                     #
#===============================================================#

from wolfpack import tr
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
        #Thou must tell me how much thou wishest to withdraw
        speaker.socket.clilocmessage( 0x7A29B, "", 0x3b2, 3, listener )
        return

      # Withraw
      else:
        # Check if the player has enough gold in his bank
        bank = speaker.getbankbox()
        gold = 0

        if bank:
          gold = bank.countresource( 0xEED, 0x0 )

        if amount > gold:
          # Thou dost not have sufficient funds in thy account to withdraw that much.
          speaker.socket.clilocmessage( 0x7A29E, "", 0x3b2, 3, listener )
          return

        else:
          # We have enough money, so let's withdraw it
          # Thou hast withdrawn gold from thy account.
          speaker.socket.clilocmessage( 0xF6955, "", 0x3b2, 3, listener )
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
      listener.turnto(speaker)
      bank = speaker.getbankbox()
      amount = bank.countresource(0xeed, 0x0)
      if not amount:
        listener.say(tr("Alas you don't have any money in your bank."))
      else:
        listener.say(tr("You have %i gold in your bank.") % amount)
      return 1

    # bank
    elif keyword == 0x2:
      if speaker.dead:
        listener.say(500895) # That sounded spooky.
        return 1

      if speaker.iscriminal():
        listener.say(500378) # Thou art a criminal and cannot access thy bank box.
        return 1

      bank = speaker.getbankbox()
      listener.turnto(speaker)
      listener.say(tr("Here is your bank box, %s.") % speaker.name)
      speaker.socket.sendcontainer(bank)
      return 1

    # check
    elif keyword == 0x3:
      result = amountre.search( text )
      amount = 0

      if result:
        amount = int( result.group( 1 ) )

      # Invalid Withdraw Amount
      if amount < 5000:
        speaker.socket.clilocmessage( 0x7A29F, "", 0x3b2, 3, listener )
        return

      # Withraw
      else:
        # Check if the player has enough gold in his bank
        bank = speaker.getbankbox()
        gold = 0

        if bank:
          gold = bank.countresource( 0xEED, 0x0 )

        if amount > gold:
          # Ah, art thou trying to fool me? Thou hast not so much gold!
          speaker.socket.clilocmessage( 0x7A29C, "", 0x3b2, 3, listener )
          return
        else:
          # We have enough money, so let's withdraw it
          # Into your bank box I have placed a check in the amount of:
          total = str( amount )
          speaker.socket.clilocmessage( 0xFE8F1, "", 0x3b2, 3, listener, " %s" %total )
          bank.useresource( amount, 0xEED, 0x0 )
          check = wolfpack.additem( "bank_check" )
          check.settag( 'value', amount )
          check.container = bank
          check.update()
      break

  return 1

# An item has been dropped on us
def onDropOnChar( char, item ):
	char.socket.clilocmessage( 0x7A2A4, "", 0x3b2, 3, listener )
	#backpack = char.getbackpack()
	#item.container = backpack
	return 0
