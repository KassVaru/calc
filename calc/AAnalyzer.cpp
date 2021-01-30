#include "AAnalyzer.h"
#include "Console.h"

AAnalyzer::AAnalyzer(string expr)
{
	double result = interpretate(expr.c_str(), 0);
	Console.Log(to_string(result), Console.WHITE);
}
