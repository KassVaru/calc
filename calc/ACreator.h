#include <iostream>
#include "AAnalyzer.h"
#include "Console.h"
using namespace std;
#pragma once
class ACreator
{
public:
	string expression = "";
	double result;
	ACreator();
private:
	Console Console;
	int Menu();
	void ListFeatures();
	void GetUserExpression();
};

