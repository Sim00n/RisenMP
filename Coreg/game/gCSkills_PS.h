#pragma once

class eCEntityPropertySet {};

class gCSkills_PS : public eCEntityPropertySet
{
public:
	enum SKILLS {
		NONE = -1,
		ATRIB_HP = 0,
		ATRIB_MP = 1,
		STAT_LV = 2,
		STAT_XP = 3,
		STAT_LP = 4,
		STAT_HP = 5,
		STAT_MP = 6,
		STAT_STR = 7,
		STAT_DEX = 8,
		STAT_INT = 9,
		PROT_EDGE = 10,
		PROT_BLUNT = 11,
		PROT_POINT = 12,
		PROT_FIRE = 13,
		PROT_ICE = 14,
		PROT_MAGIC = 15,
		COMBAT_CROSSBOW = 16,
		COMBAT_SWORD = 17,
		COMBAT_AXE = 18,
		COMBAT_STAFF = 19,
		COMBAT_BOT = 20,
		MAGIC_CIRCLE = 21,
		MAGIC_FIREBALL = 22,
		MAGIC_FROST = 23,
		MAGIC_MISSILE = 24,
		MISC_SMITH = 25,
		MISC_MINING = 26,
		MISC_LOCKPICK = 27,
		MISC_PICKPOCKET = 28,
		MISC_SNEAK = 29,
		MISC_ACROBAT = 30,
		MISC_ALCHEMY = 31,
		MISC_SCRIBE = 32,
		MISC_TROPHY = 33
	};

	void SetSkillValue(int skill, long newValue);
	long GetSkillValue(const int skill);
	long GetSkillMaxValue(const int skill);
	long GetSkillBaseValue(const int skill);
};