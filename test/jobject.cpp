#include "json.h"
#include <assert.h>

int main(void)
{
	json::jobject result = json::jobject::parse("{\"key1\":\"test\",\"key2\":123, \"nulltest\":null, \"booltest\":true, \"arraytest\":[1,2,3], \"objecttest\":{\"key\":\"value\"}}");
	assert(result.has_key("key1"));
	assert(result[0].key.compare("key1") == 0);
	assert(result.get_entry("key1").value.get_type() == json::jtype::jstring);
	assert(((std::string)result[0].value).compare("test") == 0);

	assert(result.has_key("key2"));
	assert(result[1].key.compare("key2") == 0);
	assert(result.get_entry("key2").value.get_type() == json::jtype::jnumber);
	assert((int)result[1].value == 123);

	assert(result[2].key.compare("nulltest") == 0);
	assert(result.get_entry("nulltest").value.get_type() == json::jtype::jnull);
	assert(((std::string)result[2].value).compare("null") == 0);

	assert(result[3].key.compare("booltest") == 0);
	assert(result[3].value.get_type() == json::jtype::jbool);
	assert((bool)result[3].value);

	assert(result[4].key.compare("arraytest") == 0);
	assert(result[4].value.get_type() == json::jtype::jarray);

	assert(result[5].key.compare("objecttest") == 0);
	assert(result[5].value.get_type() == json::jtype::jobject);
	
	std::string obj_str = (std::string)result;
	result = json::jobject::parse(obj_str);
	assert(result[0].key.compare("key1") == 0);
	assert(result[0].value.get_type() == json::jtype::jstring);
	assert(((std::string)result[0].value).compare("test") == 0);

	assert(result[1].key.compare("key2") == 0);
	assert(result[1].value.get_type() == json::jtype::jnumber);
	assert((int)result[1].value == 123);

	assert(result[2].key.compare("nulltest") == 0);
	assert(result[2].value.get_type() == json::jtype::jnull);
	assert(((std::string)result[2].value).compare("null") == 0);

	assert(result[3].key.compare("booltest") == 0);
	assert(result[3].value.get_type() == json::jtype::jbool);
	assert((bool)result[3].value);

	assert(result[4].key.compare("arraytest") == 0);
	assert(result[4].value.get_type() == json::jtype::jarray);

	assert(result[5].key.compare("objecttest") == 0);
	assert(result[5].value.get_type() == json::jtype::jobject);

	assert(result.has_key("not a key") == false);

	// Remove an entry
	result.remove_entry("key1");
	assert(!result.has_key("key1"));
}