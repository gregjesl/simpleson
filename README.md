[![Build status](https://ci.appveyor.com/api/projects/status/h9avws048watkvnr?svg=true)](https://ci.appveyor.com/project/gregjesl/simpleson)

# simpleson
Lightweight C++ JSON parser &amp; serializer that is C++98 compatible with no dependencies

## Why simpleson? 
Simpleson is built under the following requirements:
- One header and one source file only
- No external dependencies
- ISO/IEC 14882:1998 (aka C++98) compatible
- Cross-platform

A primary use case for simpleson is in an memory-constrained embedded system.  

## Building simpleson
Simpleson was intentionally built such that a developer could simply copy [json.h](json.h) into the target project's `inc` folder, copy [json.cpp](json.cpp) into the `src` folder, and then compile the target project.  No linking -> no drama.  

Building the library and tests follows the standard build chain via CMake:
```
mkdir build
cd build
cmake ../
make
```
Unit tests can then be run by executing `make test`

## Quickstart

```cpp
// Create the input
std::string input = "{ \"hello\": \"world\" }";

// Parse the input
json::jobject result = json::jobject::parse(input);

// Get a value
std::string value = (std::string)result.get_entry("hello").value

// Add entry
json::key_value_pair item;
item.key = "new_key";
item.value = json::jvalue(123.4);
result.push_back(item);

// Serialize the new object
std::string serial = (std::string)result;
```

## Using simpleson

The namespace of simpleson is simply `json`.  JSON objects can be parsed by calling `json::jobject::parse()`, which returns a `jobject`.  The `jobject` class is simply an extension of a vector of [key-value pairs](#key-value-pairs).  Useful methods of `jobject` include:
- `has_key("key")`
- `get_keys()`
- `get_entry("key")`

An instance of `jobject` can be searlized by casting it to a `std::string`.  Note that an instance of `jobject` does not retain it's original formatting (it drops tabs, spaces outside strings, and newlines).  

You can build your own `jobject` from scratch by creating a fresh instance of `jobject` and then pushing (`push_back`) key-value pairs into it.  

### Key-value pairs
A key-value pair is just that: a key and a value; the members `key` and `value` of `json::key_value_pair` are public.  The `key` member is simply the string of the key whie the `value` member is a `jvalue` instance.  A `jvalue` instance can be casted to `int`, `long`, `double`, `float`, `std::string`, and `json::jarray` depending on the type of value it holds.  

You can check the type of value by calling `get_type()` on the value.  

You can deserialze a nested object by calling `json::jobject::parse(parent.get_entry("child").value)`.  

### Arrays
Simpleson includes a class called `jarray`, which is an extension of a vector of strings.  You can create an instance of `jarray` by parsing a string via `json::jarray::parse(input)`.  An instance of `jarray` can also be casted to `std::vector<int>`, `std::vector<long>`, `std::vector<double>`, and `std::vector<float>`.  
