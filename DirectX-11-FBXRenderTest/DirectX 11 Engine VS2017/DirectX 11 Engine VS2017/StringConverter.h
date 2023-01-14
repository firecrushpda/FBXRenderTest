#pragma once
#include <string>

class StringConverter
{
public:
	static std::wstring StringToWide(std::string str);

	static int StrCmpNoCase(const char* s1, const char* s2);
	
};