#include "json.h"
#include <assert.h>

int main(void)
{
	// Empty
	assert(json::jtype::detect("") == json::jtype::not_valid);

	// Stirng
	assert(json::jtype::detect(" \"test string\"") == json::jtype::jstring);

	// Number
	assert(json::jtype::detect(" 123") == json::jtype::jnumber);
	assert(json::jtype::detect(" -123") == json::jtype::jnumber);

	// Object
	assert(json::jtype::detect(" {\"hello\":\"world\"") == json::jtype::jobject);

	// Array
	assert(json::jtype::detect(" [1,2,3]") == json::jtype::jarray);

	// Bool
	assert(json::jtype::detect(" true") == json::jtype::jbool);
	assert(json::jtype::detect(" false") == json::jtype::jbool);

	// Null
	assert(json::jtype::detect(" null") == json::jtype::jnull);

	// Invalid
	assert(json::jtype::detect(" abc") == json::jtype::not_valid);
	assert(json::jtype::detect(" fail") == json::jtype::not_valid);
	assert(json::jtype::detect(" test") == json::jtype::not_valid);
	assert(json::jtype::detect(" no") == json::jtype::not_valid);
}