#include "json.h"
#include <assert.h>
#include <string>

int main(void)
{
	std::string input = " 123457890";
	assert(json::parsing::read_digits(input) == "123457890");
	assert(input == "");
	input = " 123457890a";
	assert(json::parsing::read_digits(input) == "123457890");
	assert(input == "a");
}