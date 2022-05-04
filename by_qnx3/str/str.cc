#include <cstdlib>
#include <iostream>
#include <stdio.h>
#include "../Base/CString.h"

int main(int argc, char *argv[])
{
	CString str;
	str = "1234567";

	//printf("%s\b", str);
	printf("%s %d\n", (const char*)str, std::string::npos);
	std::cout << "Welcome to the QNX Momentics IDE" << std::endl;
	return EXIT_SUCCESS;
}
