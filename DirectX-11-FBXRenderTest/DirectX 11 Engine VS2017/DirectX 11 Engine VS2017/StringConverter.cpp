#include "StringConverter.h"

std::wstring StringConverter::StringToWide(std::string str)
{
	std::wstring wide_string(str.begin(), str.end());
	return wide_string;
}

int StringConverter::StrCmpNoCase(const char* s1, const char* s2)
{
	const unsigned char* us1 = (const unsigned char*)s1;
	const unsigned char* us2 = (const unsigned char*)s2;
	unsigned char tolower1 = 1;
	unsigned char tolower2 = 1;

	while (tolower1 == tolower2)
	{
		if (tolower1 == 0)
		{
			return 0;
		}
		else
		{
			tolower1 = *us1++;
			tolower2 = *us2++;
			if (tolower1 >= 65 && tolower1 <= 90)
			{
				tolower1 += 32;
			}
			if (tolower2 >= 65 && tolower2 <= 90)
			{
				tolower2 += 32;
			}
		}
	}
	return (tolower1 - tolower2);
}
