#include "json.h"
#include "assert.h"

int main(void)
{
	json::parsing::parse_results result;
	std::string input;

	// Parse string
	input = " \t \n \v \f \r \"abc123 \\\"\"";
	result = json::parsing::parse(input);
	assert(result.type == json::jtype::jstring);
	assert(result.value == "abc123 \\\"");

	// Parse numbers
	const std::string numbers[] = { "123", "-123", "123.456", "-123.456", "123e456", "123e+456", "123e-456", "123.456e789", "123.456e+789", "123.456e-789" };
	for (int i = 0; i < 10; i++)
	{
		input = numbers[i];
		result = json::parsing::parse(input);
		assert(result.type == json::jtype::jnumber);
		assert(result.value == numbers[i]);
	}
}