#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: codex                          #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

from wolfpack.consts import *
import wolfpack
from wolfpack.utilities import *
import re

# Register as a global script
def onLoad():
	wolfpack.registerglobal( EVENT_SHOWTOOLTIP, "tooltip" )

def onShowTooltip( sender, target, tooltip ):  
  if target.isitem():
    name = target.getname()

    if target.id == 0x2006 and len(target.name) > 0:
      notoriety = 0x00

      if target.hastag('notoriety'):
        notoriety = int(target.gettag('notoriety'))

      color = '#FFFFFF'

      # 0x01 Blue, 0x02 Green, 0x03 Grey, 0x05 Orange, 0x06 Red
      colors = {
        0x01 : '#00FFFF',
        0x02 : '#10d010',
        0x03 : '#d0d0d0',
        0x04 : '#d0d0d0',
        0x05 : '#ff9900',
        0x06 : '#d01010'
      }

      if not colors.has_key(notoriety):      
        tooltip.add(1050045, " \tThe Remains Of \t" + target.name)
      else:
        tooltip.add(1050045, '<basefont color="%s">' % colors[notoriety] + " \tThe Remains Of \t<h3>" + target.name)

    elif isarmor ( target ):
    	armor( target, tooltip )
    
    elif ishat( target ):
    	hat( target, tooltip )
    
    elif isshield( target ):
    	shield( target, tooltip )
           
    elif isweapon( target ):
    	weapon( target, tooltip )
                   
    else:
      if target.name == '#' or target.name == '':
        labelname = '#' + str(0xf9060 + target.id)
      else:
        labelname = target.name

      if target.amount > 1:
        tooltip.add(1050039, str(target.amount) + "\t" + labelname)
      else:
        tooltip.add(1050039, " \t" + labelname)

      if target.visible == 0 and sender.gm:
          tooltip.add(3000507, "")
      
      modifiers( target, tooltip )
      
      # If the character is a gm and the targetted item has a lock, display the lock id
      if sender.gm and 'lock' in target.events:
      	lock = 'None'
        
      	if target.hastag('lock'):
      		lock = str(target.gettag('lock'))
        
      	tooltip.add(1050045, " \t" + "Lock: " + lock + "\t ")
      
      if isspellbook( target ):
      	tooltip.add( 1042886, str( target.spellscount() ) )
  else:
    tooltip.add(1050045, " \t%s\t " % target.name)
  
    # Tame = 502006
    if target.tamed:
      tooltip.add(502006, "")

    # Seems to break it ??
    #if target.invulnerable and sender.gm:
    #  tooltip.add(1050045, " \t(invulnerable)\t ")
    #  tooltip.add(502006, "")

    if target.frozen:
      tooltip.add(1050045, " \t(frozen)\t ")

    if target.iscriminal():
      tooltip.add(1050045, " \t(criminal)\t ")

  tooltip.send(sender)
  return 1

def armor( target, tooltip ):
	name = target.getname()
	tooltip.add( 1050045, " \t" + name + "\t " ) #prefix name suffix
	modifiers( target, tooltip )
	tooltip.add( 1060639, str( target.health ) + "\t" + str( target.maxhealth ) )           #Durability

def container( target, tooltip ):
	tooltip.add( 1050045, " \tBag\t " )
	tooltip.add( 1050044, "1000\t10000" )     #$count items, $weight stones

def shield( target, tooltip ):
	name = target.getname()
	tooltip.add( 1050045, " \t" + name + "\t " )
	modifiers( target, tooltip )
	tooltip.add( 1060639, str( target.health ) + "\t" + str( target.maxhealth ) )  #Durability

def hat( target, tooltip ):
	name = target.getname()
	tooltip.add( 1050045, " \t" + name + "\t " )
	modifiers( target, tooltip )
	tooltip.add( 1060639, str( target.health ) + "\t"  + str( target.maxhealth) )    #Durability

def weapon( target, tooltip ):
	name = target.getname()
	tooltip.add( 1050045, " \t" + name + "\t " )
	modifiers( target, tooltip )
	if ( target.twohanded ):
		tooltip.add( 1061171, "" ) # two-handed
	else:
		tooltip.add( 1061824, "" ) # One-handed
	if ( target.type ) == 1001 or ( target.type ) == 1002:
		tooltip.add( 1061172, "" ) # Weapon Skill: Swordsmanship
	if ( target.type ) == 1003 or ( target.type ) == 1004:
		tooltip.add( 1061173, "" ) # Weapon Skill: Mace Fighting
	if ( target.type ) == 1005:
		tooltip.add( 1061174, "" ) #Weapon Skill: Fencing
	if ( target.type ) == 1006 or ( target.type ) == 1007:
		tooltip.add( 1061175, "" ) # Weapon Skill: Archery
	tooltip.add( 1060639, str( target.health ) + "\t" + str( target.maxhealth ))

def modifiers( target, tooltip ):
	modifiers = {
	"blessed" 	: [1038021,0],
	"req_str" 	: [1061170,1],
	"aos_req_str"   : [1061170,1],
	"boni_dex"	: [1060409,1],
	"aos_boni_dex"	: [1060409,1],
	"boni_int"	: [1060432,1],
	"aos_boni_int"	: [1060432,1],
	"boni_str"	: [1060485,1],
	"aos_boni_str"	: [1060485,1],
	"res_physical" 	: [1060448,1],
	"res_fire"	: [1060447,1],
	"res_cold"	: [1060445,1],
	"res_poison"	: [1060449,1],
	"res_energy"	: [1060446,1],
	"dmg_physical" 	: [1060403,1],
	"dmg_fire"	: [1060405,1],
	"dmg_cold"	: [1060404,1],
	"dmg_poison"	: [1060406,1],
	"dmg_energy"	: [1060407,1],
	}

	for tagname in modifiers.keys():
		if target.hastag( tagname ):
			params = ""
			if modifiers[ tagname ][1]:
				params = str( target.gettag( tagname ) )
			tooltip.add( modifiers[ tagname ][0], params )
