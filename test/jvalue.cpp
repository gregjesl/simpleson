#include "json.h"
#include <assert.h>

int main(void)
{
	json::jvalue value;

	// String
	value = std::string("Hello world");
	assert(((std::string)value).compare("Hello world") == 0);

	json::basic::istringnumber test = 123;
	// Number
	value = 123;
	assert(((int)value) == 123);

	// Boolean
	value = json::jvalue::parse("true");
	assert(value.get_type() == json::jtype::jbool);
	assert((bool)value);

	value = json::jvalue::jbool(true);
	assert(value.get_type() == json::jtype::jbool);
	assert((bool)value);

	value = json::jvalue::parse("false");
	assert(value.get_type() == json::jtype::jbool);
	assert(!(bool)value);

	value = json::jvalue::jbool(false);
	assert(value.get_type() == json::jtype::jbool);
	assert(!(bool)value);

	// Array
	value = json::jarray::parse("[1,2,3]");
	std::vector<int> cast_vec = value;
	assert(cast_vec.at(0) == 1);
	assert(cast_vec.at(1) == 2);
	assert(cast_vec.at(2) == 3);

	// Object
	value = json::jvalue::parse("{\"hello\":\"world\"}");
	json::jobject obj_new = (std::string)value;
	assert(obj_new.has_key("hello"));
	assert(((std::string)obj_new.get_entry("hello").value).compare("world") == 0);
}