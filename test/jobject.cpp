#include "json.h"
#include "test.h"
#include <math.h>

int main(void)
{
	const char *input =
		"{"
		"	\"number\":123.456,"
		"	\"string\":\"hello \\\" world\","
		"	\"array\":[1,2,3],"
		"	\"boolean\":true,"
		"	\"isnull\":null,"
		"	\"objarray\":[{\"key\":\"value\"}],"
		"	\"strarray\":[\"hello\",\"world\"],"
		"	\"emptyarray\":[]"
		"}";

	json::jobject result = json::jobject::parse(input);
	TEST_STRING_EQUAL(result.get("number").c_str(), "123.456");
	TEST_STRING_EQUAL(result.get("string").c_str(), "\"hello \\\" world\"");
	TEST_STRING_EQUAL(result.get("array").c_str(), "[1,2,3]");
	TEST_STRING_EQUAL(result.get("boolean").c_str(), "true");
	TEST_STRING_EQUAL(result.get("isnull").c_str(), "null");
	TEST_STRING_EQUAL(result.get("objarray").c_str(), "[{\"key\":\"value\"}]");
	TEST_STRING_EQUAL(result.get("strarray").c_str(), "[\"hello\",\"world\"]");
	TEST_STRING_EQUAL(result.get("emptyarray").c_str(), "[]");
	TEST_TRUE(result.has_key("number"));
	TEST_FALSE(result.has_key("nokey"));
	std::vector<std::string> strarray = result["strarray"];
	TEST_EQUAL(strarray.size(), 2);
	TEST_STRING_EQUAL(strarray[0].c_str(), "hello");
	TEST_STRING_EQUAL(strarray[1].c_str(), "world");
	std::vector<std::string> emptyarray = result["emptyarray"];
	TEST_EQUAL(emptyarray.size(), 0);

	// Assign some new values
	result.set("newvalue", "789");
	TEST_STRING_EQUAL(result.get("newvalue").c_str(), "789");
	result.set("array", "[4,5,6]");
	TEST_STRING_EQUAL(result.get("array").c_str(), "[4,5,6]");

	// Create a JSON object
	json::jobject test;
	test["int"] = 123;
	test["float"] = 12.3f;
	test["string"] = "test \"string";
	int test_array[3] = { 1, 2, 3 };
	test["array"] = std::vector<int>(test_array, test_array + 3);
	std::string test_string_array[2] = { "hello", "world" };
	test["strarray"] = std::vector<std::string>(test_string_array, test_string_array + 2);
	test["emptyarray"] = std::vector<std::string>();
	test["boolean"].set_boolean(true);
	test["null"].set_null();

	json::jobject subobj;
	char world[6] = "world";
	subobj["hello"] = world;
	test["subobj"] = subobj;

	std::vector<json::jobject> objarray;
	objarray.push_back(subobj);
	objarray.push_back(subobj);
	test["objarray"] = objarray;

	std::string serial = (std::string)test;
	json::jobject retest = json::jobject::parse(serial.c_str());
	TEST_EQUAL((int)retest["int"], 123);
	TEST_TRUE(fabs((float)retest["float"] - 12.3f) < 1.0e-6);
	TEST_STRING_EQUAL(retest["string"].as_string().c_str(), "test \"string");
	std::vector<int> retest_array = retest["array"];
	TEST_TRUE(retest_array == std::vector<int>(test_array, test_array + 3));
	json::jobject resubobj = test["subobj"];
	TEST_STRING_EQUAL(resubobj["hello"].as_string().c_str(), "world");
	strarray = retest["strarray"];
	TEST_EQUAL(strarray.size(), 2);
	TEST_STRING_EQUAL(strarray[0].c_str(), "hello");
	TEST_STRING_EQUAL(strarray[1].c_str(), "world");
	emptyarray = retest["emptyarray"];
	TEST_EQUAL(emptyarray.size(), 0);
	TEST_TRUE(test.has_key("boolean"));
	TEST_TRUE(test["boolean"].is_true());
	TEST_TRUE(test.has_key("null"));
	TEST_TRUE(test["null"].is_null());
	TEST_FALSE(test["boolean"].is_null());
	std::vector<json::jobject> objarrayecho = test["objarray"];
	TEST_EQUAL(objarrayecho.size(), 2);
}