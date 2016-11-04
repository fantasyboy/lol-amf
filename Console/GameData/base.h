#pragma once
/*
这个类为所有的对象类的基类
*/
#include "utils.h"

class base
{
public:
	 base();
	 virtual~base();
	 void setError(std::string _str);
	 std::string error();
	 virtual bool init() = 0;
private:
	std::string m_errorString;
};

