
#if !defined(__SKILLS_H__)
#define __SKILLS_H__

#include <QObject>

#include "muls/localization.h"

class cSkills : public QObject {
Q_OBJECT
Q_ENUMS(Skill);
public:
	enum Skill {
		Alchemy = 0,
		Anatomy,
		AnimalLore,
		ItemIdentification,
		ArmsLore,
		Parrying,
		Begging,
		Blacksmithy,
		BowcraftandFletching,
		Peacemaking,
		Camping,
		Carpentry,
		Cartography,
		Cooking,
		DetectingHidden,
		Discordance,
		EvaluatingIntelligence,
		Healing,
		Fishing,
		Forensics,
		Herding,
		Hiding,
		Provocation,
		Inscription,
		LockPicking,
		Magery,
		MagicResistance,
		Tactics,
		Snooping,
		Musicianship,
		Poisoning,
		Archery,
		SpiritSpeak,
		Stealing,
		Tailoring,
		AnimalTaming,
		TasteIdentification,
		Tinkering,
		Tracking,
		Veterinary,
		Swordsmanship,
		MaceFighting,
		Fencing,
		Wrestling,
		Lumberjacking,
		Mining,
		Meditation,
		Stealth,
		PreAosSkillCount,
		Disarming = Stealth + 1,
		Necromancy,
		Focus,		
		Chivalry,
		PreSeSkillCount,
		Bushido = Chivalry + 1,
		Ninjitsu,
		PreMlSkillCount,
		Spellweaving = Ninjitsu + 1,
		SkillCount
	};

	cSkills();
	~cSkills();

	void load();
	void unload();

public slots:

	// This depends more or less on the configuration
	uint count() const;
	Skill get(uint id) const;
	uint getSkillId(Skill skill) const;
 };

inline cSkills::Skill cSkills::get(uint id) const {
	if (id >= count()) {
		return Alchemy;
	} else {
		return Skill(id);
	}
}

inline uint cSkills::getSkillId(Skill skill) const {
	return (uint)skill;
}

extern cSkills *Skills;

#endif
