#include "json.h"
#include <assert.h>
#include <string>

int main(void)
{
	const char *input = " 123457890";
	assert(json::parsing::read_digits(input) == "123457890");
	input = " 123457890a";
	assert(json::parsing::read_digits(input) == "123457890");
}