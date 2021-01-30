#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
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
	wifstream plik;
	vector<vector<wstring>> features;
	int Menu();
	void ListFeatures();
	void GetUserExpression();
	void ShowFeaturesFromFile();
	vector<wstring> parseData(wstring line);
};

