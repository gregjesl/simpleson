#include "json.h"
#include "assert.h"

int main(void)
{
	json::jobject obj1;
	obj1["key1"] = "value1";
	json::jobject obj2;
	obj2["key2"] = "value2";

	obj1 += obj2;

	assert(obj1.size() == 2);
	assert(obj1.has_key("key1"));
	assert(obj1["key1"] == "value1");
	assert(obj1.has_key("key2"));
	assert(obj1["key2"] == "value2");

	json::jobject obj3;
	obj3["key3"] = "value3";

	json::jobject obj4 = obj1 + obj3;
	assert(obj4.size() == 3);
	assert(obj4.has_key("key1"));
	assert(obj4["key1"] == "value1");
	assert(obj4.has_key("key2"));
	assert(obj4["key2"] == "value2");
	assert(obj4.has_key("key3"));
	assert(obj4["key3"] == "value3");
}