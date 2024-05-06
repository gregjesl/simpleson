#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "json.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size == 0) {
        return 0;
    }

    // Copy input data to a null-terminated string
    char* input = new char[size + 1];
    memcpy(input, data, size);
    input[size] = '\0';

    try {
        json::jobject::parse(input);
    } catch (...) {
        // Catch all exceptions thrown by the target code
    }

    delete[] input;

    return 0;
}
