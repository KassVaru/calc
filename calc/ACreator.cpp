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

void ACreator::ListFeatures()
{
	Console.Log("Lista dost�pnych wyra�e�\n", Console.WHITE);
	Sleep(2000);
	system("PAUSE");
}

void ACreator::GetUserExpression()
{
	Console.Log("Wprowad� wyra�enie: ", Console.WHITE); expression = Console.Get(Console.AQUA);
	AAnalyzer analyze(expression);
	Sleep(2000);
}
