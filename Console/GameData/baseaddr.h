#pragma  once
#include "utils.h"
/*
基地址
*/
const DWORD Base_RoleAddr = 0x017D41C4;    //人物属性基地址     偏移:2    //OD地址:0x005BE463
const DWORD Base_GameUiAddr = 0x017D850C;    //游戏UI基地址     偏移:2    //OD地址:0x007F84F4
const DWORD Base_MonsterAddr = 0x027FACB4;    //怪物列表基地址     偏移:2    //OD地址:0x0071516F

/*
偏移
*/
const DWORD Offset_RoleMutilKill = 0x3f58;//玩家连杀偏移
const DWORD Offset_RoleKeepKill = 0x3edc; //玩家345杀偏移
const DWORD Offset_RoleLevel = 0x00003A2C;    //玩家等级偏移     偏移:2    //OD地址:0x00C0E949
const DWORD Offset_SkillOffset1 = 0x000027E8;    //技能对象第一层偏移     偏移:1    //OD地址:0x00A27D0D
const DWORD Offset_SkillOffset2 = 0x00000538;    //技能对象第二层偏移     偏移:3    //OD地址:0x00873B34