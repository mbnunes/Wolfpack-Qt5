#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: Correa                         #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification:                         #
#################################################################


#
# This is just a sample implementation, still needs to be filled in properly
#

def onContextMenuCheckVisible( char, target, tag ):
    if ( target.skill[tag] >= 300 ):
        return 1 # visible
    else:
        return 0 # not visible

def onContextMenuCheckEnabled( char, target, tag ):
    if ( char.skill[tag] >= 300 ):
        return 0 # disabled
    else: 
        return 1 #enabled
    
