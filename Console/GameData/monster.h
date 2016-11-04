#pragma once
#include "base.h"
/*
นึฮ๏สýื้
*/
#include "utils.h"
#include "datastruct.h"
class monster :
	public base
{
public:
	monster();
	~monster();
	virtual bool init();

private:
	std::vector<MONSTER_PROPERTY> m_monster;
};

