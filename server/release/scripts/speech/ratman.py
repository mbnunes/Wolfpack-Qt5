
import random

SAY_COLOR = 149
SAY_SOUND = 438

# List of syllables inspired by RunUO
SYLLABLES = [
	"skrit", "ch", "ch", "it", "ti", "it", "ti", "ak", "ek", "ik", 
	"ok", "uk", "yk", "ka", "ke", "ki", "ko", "ku", "ky", "at", "et", 
	"it", "ot", "ut", "yt", "cha", "che", "chi", "cho", "chu", "chy", 
	"ach", "ech", "ich", "och", "uch", "ych", "att", "ett", "itt", 
	"ott", "utt", "ytt", "tat", "tet", "tit", "tot", "tut", "tyt", 
	"tta", "tte", "tti", "tto", "ttu", "tty", "tak", "tek", "tik", 
	"tok", "tuk", "tyk", "ack", "eck", "ick", "ock", "uck", "yck", 
	"cka", "cke", "cki", "cko", "cku", "cky", "rak", "rek", "rik", 
	"rok", "ruk", "ryk", "tcha", "tche", "tchi", "tcho", "tchu", "tchy", 
	"rach", "rech", "rich", "roch", "ruch", "rych", "rrap", "rrep", 
	"rrip", "rrop", "rrup", "rryp", "ccka", "ccke", "ccki", "ccko", 
	"ccku", "ccky"
]

#
# Build a random word with a given number of syllables
#
def buildword(syllables):
	result = ''
	for i in range(0, syllables):
		result += random.choice(SYLLABLES)
	return result

#
# Delimit a sentence
#
def delimiter(end):
	# End the sentence only if it's the real end or
	# with a chance of 1/3
	if end:
		if random.random() >= 0.20:
			return '.'
		else:
			return '!'
	# Just end the word
	else:
		if random.random() <= 0.33:
			if random.random() >= 0.20:
				return '. '
			else:
				return '! '
		else:
			return ' '

#
# Build a random sentence with a given number of words
# 
def buildsentence(words):
	sentence = ''
	sentencestart = True
	
	for i in range(0, words):
		# 2/3 long words, 1/3 shorts
		if random.random() >= 0.33:
			word = buildword(random.randint(1, 5))
		else:
			word = buildword(random.randint(1, 3))
			
		# Captalize if beginning of sentence
		if sentencestart:
			sentence += word.capitalize()
		else:
			sentence += word
			
		# Add a delimiter
		char = delimiter(i + 1 == words)
		sentence += char		
		sentencestart = char[0] != ' '

	return sentence

def onDamage(char, type, amount, source):
	# 10% chance to talk
	if random.random() >= 0.10:
		return amount

	if char.health - amount > 0:	
		if amount < 5:
			sentence = random.choice([ "Ouch!", "Me not hurt bad!", "Thou fight bad.", "Thy blows soft!", "You bad with weapon!" ])				
		else:
			sentence = random.choice([ "Ouch! Me hurt!", "No, kill me not!", "Me hurt!", "Away with thee!", "Oof! That hurt!", "Aaah! That hurt...", "Good blow!" ])
	else:
		sentence = random.choice(["Revenge!", "NOOooo!", "I... I...", "Me no die!", "Me die!", "Must... not die...", "Oooh, me hurt...", "Me dying?"])
					
	# Say english sentence after constructed one
	char.say( buildsentence( random.randint(2, 3) ), SAY_COLOR )
	char.soundeffect( SAY_SOUND )
	char.say( sentence, SAY_COLOR )

	return amount

def onWalk(char, dir, sequence):
	# Only talk if talking toward our attack target
	if not char.attacktarget or char.distanceto(char.attacktarget) > 5:	
		return False
		
	# Otherwise a 10% chance
	if random.random() >= 0.10:
		return False
	
	char.soundeffect( SAY_SOUND )
	char.say( buildsentence(6), SAY_COLOR )
 	return False
