
import wolfpack
from wolfpack import console
from wolfpack.consts import *
from random import random, randint, choice
from wolfpack import settings
from system.debugging import DEBUG_STATS, DEBUG_SKILLS

#
# Skill Properties
#
SKILL_NAME = 1
SKILL_TITLE = 2
SKILL_DEFNAME = 3
SKILL_GAINFACTOR = 4
SKILL_STRCHANCE = 5
SKILL_DEXCHANCE = 6
SKILL_INTCHANCE = 7

GLOBAL_FACTOR = 0.5

#
# Skill data registry
#
SKILLS = {}

#
# Gain in a certain stat
# 0: Strength
# 1: Dexterity
# 2: Intelligence
#
def gainstat(char, stat):
	realstr = char.strength - char.strength2
	realdex = char.dexterity - char.dexterity2
	realint = char.intelligence - char.intelligence2
	totalstats = realstr + realdex + realint
	resendstats = 0

	# Atrohpy
	# Like the skill atrophy we have a certain chance to
	# decrease another (lock: down) stat here.
	# Only active for players
	if char.player and totalstats / char.statcap >= random():
		stats = [0, 1, 2]
		if char.strengthlock != 1 or stat == 0 or realstr <= 1:
			stats.remove(0)

		if char.dexteritylock != 1 or stat == 1 or realdex <= 1:
			stats.remove(1)

		if char.intelligencelock != 1 or stat == 2 or realint <= 1:
			stats.remove(2)

		# We found a stat to lower
		if len(stats) > 0:
			lower = choice(stats)
			if lower == 0:
				char.strength -= 1
				# NPCs hitpoints are not related to strength at all
				if char.npc:
					char.maxhitpoints -= 1
				char.updatehealth()
				if DEBUG_STATS == 1:
					char.log(LOG_TRACE, 'Character [%x] lost one point of strength [%u].\n' % (char.serial, char.strength))
			elif lower == 1:
				char.dexterity -= 1
				if DEBUG_STATS == 1:
					char.log(LOG_TRACE, 'Character [%x] lost one point of dexterity [%u].\n' % (char.serial, char.dexterity))
			elif lower == 2:
				char.intelligence -= 1
				if DEBUG_STATS == 1:
					char.log(LOG_TRACE, 'Character [%x] lost one point of intelligence [%u].\n' % (char.serial, char.intelligence))
			totalstats -= 1
			resendstats = 1

	# Increase the stat if we didn't hit our total
	# statcap yet.
	if totalstats < char.statcap:
		if stat == 0:
			char.strength += 1
			# For NPCs we manually increase the max hitpoints
			# Players are accounted for automatically
			if char.npc:
				char.maxhitpoints += 1
			if DEBUG_STATS == 1:
				char.log(LOG_TRACE, 'Character [%x] gained one point of strength [%u].\n' % (char.serial, char.strength))
		elif stat == 1:
			char.dexterity += 1
			if DEBUG_STATS == 1:
				char.log(LOG_TRACE, 'Character [%x] gained one point of dexterity [%u].\n' % (char.serial, char.dexterity))
		elif stat == 2:
			char.intelligence += 1
			if DEBUG_STATS == 1:
				char.log(LOG_TRACE, 'Character [%x] gained one point of intelligence [%u].\n' % (char.serial, char.intelligence))
		resendstats = 1

	if resendstats and char.socket:
		char.socket.resendstatus()

#
# Awards one skill point to the given character.
# Totalskill is our skill total while total cap
# is the global server skill cap.
#
def gainskill(char, skill, totalskill, totalcap):
	value = char.skill[skill] / 10.0
	lock = char.skilllock[skill]
	cap = char.skillcap[skill] / 10.0
	info = SKILLS[skill]
	
	if lock == 0 and value < cap:
		# Skills lower than 10.0% will gain 0.1% - 0.5% at once
		if value < 10.0:
			points = randint(1, 5) / 10.0
		else:
			points = 0.1

		# This will always happen if we are above the skillcap
		# But if we have some skills set to lower, they
		# are lowered even if we are below the skillcap
		if totalskill / totalcap >= random():
			for i in range(0, ALLSKILLS):
				if i != skill and char.skilllock[i] == 1 and char.skill[i] / 10.0 >= points:
					# See if there is a bonus for this skill and reducing further would go below the bonus
					try:
						if char.hastag('skillbonus_%u' % i):
							value = int(char.gettag('skillbonus_%u' % i))
							
							# Skip this skill if reducing it wouldn't work
							if char.skill[i] - int(points * 10) < value:
								continue
					except:
						pass
				
					char.skill[i] -= int(points * 10)
					totalskill -= points

					if DEBUG_SKILLS == 1:
						char.log(LOG_TRACE, 'Character [%x] lost %.01f%% %s [%.01f%%].\n' % (char.serial, points, SKILLS[i][SKILL_NAME], char.skill[i] / 10.0))

					if char.socket:
						char.socket.updateskill(i)
					break

		# We can only gain in this skill if we are still below
		# the total skill cap. Atrophy has been taken care of
		# in the if before this.
		if totalskill + points <= totalcap:
			char.skill[skill] += int(points * 10)
			totalskill += points
			if DEBUG_SKILLS == 1:
				char.log(LOG_TRACE, u'Character [%(serial)x] gained %(points).01f%% %(name)s [%(value).01f%%].\n' % { \
					'serial': char.serial, 
					'points': points,
					'name': info[SKILL_NAME], 
					'value': (char.skill[skill] / 10.0)
					})

			if char.socket:
				char.socket.updateskill(skill)

	# It's not important that we actually gained the skill
	# in order to gain stats by using it.
	if lock == 0:
		strchance = info[SKILL_STRCHANCE]
		dexchance = info[SKILL_DEXCHANCE]
		intchance = info[SKILL_INTCHANCE]
		realstr = char.strength - char.strength2
		realdex = char.dexterity - char.dexterity2
		realint = char.intelligence - char.intelligence2

		if (char.npc or char.strengthlock != 0) or realstr >= char.strengthcap:
			strchance = 0.0
		else:
			strchance /= 33.3

		if (char.npc or char.dexteritylock != 0) or realdex >= char.dexteritycap:
			dexchance = 0.0
		else:
			dexchance /= 33.3

		if (char.npc or char.intelligencelock != 0) or realint >= char.intelligencecap:
			intchance = 0.0
		else:
			intchance /= 33.3

		if strchance > random():
			gainstat(char, 0)
		elif dexchance > random():
			gainstat(char, 1)
		elif intchance > random():
			gainstat(char, 2)

#
# Called when the character gains in a skill.
#
def onSkillGain(char, skill, lower, higher, success):
	# See if we can gain at all
	# GMs don't gain skills
	if char.dead or char.gm or char.polymorph:
		return

	if not SKILLS.has_key(skill):
		char.log(LOG_ERROR, "Is using an unidentified skill: %u\n" % skill)
		return

	info = SKILLS[skill]
	skills = char.skill
	value = skills[skill] / 10.0 # Convert into float
	lower /= 10.0
	higher /= 10.0

	# NPCs only learn by using skills
	# they already have
	if char.npc and value <= 0.0:
		return

	# No gain for effortless or futile attempts
	if value >= higher:
		return

	chance = (value - lower) / (higher - lower)
	cap = char.skillcap[skill] / 10.0

	# Skills capped at zero are disabled for this character
	if cap == 0:
		return

	totalskills = 0.0
	for i in range(0, ALLSKILLS):
		# See if there is a modifier for the skill
		tagname = 'skillbonus_%u' % i
		totalskills += skills[i] / 10.0

		if char.hastag(tagname):
			try:
				value = int(char.gettag(tagname))
				totalskills -= value / 10.0
			except:
				pass

	# Calculate the GainChance
	# (RunUO has a nice approach. Doing it similar)
	totalcap = settings.getnumber("General", "SkillCap", 700)
	gainchance = (totalcap - totalskills) / totalcap # How near are we to our global skill cap
	gainchance += (cap - value) / cap # How near are we to our skill cap

	# Use the difficulty to influence the skill gain
	if success:
		gainchance += 0.5 - chance * 0.5
	else:
		gainchance += 0.2 - chance * 0.2

	gainchance /= 3.0 # The average of the three values

	# Introduce a new "Gain Factor"
	# There is also a 1% minimum chance for gain
	gainchance = gainchance * info[SKILL_GAINFACTOR])
	
	# Multiply with another gainfactor
	gainchance = gainchance * GLOBAL_FACTOR

	# Tamed creatures get a * 2 bonus for their gain.
	if char.npc and char.tamed:
		gainchance *= 2

	# Skills below 10% always gain, otherwise take the gainchance into
	# account.
	if gainchance >= random() or value < 10.0:
		gainskill(char, skill, totalskills, totalcap)

#
# Register our hook and load the skills.xml data.
#
def onLoad():
	wolfpack.registerglobal(EVENT_SKILLGAIN, "system.skillgain")

	# Load all the neccesary data from the definitions
	for i in range(0, ALLSKILLS):
		skilldef = wolfpack.getdefinition(WPDT_SKILL, str(i))

		# Load the skill information
		if skilldef:
			SKILLS[i] = {
				SKILL_GAINFACTOR: 1.0,
				SKILL_STRCHANCE: 0.0,
				SKILL_DEXCHANCE: 0.0,
				SKILL_INTCHANCE: 0.0
			}

			for j in range(0, skilldef.childcount):
				child = skilldef.getchild(j)
				if child.name == 'name':
					SKILLS[i][SKILL_NAME] = child.value
				elif child.name == 'title':
					SKILLS[i][SKILL_TITLE] = child.value
				elif child.name == 'defname':
					SKILLS[i][SKILL_DEFNAME] = child.value
				elif child.name == 'gainfactor':
					SKILLS[i][SKILL_GAINFACTOR] = float(child.value)
				elif child.name == 'strchance':
					SKILLS[i][SKILL_STRCHANCE] = float(child.value)
				elif child.name == 'dexchance':
					SKILLS[i][SKILL_DEXCHANCE] = float(child.value)
				elif child.name == 'intchance':
					SKILLS[i][SKILL_INTCHANCE] = float(child.value)
