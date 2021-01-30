#include "Console.h"
#include <windows.h>
#include <string>

void Console::Log(string text, Colors color)
{
	SetConsoleTextAttribute(handle, SetColor(color));
	cout << text;
	SetConsoleTextAttribute(handle, Default());
}

void Console::ChangeColor(Colors color)
{
	SetConsoleTextAttribute(handle, SetColor(color));
}

int Console::SetColor(Colors color)
{
	string clr = "";
	switch (color) {
		case BLUE: return 1;
		case GREEN: return 2;
		case AQUA: return 3;
		case RED: return 4;
		case PURPLE: return 5;
		case YELLOW: return 6;
		case WHITE: return 7;
		case GRAY: return 8;
	}
}
string Console::Get(Colors color)
{
	ChangeColor(color);
	string text = ""; getline(cin, text);
	ChangeColor(Console::WHITE);
	return text;
}
int Console::Default()
{
	return 7;
}
