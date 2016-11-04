#include "skill.h"
#include "baseaddr.h"
#include "StringCovert.h"
skill g_skill;
skill::skill()
{
	m_skillList.resize(4);
}


skill::~skill()
{
}

bool skill::init()
{

	try
	{
		//dd  [[[17D41C4] + 27e8 + 538 + 4*i ]+ EC ]+ 18  名字
		DWORD ndBase = *(DWORD*)(Base_RoleAddr);
		for (int i = 0; i < 4; i++)
		{
			SKILL_PROPERTY temp;
			DWORD Offset1 = *(DWORD*)(ndBase + Offset_SkillOffset1 + Offset_SkillOffset2 + 4*i);
			DWORD tempObj = *(DWORD*)(Offset1 + 0xec);
			if (tempObj == NULL)
			{
				continue;
			}
			if (*(DWORD*)(tempObj + 0x28) > 0xf) 
				temp.szpName = (char*)(*(DWORD*)(tempObj + 0x18));
			else
				temp.szpName = (char*)(tempObj + 0x18);
			tools::getInstance()->message("技能名字:%s", Utf8ToAnsi( temp.szpName).c_str());
			m_skillList.push_back(temp);
		}
	}
	catch (...)
	{
		tools::getInstance()->message("初始化技能对象失败！\n");
		return false;
	}
	return true;
}
