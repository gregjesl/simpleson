#include "json.h"
#include "assert.h"

int main(void)
{
	// Test parsing
	std::string test_string = " \t \n \v \f \r abc123";
	json::parsing::tlws(test_string);
	assert(test_string == std::string("abc123"));
	json::parsing::tlws(test_string);
	assert(test_string == std::string("abc123"));
	test_string = " \t \n \v \f \r";
	json::parsing::tlws(test_string);
	assert(test_string == std::string(""));


}