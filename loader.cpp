//#include <iostream>
#include <windows.h>
#include <stdio.h>
//using namespace std;

int main(void)
{
	HMODULE hMod = LoadLibrary("shellcode.dll");
	//HMODULE hMod = nullptr;
	if( hMod == nullptr )
	{
		//std::cout << "Failed to load DLL" << std::endl;
		printf("Failed to load DLL\n");
	}
	
	return 0;
}
