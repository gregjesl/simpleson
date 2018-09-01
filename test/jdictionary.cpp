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
	assert(result["number"] == "123.456");
	assert(result["string"] == "\"hello \\\" world\"");
	assert(result["array"] == "[1,2,3]");
	assert(result["boolean"] == "true");
	assert(result["isnull"] == "null");
	assert(result["objarray"] == "[{\"key\":\"value\"}]");
	assert(result.has_key("number"));
	assert(!result.has_key("nokey"));

	// Assign some new values
	result["newvalue"] = "789";
	assert(result["newvalue"] == "789");
}