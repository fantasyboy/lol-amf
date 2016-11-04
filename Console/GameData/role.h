#pragma once
#include "base.h"
#include "datastruct.h"
#include "amfDecoder.h"
class role :
	public base
{
public:
	role();
	~role();
	virtual bool init();
	bool hookUnsealmessage();

public:
	static std::list<std::shared_ptr<DATA_PROPERTY>> m_pDataList;
	static int m_index;
	static std::mutex m_mutex;
	ROLE_PROPERTY m_roleProperty;
	/*
	获取多杀连杀数接口
	*/
public:
	DWORD getMutilKillNum();
	DWORD getKeepKillNum();
};

void  unsealmessage_stub();
void  __stdcall unsealmessage_del(unsigned long _data);

