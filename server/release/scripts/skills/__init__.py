
import wolfpack
from wolfpack.consts import *

skills = {}

def register( id, handler ):
	if skills.has_key( id ):
		raise Exception, "Duplicate Skill: " + str( id )
	
	skills[ id ] = handler

def onLoad():
	wolfpack.registerglobal( EVENT_SKILLUSE, "skills" )
	
def onSkillUse( char, skill ):
	if skills.has_key( skill ):
		skills[ skill ]( char, skill )
		return 1
		
	return 0
