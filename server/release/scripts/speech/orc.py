
import random

SAY_COLOR = 34
SAY_SOUND = 432

# List of syllables inspired by RunUO
SYLLABLES = [
	"bu", "du", "fu", "ju", "gu", "ulg", "gug", "gub", "gur", "oog", "gub", 
	"log", "ru", "stu", "glu", "ug", "ud", "og", "log", "ro", "flu", "bo", 
	"duf", "fun", "nog", "dun", "bog", "dug", "gh", "ghu", "gho", "nug", "ig", 
	"igh", "ihg", "luh", "duh", "bug", "dug", "dru", "urd", "gurt", "grut", 
	"grunt", "snarf", "urgle", "igg", "glu", "glug", "foo", "bar", "baz", "ghat", 
	"ab", "ad", "gugh", "guk", "ag", "alm", "thu", "log", "bilge", "augh", "gha", 
	"gig", "goth", "zug", "pig", "auh", "gan", "azh", "bag", "hig", "oth", "dagh", 
	"gulg", "ugh", "ba", "bid", "gug", "bug", "rug", "hat", "brui", "gagh", "buad", 
	"buil", "buim", "bum", "hug", "hug", "buo", "ma", "buor", "ghed", "buu", "ca", 
	"guk", "clog", "thurg", "car", "cro", "thu", "da", "cuk", "gil", "cur", "dak", 
	"dar", "deak", "der", "dil", "dit", "at", "ag", "dor", "gar", "dre", "tk", "dri", 
	"gka", "rim", "eag", "egg", "ha", "rod", "eg", "lat", "eichel", "ek", "ep", "ka", 
	"it", "ut", "ewk", "ba", "dagh", "faugh", "foz", "fog", "fid", "fruk", "gag", "fub", 
	"fud", "fur", "bog", "fup", "hagh", "gaa", "kt", "rekk", "lub", "lug", "tug", "gna", 
	"urg", "l", "gno", "gnu", "gol", "gom", "kug", "ukk", "jak", "jek", "rukk", "jja", 
	"akt", "nuk", "hok", "hrol", "olm", "natz", "i", "i", "o", "u", "ikk", "ign", "juk", 
	"kh", "kgh", "ka", "hig", "ke", "ki", "klap", "klu", "knod", "kod", "knu", "thnu", 
	"krug", "nug", "nar", "nag", "neg", "neh", "oag", "ob", "ogh", "oh", "om", "dud", 
	"oo", "pa", "hrak", "qo", "quad", "quil", "ghig", "rur", "sag", "sah", "sg"
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
