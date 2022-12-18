#include "json.h"
#include <stdio.h>
#include <assert.h>

int main(void)
{
	// Create a couple objects
    json::jobject obj1 = json::jobject::parse("{\"hello\":\"world\"}");
    json::jobject obj2;
    obj2["key"] = "value";

    // Put the objects in a vector
    std::vector<json::jobject> vec;
    vec.push_back(obj1);
    vec.push_back(obj2);

    // Create an object and add the array
    json::jobject example;
    example["array"] = vec;

    // Print the result
    printf("%s\n", example.pretty().c_str());

    // Access each item
    assert(example["array"].as_array().at(0).as_object() == obj1);
    assert(example["array"].as_array().at(1).as_object() == obj2);

    // Read back the entire array
    std::vector<json::jobject> readbackarray = example["array"].as_array();

    // Access each item
    json::jobject readback1 = readbackarray[0];
    assert(readback1 == obj1);
    json::jobject readback2 = readbackarray[1];
    assert(readback2 == obj2);
}