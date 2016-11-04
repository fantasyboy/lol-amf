#pragma once
/*
AMF解析类
*/
#include "utils.h"
#include "datastruct.h"

/*
AMF_BASE
*/
struct Amf_Object
{
	Amf_Object();
	virtual ~Amf_Object();

	int type;
	AMF_TYPE amfType;
	//KEY
	std::string name;
	//VALUE
	std::string text;
	union
	{
		bool _bool = false;
		int _int;
		double _double;
	}_value;
	std::multimap<std::string, std::shared_ptr<Amf_Object>> result;
	std::vector<std::shared_ptr<Amf_Object>> temp;
};

struct Amf0_object :public Amf_Object
{
	Amf0_object();
	virtual ~Amf0_object();
};


struct Amf3_object :public Amf_Object
{
	Amf3_object();
	virtual ~Amf3_object();
};



struct ClassDefinition
{
	ClassDefinition();
	~ClassDefinition();
	//类型
	std::string type;
	//是否是扩展对象
	bool externalizable;
	//是否是动态对象
	bool dynamic;
	//成员名字
	std::vector<std::string> members;
};

/*
AMF解析类
*/
class amfDecoder
{
public:
	amfDecoder();
	~amfDecoder();

	std::shared_ptr<Amf_Object> decodeInvoke(unsigned char* _data, unsigned int _len);
	std::shared_ptr<Amf_Object> decodeConnect(unsigned char* _data, unsigned int _len);
private:
	void reset();
	unsigned char readByte();
	int readByteAsInt();
	double readDouble();
	int readInt();
	std::string readString();
	std::shared_ptr<unsigned char> readBytes(unsigned int len);
	std::string readDate();
	std::shared_ptr<Amf3_object> readArray();
	std::shared_ptr<Amf3_object> readObject();
	std::shared_ptr<Amf3_object> decode_AMF3();
	std::shared_ptr<Amf_Object> decode_AMF0();
	std::vector<int> readFlags();
	void readRemaining(int flag, int bits);
	std::shared_ptr<Amf3_object> readByteArray();
	std::shared_ptr<Amf3_object> readDSA();
	std::shared_ptr<Amf3_object> readDSK();
	std::shared_ptr<Amf0_object> readIntAMF0();
	std::shared_ptr<Amf0_object> readStringAMF0();
	std::shared_ptr<Amf0_object> readNullAMF0();
	std::shared_ptr<Amf0_object> readObjectAMF0();
private:
	std::vector<std::string> stringReferences;
	std::vector<std::shared_ptr<Amf3_object>> objectReferences;
	std::vector<std::shared_ptr<ClassDefinition>> classDefinitions;
	unsigned char* m_dataBuffer;
	unsigned int m_dataPos;
	unsigned int m_dataSize;
};

