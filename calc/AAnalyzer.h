#include <iostream>
#include <string>
#include "Console.h"
#include "CalcLib.h"

using namespace std;

#pragma once
class AAnalyzer
{
public:
	AAnalyzer(string expr);
private:
	Console Console;
};