#pragma once
/*
数据结构头文件
包含所有要用到的数据结构
*/
#include "utils.h"

/*
处理收包的数据结构
*/
struct DATA_PROPERTY
{
	//包序号
	int index;
	//包类型
	int type;

	int len;
	//数据包
	std::shared_ptr<unsigned char> _ptr;
};

/*
AMF0数据类型
*/
typedef enum
{
	AMF0_NUMBER = 0,
	AMF0_BOOLEAN,
	AMF0_STRING,
	AMF0_OBJECT,
	AMF0_MOVIECLIP,      /* reserved, not used */
	AMF0_NULL,
	AMF0_UNDEFINED,
	AMF0_REFERENCE,
	AMF0_ECMA_ARRAY,
	Amf0_object_END,
	AMF0_STRICT_ARRAY,
	AMF0_DATE,
	AMF0_LONG_STRING,
	AMF0_UNSUPPORTED,
	AMF0_RECORDSET,       /* reserved, not used */
	AMF0_XML_DOC,
	AMF0_TYPED_OBJECT,
	AMF0_AVMPLUS,        /* switch to AMF3 */
	AMF0_INVALID = 0xff
} AMF0DataType;

/*
AMF3数据类型
*/
typedef enum
{
	AMF3_UNDEFINED = 0,
	AMF3_NULL,
	AMF3_FALSE,
	AMF3_TRUE,
	AMF3_INTEGER,
	AMF3_DOUBLE,
	AMF3_STRING,
	AMF3_XMLDOCUMENT,
	AMF3_DATE,
	AMF3_ARRAY,
	AMF3_OBJECT,
	AMF3_XML,
	AMF3_BYTEARRAY,
} AMF3DataType;

/*
AMF0/AMF3
*/
typedef enum
{
	TYPE_BASE = 0,
	TYPE_AMF0,
	TYPE_AMF3
}AMF_TYPE;

//坐标
struct POINT_PROPERTY
{
	float nfX;
	float nfY;
	float nfZ;
};



struct UI_PROPERTY
{
	//当前金币
	DWORD ndMoney;
	//剩余技能点
	DWORD ndRestSkillPnt;
	//补兵数
	DWORD ndKillMiniorNum;
	//死亡数
	DWORD ndDeadNum;
	//杀人数
	DWORD ndKillNum;
	//生命恢复
	float  nfLifeRecovery;
	//释放资源恢复
	float nfResourceRecovery;
	//护甲穿透
	float nfArmorPenetration;
	//法术穿透
	float nfSpellPenetration;
	//生命偷取
	float nfLifeSteal;
	//法术吸血
	float nfSpellVampire;
	//攻击距离
	float AttackDistance;
	//韧性
	float nfToughness;
	//攻击力
	float nfAggressivity;
	//法术强度
	float nfSpellPower;
	//护甲
	float nfArmor;
	//魔抗
	float nfMagicResistance;
	//攻击速度
	float nfAttackSpeed;
	//冷却缩减
	float nfCoolingReduction;
	//暴击几率
	float nfCriticalStrikeChance;
	//移动速度
	float nfMovingSpeed;
};


/*
人物基本属性
*/
struct ROLE_PROPERTY
{
	//UI显示的信息
	UI_PROPERTY ui;
	//玩家名字
	char* szpName;
	//玩家坐标
	POINT_PROPERTY pnt;
	//当前血量
	float nfCurHp;
	//最大血量
	float nfMaxHp;
	//当前蓝量
	float nfCurMp;
	//最大蓝量
	float nfMaxMp;
	//玩家等级
	DWORD ndLevel;
	//连杀数
	DWORD ndMutilKillNum;
	//345多杀
	DWORD ndKeepKillNum;
};

/*
怪物属性
*/
struct MONSTER_PROPERTY
{
	//名字
	char* szpName;
	//坐标
	POINT_PROPERTY pnt;
	//当前HP
	float nfCurHp;
	//最大HP
	float nfMaxHp;
	//怪物类型
	int type;
	//阵营
	int camp;
};


/*
技能对象
*/
struct SKILL_PROPERTY
{
	//技能名字
	char* szpName;
	//冷却时间
	//?????
	//技能ID
	DWORD ndId1;
	DWORD ndId2;

};