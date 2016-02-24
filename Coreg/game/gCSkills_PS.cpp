#include "tools\address.h"
#include "gCSkills_PS.h"

void _declspec(naked) gCSkills_PS::SetSkillValue(int skill, long newValue)
{
	_asm {
		mov eax, SetSkillValueAddr
		jmp eax
	}
}

long _declspec(naked) gCSkills_PS::GetSkillValue(const int skill) {
	_asm {
		mov eax, GetSkillValueAddr
		jmp eax
	}
}


long _declspec(naked) gCSkills_PS::GetSkillMaxValue(const int skill)
{
	_asm {
		mov eax, GetSkillMaxValueAddr
		jmp eax
	}
}

long _declspec(naked) gCSkills_PS::GetSkillBaseValue(const int skill)
{
	_asm {
		mov eax, GetSkillBaseValueAddr
		jmp eax
	}
}