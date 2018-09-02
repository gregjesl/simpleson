#include "json.h"
#include <cassert>
#include <math.h>

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

	json::jobject result = json::jobject::parse(input);
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

	// Create a JSON object
	json::jobject test;
	test["int"] = 123;
	test["float"] = 12.3f;
	test["string"] = "test \"string";
	int test_array[3] = { 1, 2, 3 };
	test["array"] = std::vector<int>(test_array, test_array + 3);

	json::jobject subobj;
	char world[6] = "world";
	subobj["hello"] = world;
	test["subobj"] = subobj;

	std::string serial = (std::string)test;
	json::jobject retest = json::jobject::parse(serial);
	assert((int)retest["int"] == 123);
	assert(fabs((float)retest["float"] - 12.3f) < 1.0e-6);
	assert(retest["string"] == "test \"string");
	std::vector<int> retest_array = retest["array"];
	assert(retest_array == std::vector<int>(test_array, test_array + 3));
	json::jobject resubobj = test["subobj"];
	assert(resubobj["hello"] == "world");
}