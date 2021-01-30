#include "ACreator.h"
#include "AAnalyzer.h"
#include "Console.h"
#include <iostream>
#include <windows.h>
#include <stdlib.h>

using namespace std;

ACreator::ACreator()
{
	while (true) {
		system("cls");
		int choose = Menu();
		if (choose == 1) GetUserExpression();
		else if (choose == 2) ListFeatures();
	}
}

int ACreator::Menu()
{
	Console.Log("Projekt \"Calc\"\n", Console.GREEN);
	Console.Log("Wybierz dzia�anie:\n--------------\n", Console.WHITE);
	Console.Log("[1]. Wprowad� dzia�anie\n[2]. Lista dost�pnych wyra�e�\n> ", Console.WHITE);
	string option = Console.Get(Console.AQUA);
	if (option == "1") return 1;
	else if (option == "2") return 2;
	else return 0;
}

vector<wstring> ACreator::parseData(wstring line)
{
	wstringstream ss(line);
	vector<wstring> result;
	while (ss.good()) {
		wstring data;
		getline(ss, data, L';');
		result.push_back(data);
	}
	return result;
}

void ACreator::ListFeatures()
{
	system("cls");
	Console.Log("Lista dost�pnych wyra�e�\n---\n", Console.GREEN);
	ShowFeaturesFromFile();
	Sleep(2000);
	system("PAUSE");
}

void ACreator::GetUserExpression()
{
	Console.Log("Wprowad� wyra�enie: ", Console.WHITE); expression = Console.Get(Console.AQUA);
	AAnalyzer analyze(expression);
	Sleep(2000);
	system("PAUSE");
}

void ACreator::ShowFeaturesFromFile()
{
	plik.open("features.txt");
	if (plik.is_open()) {
		wstring line;
		while (getline(plik, line)) {
			features.push_back(parseData(line));
		}
		for (int i = 0; i < features.size(); i++) {
			string name, desc, syntax, example;
			for (char x : features[i][0])
				name += x;
			for (char x : features[i][1])
				desc += x;
			for (char x : features[i][2])
				syntax += x;
			for (char x : features[i][3])
				example += x;

			Console.Log(name + " - ", Console.AQUA);
			Console.Log(desc + "\n", Console.WHITE);
			Console.Log("Sk�adnia: " + syntax + "\n", Console.GREEN);
			Console.Log("Przyk�ad: " + example + "\n", Console.WHITE);
			Console.Log("---\n", Console.GRAY);
		}
	}
	else {
		Console.Log("Otwarcie pliku by�o niemo�liwe!", Console.RED);
	}
	plik.close();
}
