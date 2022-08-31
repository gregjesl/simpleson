#include "json.h"
#include "test.h"
#include <stdio.h>

int main(void)
{
    const char *input = "ΛΛΛ";
    json::utf8_string test;
    test = input;
    const std::string result = test.to_string();
    TEST_STRING_EQUAL(&result[0], input);
}