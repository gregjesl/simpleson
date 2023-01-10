#include "json.h"
#include "test.h"
#include <math.h>

bool object_open = false;
size_t keys_detected = 0;
size_t values_detected = 0;

class test_istream : public json::jobject::istream
{
protected:
    inline virtual void on_object_opened()
    {
        object_open = true;
    }
    virtual void on_key_read(const std::string &key, const json::jtype::jtype type)
    {
        TEST_EQUAL(type, json::jtype::jstring);
        switch (keys_detected)
        {
        case 0:
            TEST_STRING_EQUAL(key.c_str(), "key");
            break;
        case 1:
            TEST_STRING_EQUAL(key.c_str(), "hello");
            break;
        default:
            TEST_ERROR
            break;
        }
        keys_detected++;
    }
    virtual void on_value_read(const std::string &key, const json::data_reference &value)
    {
        switch (values_detected)
        {
        case 0:
            TEST_STRING_EQUAL(key.c_str(), "key");
            TEST_STRING_EQUAL(value.as_string().c_str(), "value");
            break;
        case 1:
            TEST_STRING_EQUAL(key.c_str(), "hello");
            TEST_STRING_EQUAL(value.as_string().c_str(), "world");
            break;
        default:
            TEST_ERROR
            break;
        }
        values_detected++;
    }
    virtual void on_object_closed()
    {
        object_open = false;
    }
};

int main(void)
{
	const char *input = "{\"key\":\"value\",\"hello\":\"world\"}";
    const char *read = input;
    test_istream stream;

    TEST_FALSE(object_open);
    TEST_FALSE(stream.is_valid());

    stream.push(*read++);
    TEST_TRUE(object_open);
    TEST_EQUAL(stream.bytes_accepted(), 1);

    for(size_t i = 1; i < strlen(input) - 1; i++)
    {
        TEST_EQUAL(stream.push(*read++), json::jistream::ACCEPTED);
        TEST_FALSE(stream.is_valid());
        TEST_EQUAL(stream.bytes_accepted(), i + 1);
    }

    stream.push(*read);
    TEST_FALSE(object_open);
    TEST_TRUE(stream.is_valid());
    TEST_EQUAL(stream.bytes_accepted(), strlen(input));
}