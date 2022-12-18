#include "json.h"
#include <stdio.h>
#include <assert.h>

int main(void)
{
	// Create a couple objects
    const std::string test = 
    "["
    "   {"
    "       \"firstName\": \"Jimmy\","
    "       \"lastName\": \"D\","
    "       \"hobbies\": ["
    "           {"
    "               \"sport\": \"tennis\""
    "           },"
    "           {"
    "               \"music\": \"rock\""
    "           }"
    "       ]"
    "   },"
    "   {"
    "       \"firstName\": \"Sussi\","
    "       \"lastName\": \"Q\","
    "       \"hobbies\": ["
    "           {"
    "               \"sport\": \"volleyball\""
    "           },"
    "           {"
    "               \"music\": \"classical\""
    "           }"
    "       ]"
    "   }"
    "]";

    // Parse the test array
    json::jarray example = json::jarray::parse(test);

    // Access the data
    std::string music_desired = example.at(0).as_object()["hobbies"].as_array().at(1).as_object()["music"].as_string();

    // Print the data
    printf("Music desired: %s\n", music_desired.c_str()); // Returns "rock"

    // Check the result
    assert(music_desired == std::string("rock"));

    // Access the second entry
    music_desired = example.at(1).as_object()["hobbies"].as_array().at(1).as_object()["music"].as_string();

    // Print the data
    printf("Music desired: %s\n", music_desired.c_str()); // Returns "classical"

    // Check the result
    assert(music_desired == std::string("classical"));
}