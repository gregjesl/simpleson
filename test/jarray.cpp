#include "json.h"
#include "test.h"
#include <math.h>

int main(void)
{
	const char *input =
		"["
			"123.456,"
			"\"hello \\\" world\","
			"[1,2,3],"
			"true,"
			"null,"
			"[{\"key\":\"value\"}],"
			"[\"hello\",\"world\"],"
			"[]"
		"]";

	json::jarray result = json::jarray::parse(input);
	TEST_STRING_EQUAL(result.at(0).as_string().c_str(), "123.456");
	TEST_STRING_EQUAL(result.at(1).as_string().c_str(), "hello \" world");
	TEST_STRING_EQUAL(result.at(2).as_string().c_str(), "[1,2,3]");
	TEST_STRING_EQUAL(result.at(3).as_string().c_str(), "true");
	TEST_STRING_EQUAL(result.at(4).as_string().c_str(), "null");
	TEST_STRING_EQUAL(result.at(5).as_string().c_str(), "[{\"key\":\"value\"}]");
	TEST_STRING_EQUAL(result.at(6).as_string().c_str(), "[\"hello\",\"world\"]");
	TEST_STRING_EQUAL(result.at(7).as_string().c_str(), "[]");

	TEST_TRUE(result.at(3).is_true());
	TEST_TRUE(result.at(4).is_null());
	TEST_TRUE(result.at(5).as_array().at(0).as_object() == json::jobject::parse("{\"key\":\"value\"}"));

	// Test serialization
	TEST_STRING_EQUAL(result.as_string().c_str(), input);

	// Test copy constructor
	json::jarray copy(result);
	TEST_STRING_EQUAL(copy.as_string().c_str(), result.as_string().c_str());
}