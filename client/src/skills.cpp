
#include "skills.h"
#include "config.h"

cSkills::cSkills() {
	setObjectName("Skills");
}

cSkills::~cSkills() {
}

void cSkills::load() {
}

void cSkills::unload() {
}

uint cSkills::count() const {
	return SkillCount;
}

cSkills *Skills = 0;
