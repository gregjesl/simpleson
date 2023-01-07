#include "json.h"
#include "test.h"
#include <math.h>

int main(void)
{
	const char *input = "{\"hello\":\"world\",\"key\":\"value\"}";
    const char *read = input;
    json::jobject::parser test;

    TEST_EQUAL(test.type(), json::jtype::jobject);
    TEST_FALSE(test.is_valid());

    for(size_t i = 0; i < strlen(input) - 1; i++)
    {
        TEST_EQUAL(test.push(*read++), json::jistream::ACCEPTED);
        TEST_FALSE(test.is_valid());
    }

    test.push(*read);
    TEST_TRUE(test.is_valid());

    TEST_STRING_EQUAL(test.result().serialize().c_str(), input);
    TEST_STRING_EQUAL(test.emit().as_object().serialize().c_str(), input);
}