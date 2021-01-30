#include <iostream>
#include <windows.h>
using namespace std;
#pragma once
class Console
{
public:
	enum Colors {
		BLUE,
		GREEN,
		AQUA,
		RED,
		PURPLE,
		YELLOW,
		WHITE,
		GRAY
	};
	void Log(string text, Colors color);
	void ChangeColor(Colors color);
	string Get(Colors color);
private:
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
	int SetColor(Colors color);
	int Default();
};

