#include "json.h"
#include "test.h"
#include <string>

int main(void)
{
    json::jobject test;
    const char * input = "\" \\ / \b \f \n \r \t";
    test["chars"] = input;
    std::string serial = test.as_string();
    const char *expected_result = "{\"chars\":\"\\\" \\\\ \\/ \\b \\f \\n \\r \\t\"}";
    TEST_STRING_EQUAL(serial.c_str(), expected_result);
    std::string echo = test["chars"];
    TEST_STRING_EQUAL(echo.c_str(), input);
}