#include "base.h"


base::base()
{
}


base::~base()
{
}

void base::setError(std::string _str)
{
	m_errorString = _str;
}

std::string base::error()
{
	return m_errorString;
}

bool base::init()
{
	return false;
}
