#include "json.h"
#include "test.h"

int main(void)
{
	// String
	TEST_EQUAL(json::jtype::peek('"'), json::jtype::jstring);

	// Number
	for(char i = '0'; i <= '9'; i++) {
		TEST_EQUAL(json::jtype::peek(i), json::jtype::jnumber);
	}
	TEST_EQUAL(json::jtype::peek('-'), json::jtype::jnumber);

	// Object
	TEST_EQUAL(json::jtype::peek('{'), json::jtype::jobject);

	// Array
	TEST_EQUAL(json::jtype::peek('['), json::jtype::jarray);

	// Bool
	TEST_EQUAL(json::jtype::peek('t'), json::jtype::jbool);
	TEST_EQUAL(json::jtype::peek('f'), json::jtype::jbool);

	// Null
	TEST_EQUAL(json::jtype::peek('n'), json::jtype::jnull);

	// Invalid
	for(char i = 'A'; i <= 'Z'; i++) {
		TEST_EQUAL(json::jtype::peek(i), json::jtype::not_valid);
	}
}