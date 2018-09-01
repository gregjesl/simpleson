#include "json.h"
#include <cassert>

int main(void)
{
	std::string input =
		"{"
		"	\"number\":123.456,"
		"	\"string\":\"hello \\\" world\","
		"	\"array\":[1,2,3],"
		"	\"boolean\":true,"
		"	\"isnull\":null,"
		"	\"objarray\":[{\"key\":\"value\"}]"
		"}";

	json::jdictionary result = json::jdictionary::parse(input);
	assert(result.get("number") == "123.456");
	assert(result.get("string") == "\"hello \\\" world\"");
	assert(result.get("array") == "[1,2,3]");
	assert(result.get("boolean") == "true");
	assert(result.get("isnull") == "null");
	assert(result.get("objarray") == "[{\"key\":\"value\"}]");
	assert(result.has_key("number"));
	assert(!result.has_key("nokey"));

	// Assign some new values
	result.set("newvalue", "789");
	assert(result.get("newvalue") == "789");
	result.set("array", "[4,5,6]");
	assert(result.get("array") == "[4,5,6]");
}