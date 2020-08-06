#include "json.h"
#include "test.h"

int main(void)
{
	json::parsing::parse_results result;
	const char *input = NULL;

	// Parse string
	input = " \t \n \v \f \r \"abc123 \\\"\"";
	result = json::parsing::parse(input);
	TEST_EQUAL(result.type, json::jtype::jstring);
	TEST_STRING_EQUAL(result.value.c_str(), "abc123 \\\"");

	// Parse numbers
	const char *numbers[] = { "123", "-123", "123.456", "-123.456", "123e456", "123e+456", "123e-456", "123.456e789", "123.456e+789", "123.456e-789" };
	for (int i = 0; i < 10; i++)
	{
		input = numbers[i];
		result = json::parsing::parse(input);
		TEST_EQUAL(result.type, json::jtype::jnumber);
		TEST_STRING_EQUAL(result.value.c_str(), numbers[i]);
	}

	// Parse empty array
	input = " []";
	result = json::parsing::parse(input);
	TEST_EQUAL(result.type, json::jtype::jarray);
	TEST_STRING_EQUAL(result.value.c_str(), "[]");

	// Parse string array
	input = " [\"world\"]},";
	result = json::parsing::parse(input);
	TEST_EQUAL(result.type, json::jtype::jarray);
	TEST_STRING_EQUAL(result.value.c_str(), "[\"world\"]");

	// Parse object
	input = " {\"hello\":\"world\"},";
	result = json::parsing::parse(input);
	TEST_EQUAL(result.type, json::jtype::jobject);
	TEST_STRING_EQUAL(result.value.c_str(), "{\"hello\":\"world\"}");

	// Parse array
	input = " [{\"hello\":\"world\"},{\"hello\":\"world\"},{\"hello\":\"world\"}],";
	result = json::parsing::parse(input);
	TEST_EQUAL(result.type, json::jtype::jarray);
	TEST_STRING_EQUAL(result.value.c_str(), "[{\"hello\":\"world\"},{\"hello\":\"world\"},{\"hello\":\"world\"}]");

	// Parse boolean
	const char* bools[] = { "true", "false" };
	for (int i = 0; i < 2; i++)
	{
		input = bools[i];
		result = json::parsing::parse(input);
		TEST_EQUAL(result.type, json::jtype::jbool);
		TEST_STRING_EQUAL(result.value.c_str(), bools[i]);
	}

	// Parse null
	input = "null";
	result = json::parsing::parse(input);
	TEST_EQUAL(result.type, json::jtype::jnull);
	TEST_STRING_EQUAL(result.value.c_str(), "null");

	// Tryparse
	json::jobject tryparse_result;
	TEST_TRUE(json::jobject::tryparse("not json", tryparse_result));
	TEST_FALSE(json::jobject::tryparse("{\"hello\":\"world\"}", tryparse_result));
	TEST_STRING_EQUAL("{\"hello\":\"world\"}", tryparse_result.as_string().c_str());
}