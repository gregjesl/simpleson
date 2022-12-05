#include "json.h"
#include "test.h"
#include <math.h>

int main(void)
{
    int8_t small = 1;
    uint8_t usmall = 2;
    float dec = 3.14f;
    std::string str = "hello world";
    json::jobject obj;
    obj["hello"] = "world";

    json::jtranscriber test;
    test.jregister("small", &small);
    test.jregister("usmall", &usmall);
    test.jregister("dec", &dec);
    test.jregister("str", &str);
    test.jregister("obj", &obj);
    json::jobject serial = test.to_json();
    TEST_EQUAL((int8_t)serial["small"], small);
    TEST_EQUAL((uint8_t)serial["usmall"], usmall);
    TEST_TRUE((float)serial["dec"] > 3.1f && (float)serial["dec"] < 3.2f);
    TEST_STRING_EQUAL(serial["str"].as_string().c_str(), str.c_str());
    TEST_STRING_EQUAL(serial["obj"].as_string().c_str(), obj.as_string().c_str());

    serial["small"] = 101;
    serial["usmall"] = 102;
    serial["dec"] = 103.14;
    serial["str"] = "hello world!";
    json::jobject obj2;
    obj2["world"] = "hello";
    serial["obj"] = obj2;
    test.from_json(serial);
    TEST_EQUAL(small, 101);
    TEST_EQUAL(usmall, 102);
    TEST_TRUE(dec > 103.1f && dec < 103.2f);
    TEST_STRING_EQUAL(obj.as_string().c_str(), obj2.as_string().c_str());
}