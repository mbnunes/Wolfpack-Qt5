#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: khpae                          #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

from wolfpack.consts import *
import wolfpack
import wolfpack.time
import wolfpack.utilities
import environment
import skills
from wolfpack import properties
import system.slayer

MUSICIANSHIP_DELAY = 1000
MINSKILL = 0
MAXSKILL = 1000
# instrument id : good sound ( poor sound = good sound + 1 )
sounds = {
		0x0e9c: 0x0038,
		0x0e9d: 0x0052,
		0x0e9e: 0x0052,
		0x0eb1: 0x0045,
		0x0eb2: 0x0045,
		0x0eb3: 0x004c,
		0x0eb4: 0x004c,
		0x2805: 0x003d
	}

# reduce the remaining uses
def wearout( player, item ):
	# We do not allow "invulnerable" tools.
	if not item.hastag('remaining_uses'):
		player.socket.clilocmessage(502079) # The instrument played its last tune.
		item.delete()
		return False

	uses = int(item.gettag('remaining_uses'))
	if uses <= 1:
		player.socket.clilocmessage(502079) # The instrument played its last tune.
		item.delete()
		return False
	else:
		item.settag('remaining_uses', uses - 1)
		item.resendtooltip()
	return True

def onUse( char, item ):
	if not wolfpack.utilities.isinstrument( item ):
		return False

	socket = char.socket

	if socket.hastag( 'skill_delay' ):
		if wolfpack.time.currenttime() < socket.gettag( 'skill_delay' ):
			socket.clilocmessage( 500118, "", 0x3b2, 3 )
			return True
		else:
			socket.deltag( 'skill_delay' )

	if skills.skilltable[ MUSICIANSHIP ][ skills.UNHIDE ] and char.hidden:
		char.reveal()

	success = char.checkskill( MUSICIANSHIP, MINSKILL, MAXSKILL )

	char.socket.settag( 'skill_delay', int( wolfpack.time.currenttime() + MUSICIANSHIP_DELAY ) )

	return play_instrument( char, item, success )

# play sound acording to the instrument good/poor
def play_instrument( char, item, success ):
	# this check is needed ? other bard skills will use this function but..
	# check will not be needed - will be removed
	if not item.id in sounds.keys():
		return 0
	# when we use bard skill, if we have more than one instrument,
	# the 'last used' item will be used
	# set last use item for musicianship
	item.settag( 'last_musicianship_use', wolfpack.time.currenttime() )
	char.socket.settag( 'instrument', item.serial )
	if success:
		wearout( char, item )
		sound = sounds[ item.id ]
	else:
		sound = sounds[ item.id ] + 1
	char.soundeffect( sound )
	return True

# The base range of all bard abilities is 5 tiles, with each 15 points of skill in the ability being used increasing this range by one tile.
def bard_range( char ):
	musi_range = 8 + char.skill[ MUSICIANSHIP ] / 150
	return musi_range

def IsPoisonImmune( char ):
	poison_immunity = char.getintproperty('poison_immunity', -1)
	return poison_immunity != -1

def GetDifficultyFor( player, instrument, target ):
	# Difficulty TODO: Add another 100 points for each of the following abilities:
	#	- Radiation or Aura Damage (Heat, Cold etc.)
	#	- Summoning Undead
	#
	totalskills = 0
	val = target.hitpoints + target.stamina + target.mana
	skills = target.skill
	for i in range(0, ALLSKILLS):
		totalskills += skills[i] / 100.0

	if val > 700:
		val = 700 + ((val - 700) / 3.66667)

	# Target is Magery Creature
	if target.ai == "Monster_Mage" and target.char.skill[MAGERY] > 50:
		val += 100

	#if IsFireBreathingCreature( bc ):
	#	val += 100

	if IsPoisonImmune( target ):
		val += 100

	if target.id == 317:
		val += 100

	val += (target.getintproperty('hit_poison_level', 0) + 1) * 20

	val /= 10

	#if ( bc != null && bc.IsParagon )
	#	val += 40.0;

	if instrument.hastag('exceptional'):
		val -= 5.0 # 10%

	slayer = properties.fromitem(instrument, SLAYER)
	if slayer != '':
		slayers = slayer.split(',')
		for slayer in slayers:
			slayer = system.slayer.findEntry(slayer)
			if slayer:
				if slayer.slays(target):
					val -= 10.0 # 20%
				elif slayer.group.opposition.super.slays(target): # not sure if this is correct
					val += 10.0 # -20%
	return val

def onShowTooltip(viewer, object, tooltip):
	slayer = properties.fromitem(object, SLAYER)
	if slayer != '':
		slayers = slayer.split(',')
		for slayer in slayers:
			slayer = system.slayer.findEntry(slayer)
			if slayer:
				tooltip.add(slayer.name, '')
