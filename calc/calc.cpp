#include <iostream>
#include <locale.h>
#include <tchar.h>
#include "ACreator.h"
#include "AAnalyzer.h"
#include "CalcLib.h"

using namespace std;

int main()
{
	setlocale(LC_ALL, "");
	SetConsoleTitle(_T("Projekt Calc - Piotr Wasielewski"));
	ACreator creator;
}