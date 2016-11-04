#include "amfDecoder.h"


amfDecoder::amfDecoder()
{
}


amfDecoder::~amfDecoder()
{
}

void amfDecoder::reset()
{
	if (!classDefinitions.empty())
	{
		classDefinitions.clear();
	}
	if (!stringReferences.empty())
	{
		stringReferences.clear();
	}
	if (!objectReferences.empty())
	{
		objectReferences.clear();
	}
}

unsigned char amfDecoder::readByte()
{
	if (m_dataSize == m_dataPos)
	{
		return 0;
	}
	unsigned char ret = m_dataBuffer[m_dataPos];
	m_dataPos++;
	return ret;
}

int amfDecoder::readByteAsInt()
{
	int ret = readByte();
	if (ret < 0)
	{
		ret += 256;
	}
	return ret;
}

double amfDecoder::readDouble()
{
	double value = 0.0;
	unsigned char result[8] = { 0 };
	result[7] = readByte();
	result[6] = readByte();
	result[5] = readByte();
	result[4] = readByte();
	result[3] = readByte();
	result[2] = readByte();
	result[1] = readByte();
	result[0] = readByte();
	return *(double*)(result);
}

int amfDecoder::readInt()
{
	int integer = 0;
	int seen = 0;
	int b = 0;
	for (;;)
	{
		b = (unsigned char)m_dataBuffer[m_dataPos++];
		if (seen == 3)
		{
			integer = (integer << 8) | b;
			break;
		}
		integer = (integer << 7) | (b & 0x7f);
		if ((b & 0x80) == 0x80)
		{
			seen++;
		}
		else {
			break;
		}
	}
	if (integer > (0x7FFFFFFF >> 3))
		integer -= (1 << 29);
	return (int)integer;
}

std::string amfDecoder::readString()
{
		int handle = readInt();
		bool line = ((handle & 1) != 0);
		handle = handle >> 1;
		if (line)
		{
			if (0 == handle)
			{
				return "";
			}
			std::string result((char*)readBytes(handle).get(), handle);
			stringReferences.push_back(result);
			return result;
		}
		else
		{
			return stringReferences.at(handle);
		}


}

std::shared_ptr<unsigned char> amfDecoder::readBytes(unsigned int len)
{
	unsigned char* result = new unsigned char[len + 1];
	memset(result, 0, len + 1);
	for (int i = 0; i != len; i++)
	{
		result[i] = readByte();
	}
	std::shared_ptr<unsigned char> temp;
	temp.reset(result);
	return temp;
}

std::string amfDecoder::readDate()
{
	int handle = readInt();
	bool line = ((handle & 1) != 0);
	handle = handle >> 1;
	if (line)
	{
		Amf3_object* tempObj = new Amf3_object;
		tempObj->type = AMF3_DATE;
		tempObj->_value._double = readDouble();
		std::stringstream ss;
		ss << "时间戳：" << tempObj->_value._double;
		tempObj->text = ss.str();
		std::shared_ptr<Amf3_object> tempPtr;
		tempPtr.reset(tempObj);
		objectReferences.push_back(tempPtr);

		return tempPtr->text;
	}
	else
	{
		return objectReferences.at(handle).get()->text;
	}
}

std::shared_ptr<Amf3_object> amfDecoder::readArray()
{
	int handle = readInt();
	bool line = ((handle & 1) != 0);
	handle = handle >> 1;
	if (line)
	{
		/*	Amf3_object* obj = new Amf3_object;
			obj->type = AMF3_ARRAY;*/
		std::shared_ptr<Amf3_object> tempPtr(new Amf3_object);
		//tempPtr.reset(obj);
		tempPtr->type = AMF3_ARRAY;
		std::string key = readString();
		if ("" != key)
		{
			return tempPtr;
		}
		objectReferences.push_back(tempPtr);
		for (int i = 0; i != handle; i++)
		{
			tempPtr->temp.push_back(decode_AMF3());
		}
		return tempPtr;
	}
	else
	{
		return objectReferences.at(handle);
	}
}



std::shared_ptr<Amf3_object> amfDecoder::readObject()
{
	int handle = readInt();
	bool line = ((handle & 1) != 0);
	handle = handle >> 1;
	if (line)
	{
		bool inlineDefine = ((handle & 1) != 0);
		handle = handle >> 1;

		
		std::shared_ptr<ClassDefinition> classPtr(new ClassDefinition);
		if (inlineDefine)
		{
			classPtr->type = readString();
			classPtr->externalizable = ((handle & 1) != 0);
			handle = handle >> 1;
			classPtr->dynamic = ((handle & 1) != 0);
			handle = handle >> 1;

			for (int i = 0; i < handle; i++)
			{
				classPtr->members.push_back(readString());
			}
			classDefinitions.push_back(classPtr);
		}
		else
		{
			classPtr = classDefinitions.at(handle);
		}


		std::shared_ptr<Amf3_object> objPtr(new Amf3_object);
		objPtr->name = classPtr->type;
		objPtr->type = AMF3_OBJECT;
		objectReferences.push_back(objPtr);
		if (classPtr->externalizable)
		{
			if (classPtr->type.compare("DSK") == 0)
			{
				objPtr = readDSK();
			}
			else if (classPtr->type.compare("DSA") == 0)
			{
				objPtr = readDSA();
			}
			else if (classPtr->type.compare("flex.messaging.io.ArrayCollection") == 0)
			{
				//ret.name.clear();
				std::shared_ptr<Amf3_object> tempP = decode_AMF3();
				if (tempP->type == AMF3_ARRAY)
				{
					//ret.result.insert(make_pair(cd.type, temp.childObject[0]));
					for (int i = 0; i != tempP->temp.size(); i++)
					{
						objPtr->result.insert(std::make_pair(tempP->temp[i]->name, tempP->temp[i]));
					}
				}

			}
			else if ((classPtr->type.compare("com.riotgames.platform.systemstate.ClientSystemStatesNotification") == 0) || (classPtr->type.compare("com.riotgames.platform.broadcast.BroadcastNotification") == 0))
			{
				int size = 0;
				for (int i = 0; i < 4; i++)
					size = size * 256 + readByteAsInt();
				std::string result = std::string((char*)readBytes(size).get(), size);
				tools::getInstance()->log2file("json.data", result, std::ios::app);
			}
		}
		else
		{

			for (int i = 0; i != classPtr->members.size(); i++)
			{
				std::string key = classPtr->members.at(i);
				std::shared_ptr<Amf3_object> tempP = decode_AMF3();
				objPtr->result.insert(std::make_pair(key, tempP));
			}

			if (classPtr->dynamic)
			{
				std::string key;
				while ((key = readString()).length() != 0)
				{
					std::shared_ptr<Amf3_object>  value = decode_AMF3();
					objPtr->result.insert(std::make_pair(key, value));
				}
			}
		}
		return objPtr;
	}
	else
	{
		return objectReferences.at(handle);
	}
}

std::shared_ptr<Amf3_object> amfDecoder::decode_AMF3()
{
	//Amf3_object *obj = new Amf3_object();
	std::shared_ptr<Amf3_object> objPtr(new Amf3_object);
	//objPtre.reset(obj);
	unsigned char type = readByte();
	switch (type)
	{
	case 0x00:
		break;
	case 0x01:
	{
				 objPtr->type = AMF3_NULL;
				 return objPtr;
	}
	case 0x02:
	{
				 objPtr->type = AMF3_FALSE;
				 objPtr->_value._bool = false;
				 return objPtr;
	}


	case 0x03:
	{
				 objPtr->type = AMF3_TRUE;
				 objPtr->_value._bool = true;
				 return objPtr;
	}

	case 0x04:
	{
				 objPtr->type = AMF3_INTEGER;
				 objPtr->_value._int = readInt();
				 return objPtr;
	}

	case 0x05:
	{
				 objPtr->type = AMF3_DOUBLE;
				 objPtr->_value._double = readDouble();
				 return objPtr;

	}

	case 0x06:
	{

				 objPtr->type = AMF3_STRING;
				 objPtr->text = readString();
				 return objPtr;
	}
	case 0x07:
		break;
	case 0x08:
	{
				 objPtr->type = AMF3_DATE;
				 objPtr->text = readDate();
				 return objPtr;
	}
	case 0x09:
	{
				 objPtr = readArray();
				 objPtr->type = AMF3_ARRAY;
				 return objPtr;
	}
	case 0x0A:
	{
				 objPtr = readObject();
				 objPtr->type = AMF3_OBJECT;
				 return objPtr;
	}
	case 0x0B:
	{
				 break;
	}
	case 0x0C:
	{
				 objPtr = readByteArray();
				 objPtr->type = AMF3_BYTEARRAY;
				 return objPtr;
	}

	}
	objPtr->type = AMF3_UNDEFINED;

	return objPtr;
}

std::vector<int> amfDecoder::readFlags()
{
	std::vector<int> flags;
	int flag;
	do
	{
		flag = readByteAsInt();
		flags.push_back(flag);
	} while ((flag & 0x80) != 0);
	return flags;
}

void amfDecoder::readRemaining(int flag, int bits)
{
	if ((flag >> bits) != 0)
	{
		for (int o = bits; o < 6; o++)
		{
			if (((flag >> o) & 1) != 0)
				decode_AMF3();
		}
	}
}

std::shared_ptr<Amf3_object> amfDecoder::readByteArray()
{
	int handle = readInt();
	bool line = ((handle & 1) != 0);
	handle = handle >> 1;
	if (line)
	{
		Amf3_object* obj = new Amf3_object;
		obj->type = AMF3_BYTEARRAY;
		std::shared_ptr<Amf3_object> tempPtr;
		tempPtr.reset(obj);
		obj->text = std::string((char*)readBytes(handle).get(), handle);
		objectReferences.push_back(tempPtr);
		return tempPtr;
	}
	else
	{
		return objectReferences.at(handle);
	}
}

std::shared_ptr<Amf3_object> amfDecoder::readDSA()
{
	Amf3_object* ret = new Amf3_object;
	std::shared_ptr<Amf3_object> retPtr;
	retPtr.reset(ret);

	int flag;
	std::vector<int> flags = readFlags();
	for (int i = 0; i != flags.size(); i++)
	{
		flag = flags.at(i);
		int bits = 0;
		if (i == 0)
		{
			if ((flag & 0x01) != 0)
				ret->result.insert(std::make_pair("body", decode_AMF3()));
			if ((flag & 0x02) != 0)
				ret->result.insert(std::make_pair("clientId", decode_AMF3()));
			if ((flag & 0x04) != 0)
				ret->result.insert(std::make_pair("destination", decode_AMF3()));
			if ((flag & 0x08) != 0)
				ret->result.insert(std::make_pair("headers", decode_AMF3()));
			if ((flag & 0x10) != 0)
				ret->result.insert(std::make_pair("messageId", decode_AMF3()));
			if ((flag & 0x20) != 0)
				ret->result.insert(std::make_pair("timeStamp", decode_AMF3()));
			if ((flag & 0x40) != 0)
				ret->result.insert(std::make_pair("timeToLive", decode_AMF3()));
			bits = 7;
		}
		else if (i == 1)
		{
			if ((flag & 0x01) != 0)
			{
				readByte();
				std::shared_ptr<Amf3_object> obj = readByteArray();
				//obj.text = byteArrayToID(obj.text);
				ret->result.insert(std::make_pair("clientId", obj));
				//ret.put("clientId", obj);
			}
			if ((flag & 0x02) != 0)
			{
				readByte();
				std::shared_ptr<Amf3_object> obj = readByteArray();
				//obj.text = byteArrayToID(obj.text);
				ret->result.insert(std::make_pair("messageIdBytes", obj));
				//ret.put("messageId", byteArrayToID(temp));
			}
			bits = 2;
		}
		readRemaining(flag, bits);
	}


	flags = readFlags();
	for (int i = 0; i != flags.size(); i++)
	{
		flag = flags.at(i);
		int bits = 0;

		if (i == 0)
		{
			if ((flag & 0x01) != 0)
				ret->result.insert(std::make_pair("correlationId", decode_AMF3()));
			if ((flag & 0x02) != 0)
			{
				readByte();
				std::shared_ptr<Amf3_object> obj = readByteArray();
				//ret.result.insert(std::make_pair("correlationIdBytes", temp);
				//obj.text = byteArrayToID(obj.text);
				ret->result.insert(std::make_pair("correlationId", obj));
			}
			bits = 2;
		}

		readRemaining(flag, bits);
	}
	return retPtr;
}

std::shared_ptr<Amf3_object> amfDecoder::readDSK()
{
	std::shared_ptr<Amf3_object> ret = readDSA();
	std::vector<int> flags = readFlags();
	for (int i = 0; i != flags.size(); i++)
	{
		readRemaining(flags.at(i), 0);
	}
	return ret;
}

std::shared_ptr<Amf_Object> amfDecoder::decode_AMF0()
{
	int type = readByte();
	Amf_Object *obj = new Amf_Object;
	std::shared_ptr<Amf_Object> objPtr;
	objPtr.reset(obj);
	switch (type)
	{
	case 0x00:
		objPtr =  readIntAMF0();
		break;
	case 0x02:
		objPtr = readStringAMF0();
		break;
	case 0x03:
		objPtr =  readObjectAMF0();
		break;
	case 0x05:
		objPtr =  readNullAMF0();
		break;
	case 0x11://AMF3
		objPtr =  decode_AMF3();
		break;
	}
	return objPtr;
}

std::shared_ptr<Amf0_object> amfDecoder::readIntAMF0()
{
	Amf0_object *obj = new Amf0_object;
	std::shared_ptr<Amf0_object> objPtr;
	objPtr.reset(obj);
	obj->type = AMF0_NUMBER;
	obj->_value._double = readDouble();
	return objPtr;
}

std::shared_ptr<Amf0_object> amfDecoder::readStringAMF0()
{
	Amf0_object *obj = new Amf0_object;
	std::shared_ptr<Amf0_object> objPtr;
	objPtr.reset(obj);
	obj->type = AMF0_STRING;
	int len = ((readByteAsInt() << 8) + readByteAsInt());
	if (0 == len)
	{
		obj->text = "";
		return objPtr;
	}
	obj->text = std::string((char*)readBytes(len).get(),len);
	return objPtr;
}

std::shared_ptr<Amf0_object> amfDecoder::readNullAMF0()
{
	Amf0_object *obj = new Amf0_object;
	std::shared_ptr<Amf0_object> objPtr;
	objPtr.reset(obj);
	obj->type = AMF0_NULL;
	return objPtr;
}

std::shared_ptr<Amf0_object> amfDecoder::readObjectAMF0()
{
	Amf0_object *obj = new Amf0_object;
	std::shared_ptr<Amf0_object> objPtr;
	objPtr.reset(obj);
	std::string key = readStringAMF0().get()->text;
	while (!key.empty())
	{
		unsigned char b = readByte();
		if (b == 0x00)
		{
			obj->result.insert(std::make_pair(key, readIntAMF0()));
		}
		else if (b == 0x02)
		{
			obj->result.insert(std::make_pair(key, readStringAMF0()));
		}
		else if (b == 0x05)
		{
			obj->result.insert(std::make_pair(key, readNullAMF0()));
		}
		key = readStringAMF0().get()->text;
	}
	readByte();
	return objPtr;
}

std::shared_ptr<Amf_Object> amfDecoder::decodeInvoke(unsigned char* _data, unsigned int _len)
{
	try{
		reset();
		m_dataPos = 0;
		m_dataBuffer = new unsigned char[_len + 1];
		memset(m_dataBuffer, 0, _len + 1);
		
		int m = 0;
		for (int i = 0; i < _len; i++)
		{
			if (0 == (1 + i) % 129) {
				continue;
			}

			m_dataBuffer[m] = _data[i];
			m++;
		}
		m_dataSize = m;

		readByte();
		std::shared_ptr<Amf_Object> objPtr(new Amf_Object);
		objPtr->result.insert(std::make_pair("result", decode_AMF0()));
		objPtr->result.insert(std::make_pair("invokeId", decode_AMF0()));
		objPtr->result.insert(std::make_pair("serviceCall", decode_AMF0()));
		objPtr->result.insert(std::make_pair("data", decode_AMF0()));
		return objPtr;
	}
	catch (...)
	{
		tools::getInstance()->message("解析包出现异常！\n");
	}
	return nullptr;
}

std::shared_ptr<Amf_Object> amfDecoder::decodeConnect(unsigned char* _data, unsigned int _len)
{
	reset();

	m_dataPos = 0;
	m_dataBuffer = new unsigned char[_len + 1];
	memset(m_dataBuffer, 0, _len + 1);
	int m = 0;
	for (int i = 0; i < _len; i ++)
	{
		if (0 == (1 + i) % 129) {
			continue;
		}
		m_dataBuffer[m] = _data[i];
		m++;
	}
	m_dataSize = m;

	std::shared_ptr<Amf_Object> objPtr(new Amf_Object);
	objPtr->result.insert(std::make_pair("result", decode_AMF0()));
	objPtr->result.insert(std::make_pair("invokeId", decode_AMF0()));
	objPtr->result.insert(std::make_pair("serviceCall", decode_AMF0()));
	objPtr->result.insert(std::make_pair("data", decode_AMF0()));
	return objPtr;
}

Amf_Object::Amf_Object()
{
	amfType = TYPE_BASE;
}

Amf_Object::~Amf_Object()
{
	if (!temp.empty())
	{
		temp.clear();
	}
	if (!result.empty())
	{
		result.empty();
	}
}

Amf0_object::Amf0_object()
{
	amfType = TYPE_AMF0;
}

Amf0_object::~Amf0_object()
{

}

Amf3_object::Amf3_object()
{
	amfType = TYPE_AMF3;
}

Amf3_object::~Amf3_object()
{

}

ClassDefinition::ClassDefinition()
{

}

ClassDefinition::~ClassDefinition()
{

}
