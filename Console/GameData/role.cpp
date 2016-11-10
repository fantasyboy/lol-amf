#include "role.h"
#include <security.h>
#include "amfDecoder.h"

#include <SDKDDKVer.h>
#include <stdio.h>
#include <tchar.h>
#include "json.h"
#include <fstream>
#include "StringCovert.h"
#include "baseaddr.h"
#pragma comment(lib,"libJson.lib")

//HOOK unsealmessage相关变量
DWORD g_HookUnSealMessageAddr = 0;
DWORD g_HookUnSealMessageRetnAddr = 0;
DWORD g_HookUnsealMessageDataAddr = 0;

int g_type =0;

int role::m_index = 0;
std::mutex role::m_mutex;
std::list<std::shared_ptr<DATA_PROPERTY>> role::m_pDataList;
role g_role;
Json::Value ObjectConvert2Json(std::shared_ptr<Amf_Object> obj);
Json::Value Amf3Convert2Json(std::shared_ptr<Amf_Object> obj);
Json::Value Amf0Convert2Json(std::shared_ptr<Amf_Object> obj);

Json::Value ObjectConvert2Json(std::shared_ptr<Amf_Object> obj)
{

	Json::Value root;
	try{
		for (auto iter = obj->result.begin(); iter != obj->result.end(); iter++)
		{
			if (iter->second->amfType == TYPE_AMF0)
			{
				root[iter->first] = Amf0Convert2Json(iter->second);
			}
			else
			{
				root[iter->first] = Amf3Convert2Json(iter->second);
			}
		}

	}
	catch (...)
	{
		tools::getInstance()->message("出现异常！\n");
		return nullptr;
	}
	return root;
}
Json::Value Amf3Convert2Json(std::shared_ptr<Amf_Object> obj)
{
	Json::Value root;
	//判断类型
	switch (obj->type)
	{
	case AMF3_UNDEFINED: break;
	case AMF3_NULL: root = "null"; break;
	case AMF3_FALSE: root = false; break;
	case AMF3_TRUE:  root = true; break;
	case AMF3_INTEGER: root = obj->_value._int; break;
	case AMF3_DOUBLE: root = obj->_value._double; break;
	case AMF3_STRING:root = obj->text; break;
	case AMF3_XMLDOCUMENT: break;
	case AMF3_DATE: root = obj->_value._double; break;
	case AMF3_ARRAY:
	{
					   Json::Value aa;
					   for (auto iter : obj->temp)
					   {
						   aa.append(Amf3Convert2Json(iter));
					   }
					   root = aa;
					   break;
	}
	case AMF3_OBJECT:
	{
						Json::Value aa;
						for (auto iter = obj->result.begin(); iter != obj->result.end(); iter++)
						{
							aa[iter->first] = Amf3Convert2Json(iter->second);
						}
						root[obj->name] = aa;
						break;
	}
	case AMF3_XML: break;
	case AMF3_BYTEARRAY:
	{
						   for (auto iter : obj->temp)
						   {
							   root.append(Amf3Convert2Json(iter));
						   }
						   break;
	}
	default:
		break;
	}
	return root;
}
Json::Value Amf0Convert2Json(std::shared_ptr<Amf_Object> obj){

	Json::Value root;
	//判断类型
	switch (obj->type)
	{
	case AMF0_NUMBER: root = obj->_value._double; break;
	case AMF0_STRING: root = obj->text; break;
	case AMF0_OBJECT:  {
						   for (auto iter = obj->result.begin(); iter != obj->result.end(); iter++)
						   {
							   root[iter->first] = Amf0Convert2Json(iter->second);
						   }
						   break;
	}
	case AMF0_NULL:root = "null"; break;
	default:
		break;
	}
	return root;
}

void threadCallbackFunc()
{
	while (true)
	{	
		if (!role::m_pDataList.empty())
		{
			role::m_mutex.lock();
			std::shared_ptr<DATA_PROPERTY> temp = role::m_pDataList.front();
			role::m_pDataList.pop_front();
			role::m_mutex.unlock();
			unsigned char basicHeader = temp->type;
			int headerType = basicHeader & 0xc0;
			int headerSize = 0;
			if (headerType == 0x00)
				headerSize = 12;
			else if (headerType == 0x40)
				headerSize = 8;
			else if (headerType == 0x80)
				headerSize = 4;
			else if (headerType == 0xC0)
				headerSize = 1;

			//解析RTMP头 长度为 headerSize
			std::shared_ptr<unsigned char> _ppdata(new unsigned char[headerSize - 1]);
			memcpy(_ppdata.get(), temp->_ptr.get(), headerSize - 1);
			int size = 0;
			int type = 0;
			if (headerSize >= 8)
			{
				for (int i = 3; i < 6; i++)
					size = size * 256 + (_ppdata.get()[i] & 0xFF);
				type = _ppdata.get()[6];
			}

			
			//解析AMF的包
			if (type == 0x11)// Invoke
			{
				amfDecoder decoder;
				std::shared_ptr<Amf_Object> objPtr = decoder.decodeInvoke(&temp->_ptr.get()[headerSize - 1], temp->len);
				Json::Value data = ObjectConvert2Json(objPtr);
				tools::getInstance()->log2file("test.json", data.toStyledString(), std::ios::app);

			}
			else if (type == 0x14)// Connect
			{
				amfDecoder decoder;
				std::shared_ptr<Amf_Object> objPtr = decoder.decodeConnect(&temp->_ptr.get()[headerSize - 1], temp->len);
				Json::Value data = ObjectConvert2Json(objPtr);
				tools::getInstance()->log2file("test.json", data.toStyledString(), std::ios::app);
			}
			else if (type == 0x6)
			{

				continue;
			}
			else if (type == 0x3)
			{

				continue;
			}
			else
			{

				continue;
			}
		}
			Sleep(200);
	}

}

role::role()
{
	//std::thread th(&threadCallbackFunc);
	//th.detach();
	CloseHandle(::CreateThread(NULL, NULL, LPTHREAD_START_ROUTINE(threadCallbackFunc), NULL, NULL, NULL));
}


role::~role()
{
}

bool role::init()
{
	try
	{
		//表达式为：dd [[[17800a0]+20]+14] 
		DWORD ndBase1 = *(DWORD*)( *(DWORD*) ( *(DWORD*)(Base_GameUiAddr) +0x20 ) +0x14);
		m_roleProperty.ui.nfSpellPower = *(float*)(ndBase1 + 0x0);
		m_roleProperty.ui.nfArmor = *(float*)(ndBase1 + 0x8);
		m_roleProperty.ui.nfAggressivity = *(float*)(ndBase1 + 0x10);
		m_roleProperty.ui.nfAttackSpeed = *(float*)(ndBase1 + 0x18);
		m_roleProperty.ui.nfMagicResistance = *(float*)(ndBase1 + 0x20);
		m_roleProperty.ui.nfMovingSpeed = *(float*)(ndBase1 + 0x28);
		m_roleProperty.ui.nfCriticalStrikeChance = *(float*)(ndBase1 + 0x30);
		m_roleProperty.ui.nfCoolingReduction = *(float*)(ndBase1 + 0x38);
		m_roleProperty.ui.nfLifeSteal = *(float*)(ndBase1 + 0x40);
		m_roleProperty.ui.nfSpellVampire = *(float*)(ndBase1 + 0x48);
		m_roleProperty.ui.nfSpellPenetration = *(float*)(ndBase1 + 0x50);
		m_roleProperty.ui.nfArmorPenetration = *(float*)(ndBase1 + 0x58);
		m_roleProperty.ui.nfToughness = *(float*)(ndBase1 + 0x60);
		m_roleProperty.ui.AttackDistance = *(float*)(ndBase1 + 0x68);
		m_roleProperty.ui.nfLifeRecovery = *(float*)(ndBase1 + 0x70);
		m_roleProperty.ui.nfResourceRecovery = *(float*)(ndBase1 + 0x78);
		tools::getInstance()->message("法术强度:%f 护甲：%f 攻击力：%f 攻击速度：%f 魔抗：%f 移动速度：%f 暴击几率：%f 冷却缩减：%f 生命偷取：%f 法术吸血：%f 法术穿透：%f 护甲穿透：%f 韧性：%f 攻击距离：%f 生命恢复：%f 施放资源恢复：%f \n",
			m_roleProperty.ui.nfSpellPower,
			m_roleProperty.ui.nfArmor,
			m_roleProperty.ui.nfAggressivity,
			m_roleProperty.ui.nfAttackSpeed,
			m_roleProperty.ui.nfMagicResistance,
			m_roleProperty.ui.nfMovingSpeed,
			m_roleProperty.ui.nfCriticalStrikeChance,
			m_roleProperty.ui.nfCoolingReduction,
			m_roleProperty.ui.nfLifeSteal,
			m_roleProperty.ui.nfSpellVampire,
			m_roleProperty.ui.nfSpellPenetration,
			m_roleProperty.ui.nfArmorPenetration,
			m_roleProperty.ui.nfToughness,
			m_roleProperty.ui.AttackDistance,
			m_roleProperty.ui.nfLifeRecovery,
			m_roleProperty.ui.nfResourceRecovery
			);
		//当前金币 [[17800a0] + 2c]+44
		DWORD ndBase2 = *(DWORD*)( *(DWORD*)(Base_GameUiAddr)+ 0x2c);
		m_roleProperty.ui.ndMoney = *(DWORD*)(ndBase2 + 0x44);
		// 剩余技能点数
		DWORD ndBase3 = *(DWORD*)(*(DWORD*)(*(DWORD*)(Base_GameUiAddr)+0x8)+0x2c) ;
		m_roleProperty.ui.ndRestSkillPnt = *(DWORD*)(ndBase3 + 0x1c0);
		//补兵数
		DWORD ndBase4 =*(DWORD*)(*(DWORD*)( *(DWORD*)(*(DWORD*)(Base_GameUiAddr) +0x40) + 0x4));
		m_roleProperty.ui.ndKillMiniorNum = *(DWORD*)(*(DWORD*)(ndBase4 + 1 * 4) + 0x24);
		//杀人数/死亡数
		m_roleProperty.ui.ndDeadNum = *(DWORD*)(*(DWORD*)(ndBase4 + 0 * 4) + 0x24);
		m_roleProperty.ui.ndKillNum = *(DWORD*)(*(DWORD*)(ndBase4 + 0 * 4) + 0x28);
		tools::getInstance()->message("当前金币：%d 剩余技能点数：%d 补兵数：%d 杀人数：%d 死亡数：%d \n",
			m_roleProperty.ui.ndMoney,
			m_roleProperty.ui.ndRestSkillPnt,
			m_roleProperty.ui.ndKillMiniorNum,
			m_roleProperty.ui.ndDeadNum,
			m_roleProperty.ui.ndKillNum);
		//人物属性
		DWORD ndBase5 = *(DWORD*)(Base_RoleAddr);
		if (*(DWORD*)(Base_RoleAddr +0x30) <= 0xf)
			m_roleProperty.szpName = (char*)(ndBase5 + 0x20);
		else
			m_roleProperty.szpName = (char*)(*(DWORD*)(ndBase5 + 0x20));
		m_roleProperty.pnt.nfX = *(float*)(ndBase5 + 0x50);
		m_roleProperty.pnt.nfZ = *(float*)(ndBase5 + 0x54);
		m_roleProperty.pnt.nfY = *(float*)(ndBase5 + 0x58);
		m_roleProperty.nfCurHp = *(float*)(ndBase5 + 0x2fc);
		m_roleProperty.nfMaxHp = *(float*)(ndBase5 + 0x30c);
		m_roleProperty.nfCurMp = *(float*)(ndBase5 + 0x21c);
		m_roleProperty.nfMaxMp = *(float*)(ndBase5 + 0x22c);
		m_roleProperty.ndLevel = *(DWORD*)(ndBase5 + Offset_RoleLevel);
		m_roleProperty.ndMutilKillNum = *(DWORD*)(ndBase5 + Offset_RoleMutilKill);
		m_roleProperty.ndKeepKillNum = *(DWORD*)(ndBase5 + Offset_RoleKeepKill);
		tools::getInstance()->message("人物名字:%s 坐标(%f,%f,%f) 当前HP : %f 最大HP：%f  当前MP:%f 最大MP：%f  当前等级：%d 连杀数:%d 多杀数:%d\n",
			Utf8ToAnsi(m_roleProperty.szpName).c_str(),
			m_roleProperty.pnt.nfX,
			m_roleProperty.pnt.nfZ,
			m_roleProperty.pnt.nfY,
			m_roleProperty.nfCurHp,
			m_roleProperty.nfMaxHp,
			m_roleProperty.nfCurMp,
			m_roleProperty.nfMaxMp,
			m_roleProperty.ndLevel,
			m_roleProperty.ndMutilKillNum,
			m_roleProperty.ndKeepKillNum);

		if (m_roleProperty.ndKeepKillNum >= 1)
		{
			std::stringstream ss;
			ss << "恭喜你完成了345杀：" << m_roleProperty.ndKeepKillNum <<"杀！"<< std::endl;
			tools::getInstance()->log2file("keepKill.log", ss.str());
		}
		if (m_roleProperty.ndMutilKillNum >=3)
		{
			std::stringstream ss;
			ss << "恭喜你完成了连杀：" << m_roleProperty.ndMutilKillNum << "个！" << std::endl;
			tools::getInstance()->log2file("keepKill.log", ss.str());
		}

	}
	catch (...)
	{
		return false;
	}
	return true;
}

bool role::hookUnsealmessage()
{
	MODULEINFO miSspiCli = tools::getInstance()->getModuleInfo("Adobe AIR.dll");
	if ((DWORD)miSspiCli.lpBaseOfDll <1)
	{
		setError("获取模块信息失败！");
		return false;
	}
	DWORD searchAddr = tools::getInstance()->findPattern((DWORD)miSspiCli.lpBaseOfDll, miSspiCli.SizeOfImage,
		(PUCHAR)"\xA1\x2C\xC8\xB6\x52\x8B\x80\x3C\x02\x00\x00\x8B\x40\x04\x57\x57\x8D\x4D\xB4\x51\x8D\x4E\x40\x51\xFF\x50\x68",
		(PCHAR)"x????xxxxxxxxxxxxxxxxxxxxxx");
	if (searchAddr < 1)
	{
		setError("查找特征码失败！");
		return false;
	}
	g_HookUnSealMessageAddr = searchAddr + 0x1b;
	g_HookUnSealMessageRetnAddr = g_HookUnSealMessageAddr + 0x5;
	tools::getInstance()->message("HOOK地址:%x  返回的地址为：%x", g_HookUnSealMessageAddr, g_HookUnSealMessageRetnAddr);
	
	byte hookData[5] = { 0xe9, 0x0, 0x0, 0x0, 0x0 };
	*(DWORD*)(&hookData[1]) = (DWORD)(&unsealmessage_stub) - g_HookUnSealMessageAddr - 0x5;
	if (!tools::getInstance()->write(g_HookUnSealMessageAddr ,hookData , 5))
	{
		setError("写入钩子失败！");
		return false;
	}

	return true;
}

//获取连杀数个数
DWORD role::getMutilKillNum()
{
	DWORD ndBase5 = *(DWORD*)(Base_RoleAddr);
	return  *(DWORD*)(ndBase5 + Offset_RoleMutilKill);
}
//获取多杀个数
DWORD role::getKeepKillNum()
{
	DWORD ndBase5 = *(DWORD*)(Base_RoleAddr);
	return *(DWORD*)(ndBase5 + Offset_RoleKeepKill);
}

void __declspec(naked) unsealmessage_stub()
{
	__asm
	{
		pushad
		pushfd
		lea ecx, [ebp - 0x4c]
		mov g_HookUnsealMessageDataAddr, ecx
	}
	unsealmessage_del(g_HookUnsealMessageDataAddr);
	__asm{
	popfd
		popad
		CMP EAX, 80090318h
		jmp g_HookUnSealMessageRetnAddr
	}

}

void __stdcall unsealmessage_del(unsigned long _data)
{
	try
	{
		if (IsBadReadPtr((void*)_data ,4))
		{
			return ;
		}
		PSecBufferDesc pBuf = (PSecBufferDesc)(_data);
		if (pBuf->cBuffers <= 1)
		{
			return;
		}
		if (nullptr == pBuf->pBuffers[1].pvBuffer || pBuf->pBuffers[1].cbBuffer <1)
		{
			return;
		}
		role::m_index++;
		if (role::m_index <= 1)
		{
			return;
		}
		if (pBuf->pBuffers[1].cbBuffer  == 1)
		{
			g_type = ((char*)pBuf->pBuffers[1].pvBuffer)[0];
		}
		else
		{
			std::shared_ptr<DATA_PROPERTY> _ppdata(new DATA_PROPERTY);
			_ppdata->index = role::m_index;
			_ppdata->type = g_type;
			_ppdata->len = pBuf->pBuffers[1].cbBuffer;
			std::shared_ptr<unsigned char> _data(new unsigned char[pBuf->pBuffers[1].cbBuffer + 1]);
			memcpy((void*)_data.get(), pBuf->pBuffers[1].pvBuffer, pBuf->pBuffers[1].cbBuffer);
			_ppdata->_ptr = _data;
			role::m_mutex.lock();
			role::m_pDataList.push_back(_ppdata);
			role::m_mutex.unlock();

			/*
			把包写入文件
			*/
			//std::shared_ptr<unsigned char> tempPtr(new unsigned char[pBuf->pBuffers[1].cbBuffer + 1]);
			//memcpy(tempPtr.get(), pBuf->pBuffers[1].pvBuffer, pBuf->pBuffers[1].cbBuffer);
			//std::shared_ptr< char> resultPtr(new  char[pBuf->pBuffers[1].cbBuffer * 3 + 1]);
			//tools::getInstance()->bytes2hexstr(tempPtr.get(), pBuf->pBuffers[1].cbBuffer, resultPtr.get());
			//std::stringstream ss;
			//ss << "类型：" << g_type << "包长：" << pBuf->pBuffers[1].cbBuffer << "数据：" << std::string(resultPtr.get(), pBuf->pBuffers[1].cbBuffer * 3 + 1) << std::endl;
			//tools::getInstance()->log2file("temp.dat", ss.str(), std::ios::app | std::ios::binary);

		}


	}
	catch (...)
	{
		tools::getInstance()->message("数据包处理异常！\n");
		return;
	}
}




//Json::Value ObjectConvert2Json(Amf_object obj)
//{
//
//	Json::Value root;
//
//
//	if (obj.result.empty())
//	{
//		return root;
//	}
//
//	std::map<std::string, Amf_object>::const_iterator iter;
//	for (iter = obj.result.begin(); iter != obj.result.end(); iter++)
//	{
//		//std::cout << "key: " << iter->first << " ";
//		if (iter->second.amftype == TYPE_AMF0)
//		{
//			//AFM0
//			switch (iter->second.type)
//			{
//			case 0:
//				//std::cout << "value: " << iter->second._value._double;
//				root[iter->first] = iter->second._value._double;
//				break;
//			case 2:
//				//value = obj.text;
//				//std::cout << "value: " << iter->second.text;
//				root[iter->first] = iter->second.text;
//				break;
//			case 3:
//				//return false;
//				root[iter->first] = ObjectConvert2Json(iter->second);
//				break;
//			case 5:
//				root[iter->first] = "null";
//				break;
//
//			default:
//				//调用到这，说明是对象
//				break;
//			}
//		}
//		else if (iter->second.amftype == TYPE_AMF3)
//		{
//
//			switch (iter->second.type)
//			{
//			case 1:
//				root[iter->first] = "null";
//				break;
//			case 2:
//			{
//					  // bool b = false;
//					  // std::cout << "value: " << iter->second._value._bool;
//					  root[iter->first] = iter->second._value._bool;
//					  break;
//			}
//			case 3:
//			{
//					  // std::cout << "value: " << iter->second._value._bool;
//					  root[iter->first] = iter->second._value._bool;
//					  break;
//			}
//			case 4:
//				//std::cout << "value: " << iter->second._value._int;
//				root[iter->first] = iter->second._value._int;
//				break;
//			case 5:
//				//std::cout << "value: " << iter->second._value._double;
//				root[iter->first] = iter->second._value._double;
//				break;
//			case 6:
//				//std::cout << "value: " << Utf8ToAnsi(iter->second.text);
//				root[iter->first] = Utf8ToAnsi(iter->second.text);
//				break;
//			case 8:
//				//std::cout << "value: " << Utf8ToAnsi(iter->second.text);
//				root[iter->first] = Utf8ToAnsi(iter->second.text);
//				break;
//			case 10:
//				root[iter->first] = ObjectConvert2Json(iter->second);
//				break;
//			default:
//				//return "";
//				break;
//
//			}
//		}
//		else
//		{
//			root[iter->first] = ObjectConvert2Json(iter->second);
//		}
//		//std::cout << endl;
//	}
//
//
//	return root;
//
//
//}