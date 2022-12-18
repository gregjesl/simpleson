#include "json.h"
#include "test.h"
#include <math.h>

int main(void)
{
	const char *input =
		"[\n"
		    "\t123.456,\n"
		    "\t\"hello \\\" world\",\n"
		    "\t[\n"
                "\t\t1,\n"
                "\t\t2,\n"
                "\t\t3\n"
            "\t],\n"
			"\ttrue,\n"
			"\tnull,\n"
			"\t[\n"
                "\t\t{\n"
                    "\t\t\t\"key\": \"value\"\n"
                "\t\t}\n"
            "\t],\n"
			"\t[\n"
                "\t\t\"hello\",\n"
                "\t\t\"world\"\n"
            "\t],\n"
			"\t[]\n"
		"]";

	printf("Input: \n%s\n", input);

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

	printf("Output\n%s\n", result.pretty().c_str());

	// Test serialization
	TEST_STRING_EQUAL(result.pretty().c_str(), input);

	// Test copy constructor
	json::jarray copy(result);
	TEST_STRING_EQUAL(copy.as_string().c_str(), result.as_string().c_str());
}