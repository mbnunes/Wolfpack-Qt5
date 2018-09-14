
import wolfpack
from wolfpack import tr
import guilds.stone
from commands.jail import jailPlayer
import wolfpack.settings
from quests.players import openquestplayer
from wolfpack.utilities import staffmessage
from wolfpack.consts import ALLSKILLS, LOG_ERROR
from skills import SKILLS, SKILL_RANKS
#from wolfpack.utilities import isyoung

#enable_young = int( wolfpack.settings.getbool("General", "Enable Young Status", True, True) )

#def onCreate(object, id):
#	if enable_young:
#		object.addscript("system.young")

#
# Called after client logged into the game and was NOT logged in before
#
def onLogin( player ):
	socket = player.socket
	socket.sysmessage( tr("Welcome to %s") % ( wolfpack.serverversion() )  )
	socket.sysmessage( tr("Report Bugs: http://bugs.wpdev.org/") )
	player.hidden = False

	# send to jail if account is jailed
	if player.account.flags & 0x80:
		if not player.jailed:
			jailPlayer( player, player )
	return False

#
# Called when character has been in game before. Otherwise see onLogin event.
#
def onConnect( player, reconnecting ):
	socket = player.socket
	if reconnecting:
		socket.sysmessage( tr("Reconnecting.") )
	player.update()
	socket.resendplayer()

	# send to jail if account is jailed
	if player.account.flags & 0x80:
		if not player.jailed:
			jailPlayer( player, player )

#
# Called when the character logs out
#
def onLogout( player ):
	player.removefromview()
	player.hidden = 1
	player.update()

#
# Called when the character receives damage. Returns the damage value.
#
def onDamage(char, type, amount, source):
#	if isyoung(char):
#		return 0

	socket = char.socket

	if source and source.player:
		slip_amount = 18
	else:
		slip_amount = 25

	if socket and amount > slip_amount and socket.hastag('bandage_slipped'):
		socket.settag('bandage_slipped', int(socket.gettag('bandage_slipped')) + 1)
		socket.clilocmessage(500961) # Your fingers slip!

	return amount

#
# Called when player clicks Guild Button in the paperdoll.
#
def onGuildButton(player):
	if not player.guild:
		player.socket.sysmessage(tr('You aren\'t in a guild.'))
	else:
		guilds.stone.mainMenu(player, player.guild)

#
# Called when player clicks Quest Button in the paperdoll.
#
def onQuestButton(player):
	openquestplayer(player)

#
# Called when character is saying something.
#
def onTalk(player, type, color, font, text, lang):
	wolfpack.utilities.staffmessage(player, text, 0)

#
# Returns the name of the character
#
# char: Char to get the name from
# player: Char requesting the name
#
def getName( char, player ):
	return char.name

#
# Returns the title of the character
#
# char: Char to get the title from
# player: Char requesting the title
#
def getTitle( char, player ):
	return char.title

#
# Returns the skill title of the character
#
# char: Char to get the title from
# player: Char requesting the title
#
def getSkillTitle( char, player ):
	skillTitle = ""
	if (not char.gm and not hideSkillTitle(char, player)): # do not show if the char requesting the title from is gm or hides his title
		skill = 0
		skillValue = 0
		for skillid in range( 0, ALLSKILLS ):
			if (char.skill[skillid] > skillValue):
				skillValue = char.skill[ skillid ]
				skill = skillid
		
		title = max(1, (skillValue - 300 ) / 100 )
		
		if ( len(SKILL_RANKS) < title ):
			wolfpack.console.log( LOG_ERROR, tr( "Invalid skill rank information.\n" ) )
			return skillTitle;

		# Skill not found
		if (skill > ALLSKILLS):
			wolfpack.console.log( LOG_ERROR, tr( "Skill id out of range: %u.\n") %skill)
			return skillTitle;
			
		skillTitle = SKILL_RANKS[title] + " " + SKILLS[skill][2]
	return skillTitle

# Skill title is hidden when char has tag 'hideskilltitle' and the requester is NOT a gm
def hideSkillTitle(char, requester):
	return char.hastag('hideskilltitle') and not requester.gm

#
# Called when opening the paperdoll. Sets the name and title in the paperdoll.
#
def onShowPaperdollName( char, player ):
	title = getTitle(char, player)
	name = getName(char, player)
	if ( ( getTitle(char, player) == "") and wolfpack.settings.getbool("General", "ShowSkillTitles", True) and player.hastag("showskilltitles") ):
		title = getSkillTitle(char, player)
	
	if title != "":
		title = ", " + title
	return name + title

# TODO: REPUTATION_TITLES (need something like getlist() for definitions)
