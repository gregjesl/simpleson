#include "json.h"
#include "assert.h"

int main(void)
{
	json::parsing::parse_results result;
	const char *input = NULL;

	// Parse string
	input = " \t \n \v \f \r \"abc123 \\\"\"";
	result = json::parsing::parse(input);
	assert(result.type == json::jtype::jstring);
	assert(result.value == "abc123 \\\"");

	// Parse numbers
	const char *numbers[] = { "123", "-123", "123.456", "-123.456", "123e456", "123e+456", "123e-456", "123.456e789", "123.456e+789", "123.456e-789" };
	for (int i = 0; i < 10; i++)
	{
		input = numbers[i];
		result = json::parsing::parse(input);
		assert(result.type == json::jtype::jnumber);
		assert(result.value == numbers[i]);
	}

	// Parse empty array
	input = " []";
	result = json::parsing::parse(input);
	assert(result.type == json::jtype::jarray);
	assert((std::string)result.value == "[]");

	// Parse string array
	input = " [\"world\"]},";
	result = json::parsing::parse(input);
	assert(result.type == json::jtype::jarray);
	assert((std::string)result.value == "[\"world\"]");

	// Parse object
	input = " {\"hello\":\"world\"},";
	result = json::parsing::parse(input);
	assert(result.type == json::jtype::jobject);
	assert(result.value == "{\"hello\":\"world\"}");

	// Parse array
	input = " [{\"hello\":\"world\"},{\"hello\":\"world\"},{\"hello\":\"world\"}],";
	result = json::parsing::parse(input);
	assert(result.type == json::jtype::jarray);
	assert(result.value == "[{\"hello\":\"world\"},{\"hello\":\"world\"},{\"hello\":\"world\"}]");

	// Parse boolean
	const char* bools[] = { "true", "false" };
	for (int i = 0; i < 2; i++)
	{
		input = bools[i];
		result = json::parsing::parse(input);
		assert(result.type == json::jtype::jbool);
		assert(result.value == bools[i]);
	}

	// Parse null
	input = "null";
	result = json::parsing::parse(input);
	assert(result.type == json::jtype::jnull);
	assert(result.value == "null");

	// Tryparse
	json::jobject tryparse_result;
	assert(json::jobject::tryparse("not json", tryparse_result));
	assert(!json::jobject::tryparse("{\"hello\":\"world\"}", tryparse_result));
	assert("{\"hello\":\"world\"}" == (std::string)tryparse_result);
}