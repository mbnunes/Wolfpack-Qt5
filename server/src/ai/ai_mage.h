
#if !defined(__AI_MAGE_H__)

class Monster_Mage: public Monster_Aggressive {
protected:
	Monster_Mage() : Monster_Aggressive() {
	}

public:
	Monster_Mage(P_NPC npc);

	static AbstractAI *create() {
		return new Monster_Mage(0);
	}

	static void registerInFactory() {
		AIFactory::instance()->registerType( "Monster_Mage", create );
	}

	virtual QString name() {
		return "Monster_Mage";
	}

protected:
	virtual void selectVictim();
};

#endif
