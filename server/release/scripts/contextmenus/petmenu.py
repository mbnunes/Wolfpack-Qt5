#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: Correa                         #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#


def onContextEntry( char, target, tag  ):
    ai = target.aiengine()
    if not ai:
        char.socket.sysmessage("Outch, a brainless pet!")
        
    if ( tag == 1 ): # Command: Kill
        ai.onSpeechInput( char, "kill" )
    elif ( tag == 2 ): # Command: Follow
        ai.onSpeechInput( char, "follow" )
    elif ( tag == 3 ): # Command: Guard
        ai.onSpeechInput( char, "guard" )
    elif ( tag == 4 ): # Command: Stop
        ai.onSpeechInput( char, "stop" )
    elif ( tag == 5 ): # Command: Stay
        ai.onSpeechInput( char, "stay" )
    elif ( tag == 6 ): # Add Friend
        ai.onSpeechInput( char, "add friend" )
    elif ( tag == 7 ): # Transfer
        ai.onSpeechInput( char, "transfer" )
    elif ( tag == 8 ): # Release
        ai.onSpeechInput( char, "release" )

    return 1


        
