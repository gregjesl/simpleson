#include "json.h"
#include "assert.h"

int main(void)
{
	// Test parsing
	const char *test_string = " \t \n \v \f \r abc123";
	assert(std::string(json::parsing::tlws(test_string)) == std::string("abc123"));
	json::parsing::tlws(test_string);
	assert(std::string(json::parsing::tlws(test_string)) == std::string("abc123"));
	test_string = " \t \n \v \f \r";
	assert(std::string(json::parsing::tlws(test_string)) == std::string(""));
}