#include "json.h"
#include "test.h"
#include <math.h>

bool array_opened = false;
size_t values_detected = 0;

class test_istream : public json::jarray::istream
{
protected:
    inline virtual void on_array_opened()
    {
        array_opened = true;
    }
    virtual void on_value_read(const json::data_reference &value)
    {
        switch (values_detected)
        {
        case 0:
            TEST_STRING_EQUAL(value.as_string().c_str(), "hello");
            break;
        case 1:
            TEST_STRING_EQUAL(value.as_string().c_str(), "world");
            break;
        default:
            TEST_ERROR
            break;
        }
        values_detected++;
    }
    virtual void on_array_closed()
    {
        array_opened = false;
    }
};

int main(void)
{
	const char *input = "[\"hello\",\"world\"]";
    const char *read = input;
    test_istream stream;

    TEST_FALSE(array_opened);
    TEST_EQUAL(stream.type(), json::jtype::jarray);
    TEST_FALSE(stream.is_valid());

    stream.push(*read++);
    TEST_TRUE(array_opened);
    TEST_EQUAL(stream.bytes_accepted(), 1);

    for(size_t i = 1; i < strlen(input) - 1; i++)
    {
        TEST_EQUAL(stream.push(*read++), json::jistream::ACCEPTED);
        TEST_FALSE(stream.is_valid());
        TEST_EQUAL(stream.bytes_accepted(), i + 1);
    }

    stream.push(*read);
    TEST_FALSE(array_opened);
    TEST_TRUE(stream.is_valid());
    TEST_EQUAL(stream.bytes_accepted(), strlen(input));
}