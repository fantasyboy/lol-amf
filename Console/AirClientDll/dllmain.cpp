// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "stdafx.h"
#include "global.h"
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		tools::getInstance()->message("挂载钩子！\n");
		if (!g_role.hookUnsealmessage())
		{
			tools::getInstance()->message("错误信息：%s", g_role.error().c_str());
			return false;
		}
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

