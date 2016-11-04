#pragma once
#include "base.h"
#include "utils.h"
#include "datastruct.h"
/*
¼¼ÄÜÀà
*/
class skill :
	public base
{
public:
	skill();
	~skill();

	 bool init();

private:
	std::vector<SKILL_PROPERTY> m_skillList;

};

