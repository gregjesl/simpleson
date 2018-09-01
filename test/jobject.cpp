#include "json.h"

int main(void)
{
	// Create a JSON object
	json::jobject test;
	test["int"] = 123;
	test["float"] = 12.3f;
	test["string"] = "test \"string";
	int test_array[3] = { 1, 2, 3 };
	test["array"] = std::vector<int>(test_array, test_array + 3);

	json::jobject subobj;
	subobj["hello"] = "world";
	test["subobj"] = subobj;

	std::string serial = (std::string)test;



}