
import random
from wolfpack.consts import *
from wolfpack import console

#
# The monster speechbuilder class
#
class SpeechBuilder:
	def __init__(self, saycolor, saysound, syllables):
		self.saycolor = saycolor
		self.saysound = saysound
		self.syllables = syllables
		
	#
	# Build a random word with a given number of syllables
	#
	def buildword(self, syllables):
		result = ''
		for i in range(0, syllables):
			result += random.choice(self.syllables)
		return result
	
	#
	# Delimit a sentence
	#
	def delimiter(self,end):
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
	def buildsentence(self, words):
		sentence = ''
		sentencestart = True
		
		for i in range(0, words):
			# 2/3 long words, 1/3 shorts
			if random.random() >= 0.33:
				word = self.buildword(random.randint(1, 5))
			else:
				word = self.buildword(random.randint(1, 3))
				
			# Captalize if beginning of sentence
			if sentencestart:
				sentence += word.capitalize()
			else:
				sentence += word
				
			# Add a delimiter
			char = self.delimiter(i + 1 == words)
			sentence += char		
			sentencestart = char[0] != ' '
	
		return sentence

RATMAN_SAY_COLOR = 149
RATMAN_SAY_SOUND = 438
RATMAN_SYLLABLES = [
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

ORC_SAY_COLOR = 34
ORC_SAY_SOUND = 432
ORC_SYLLABLES = [
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

LIZARDMAN_SAY_COLOR = 58
LIZARDMAN_SAY_SOUND = 418
LIZARDMAN_SYLLABLES = [
	"ss", "sth", "iss", "is", "ith", "kth", "sith", "this", "its", 
	"sit", "tis", "tsi", "ssi", "sil", "lis", "sis", "lil", "thil", 
	"lith", "sthi", "lish", "shi", "shash", "sal", "miss", "ra", 
	"tha", "thes", "ses", "sas", "las", "les", "sath", "sia", "ais", 
	"isa", "asi", "asth", "stha", "sthi", "isth", "asa", "ath", "tha", 
	"als", "sla", "thth", "ci", "ce", "cy", "yss", "ys", "yth", "syth", 
	"thys", "yts", "syt", "tys", "tsy", "ssy", "syl", "lys", "sys", 
	"lyl", "thyl", "lyth", "sthy", "lysh", "shy", "myss", "ysa", "sthy", 
	"ysth"
]

# The speechbuilder map
speechbuilders = {
	'ratman': SpeechBuilder(RATMAN_SAY_COLOR, RATMAN_SAY_SOUND, RATMAN_SYLLABLES),
	'orc': SpeechBuilder(ORC_SAY_COLOR, ORC_SAY_SOUND, ORC_SYLLABLES),
	'lizardman': SpeechBuilder(LIZARDMAN_SAY_COLOR, LIZARDMAN_SAY_SOUND, LIZARDMAN_SYLLABLES),
}

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

	speech = char.getstrproperty('monsterspeech', '')
	global speechbuilders
	if not speechbuilders.has_key(speech):
		return amount
	speechbuilder = speechbuilders[speech]

	# Say english sentence after constructed one
	char.say( speechbuilder.buildsentence( random.randint(2, 3) ), speechbuilder.saycolor )
	char.soundeffect( speechbuilder.saysound )
	char.say( sentence, speechbuilder.saycolor )

	return amount

def onWalk(char, dir, sequence):
	# Only talk if talking toward our attack target
	if not char.attacktarget or char.distanceto(char.attacktarget) > 5:	
		return False
		
	# Otherwise a 10% chance
	if random.random() >= 0.10:
		return False
		
	speech = char.getstrproperty('monsterspeech', '')
	global speechbuilders
	if not speechbuilders.has_key(speech):
		console.log(LOG_MESSAGE, speech + "\n")
		return False
	speechbuilder = speechbuilders[speech]	
	
	char.soundeffect( speechbuilder.saysound )
	char.say( speechbuilder.buildsentence(6), speechbuilder.saycolor )
 	return False
