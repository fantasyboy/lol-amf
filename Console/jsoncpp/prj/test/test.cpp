#include "stdafx.h"

#include <fstream>
#include <iostream>

void testReader()
{
	std::ifstream in("myjson.json");

	Json::Reader reader;
	Json::Value root;
	if(!reader.parse(in, root)) {
		std::cout << reader.getFormattedErrorMessages() << std::endl;
		return;
	}

	std::cout << root.toStyledString();
}

void testWriter()
{
	std::ofstream out("myjsonWriter.json");

	Json::Value root;
	Json::Value arrayObj;
	Json::Value item;
	for(int i = 0; i < 4; ++i){
		item["key"] = i;
		arrayObj.append(item);
	}

	root["key1"] = "value1";
	root["key2"] = "value2";
	root["array"] = arrayObj;
	std::cout << root.toStyledString();

	out.write(root.toStyledString().c_str(), root.toStyledString().length());
}

int _tmain(int argc, _TCHAR* argv[])
{

	testReader();
	testWriter();

	getchar();
	return 0;
}

