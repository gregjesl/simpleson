/*! \file json.cpp
 * \brief Simpleson source file
 */

#include "json.h"
#include <string.h>

/*! \brief Checks for an empty string
 * 
 * @param str The string to check
 * @return True if the string is empty, false if the string is not empty
 * @warning The string must be null-terminated for this macro to work
 */
#define EMPTY_STRING(str) (*str == '\0')

/*! \brief Moves a pointer to the first character that is not white space
 *
 * @param str The pointer to move
 */
#define SKIP_WHITE_SPACE(str) { const char *next = json::parsing::tlws(str); str = next; }

/*! \brief Determines if the end character of serialized JSON is encountered
 * 
 * @param obj The JSON object or array that is being written to
 * @param index The pointer to the character to be checked
 */
#define END_CHARACTER_ENCOUNTERED(obj, index) (obj.is_array() ? *index == ']' : *index == '}')

/*! \brief Calculates the remaining bytes in a buffer
 *
 * @param buffer The pointer to the start of the buffer
 * @param pointer The position of the read pointer
 * @param buffer_length The total length of the buffer
 */
#define REMAINING_BUFFER_LENGTH(buffer, pointer, buffer_length) (buffer_length - (pointer - buffer))

/*! \brief Format used for integer to string conversion */
const char * INT_FORMAT = "%i";

/*! \brief Format used for unsigned integer to string conversion */
const char * UINT_FORMAT = "%u";

/*! \brief Format used for long integer to string conversion */
const char * LONG_FORMAT = "%li";

/*! \brief Format used for unsigned long integer to string conversion */
const char * ULONG_FORMAT = "%lu";

/*! \brief Format used for character to string conversion */
const char * CHAR_FORMAT = "%c";

/*! \brief Format used for floating-point number to string conversion */
const char * FLOAT_FORMAT = "%f";

/*! \brief Format used for double floating-opint number to string conversion */
const char * DOUBLE_FORMAT = "%lf";

void json::utf8_string::clear() 
{
    this->value.clear();
    this->codepoints = 0;
}

void json::utf8_string::from_string(const char *input)
{
    json::utf8_stream stream;
    stream.read(input);
    if(!stream.is_valid()) throw json::utf8_string::invalid_utf8_string();
    *this = stream;
}

void json::utf8_string::from_string(const std::string input)
{
    this->clear();
    if(input.size() > 0) {
        json::utf8_stream stream;
        stream.read(&input[0]);
        if(!stream.is_valid()) throw json::utf8_string::invalid_utf8_string();
        *this = stream;
    } else {
        this->clear();
    }
}

void json::utf8_stream::push(const char value)
{
    const uint8_t one_byte_mask = 127;
    const uint8_t two_byte_mask = 192; // 110xxxxx
    const uint8_t three_byte_mask = 224; // 1110xxxx
    const uint8_t four_byte_mask = 240; // 11110xxx
    const uint8_t multibyte_mask = 128; // 10xxxxxx

    // Check for existing partial code point
    if(this->point.size() > 0) {

        // There should never be more than three bytes stored
        assert(this->point.size() < 4);

        // The point should have the multibyte flag
        if((value & multibyte_mask) != multibyte_mask) throw json::utf8_string::invalid_utf8_string();

        // Check the expected length
        if((point.at(0) & two_byte_mask) == two_byte_mask) {

            // Finish the point
            this->value += point.at(0);
            this->value += value;
            this->codepoints++;
            this->point.clear();
        } 
        else if((point.at(0) & three_byte_mask) == three_byte_mask) {
            
            // Add the point
            this->point.push_back(value);

            // Check for completion
            if(this->point.size() == 3) {
                this->value.push_back(this->point.at(0));
                this->value.push_back(this->point.at(1));
                this->value.push_back(this->point.at(2));
                this->codepoints++;
                this->point.clear();
            }
        }
        else
        {
            assert((this->point.at(0) & four_byte_mask) == four_byte_mask);
            // Add the point
            this->point.push_back(value);

            // Check for completion
            if(this->point.size() == 4) {
                this->value.push_back(this->point.at(0));
                this->value.push_back(this->point.at(1));
                this->value.push_back(this->point.at(2));
                this->value.push_back(this->point.at(3));
                this->codepoints++;
                this->point.clear();
            }
        }
        // No additional logic needed
        return;
    }

    // At this point, there should not be a partial value
    assert(this->point.size() == 0);

    // Check for a multi-byte value
    if((value & one_byte_mask) == value) {
        // One-byte value detected
        this->value += value;
        this->codepoints++;
        return;
    }
    
    // At this point, it should be a new multi-point value
    assert((value & two_byte_mask) == two_byte_mask || (value & three_byte_mask) == three_byte_mask || (value & four_byte_mask) == four_byte_mask);
    this->point.push_back(value);
}

size_t json::utf8_stream::read(const char *value)
{
    if(value == NULL) throw std::invalid_argument("value");

    const char *reader = value;
    while(*reader != '\0') {
        this->push(*reader);
        reader++;
    }
    return reader - value;
}

size_t json::utf8_stream::read(const char *value, const size_t max_bytes)
{
    if(value == NULL) throw std::invalid_argument("value");

    const char *reader = value;
    while(reader - value < max_bytes) {
        if(*reader == '\0') break;
        this->push(*reader);
        reader++;
    }
    return reader - value;
}

const char* json::parsing::tlws(const char *input)
{
    const char *output = input;
    while(!EMPTY_STRING(output) && std::isspace(*output)) output++;
    return !EMPTY_STRING(output) ? output : NULL;
}

const char* json::parsing::tlws(const char *input, const size_t max_bytes)
{
    const char *output = input;
    while(output - input < max_bytes && !EMPTY_STRING(output) && std::isspace(*output)) output++;
    return output - input < max_bytes ? output : NULL;
}

json::jtype::jtype json::jtype::detect(const char input)
{
    switch (input)
    {
    case '[':
        return json::jtype::jarray;
        break;
    case '"':
        return json::jtype::jstring;
        break;
    case '{':
        return json::jtype::jobject;
        break;
    case '-':
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
        return json::jtype::jnumber;
    case 't':
    case 'f':
        return json::jtype::jbool;
        break;
    case 'n':
        return json::jtype::jnull;
        break;
    default:
        return json::jtype::not_valid;
        break;
    }
}

std::string json::parsing::read_digits(const char *input)
{
    // Trim leading white space
    const char *index = json::parsing::tlws(input);

    // Initialize the result
    std::string result;

    // Loop until all digits are read
    while (
        !EMPTY_STRING(index) &&
        (
            *index == '0' ||
            *index == '1' ||
            *index == '2' ||
            *index == '3' ||
            *index == '4' ||
            *index == '5' ||
            *index == '6' ||
            *index == '7' ||
            *index == '8' ||
            *index == '9'
            )
        )
    {
        result += *index;
        index++;
    }

    // Return the result
    return result;
}

std::string json::parsing::escape_characters(const char *input)
{
    std::string result = "\"";

    while (!EMPTY_STRING(input))
    {
        switch (*input)
        {
        case '"':
        case '\\':
        case '/':
            result += "\\";
            result += *input;
            break;
        case '\b':
            result += "\\b";
            break;
        case '\f':
            result += "\\f";
            break;
        case '\n':
            result += "\\n";
            break;
        case '\r':
            result += "\\r";
            break;
        case '\t':
            result += "\\t";
            break;
        default:
            result += *input;
            break;
        }
        input++;
    }
    result += '\"';
    return result;
}

std::string json::parsing::unescape_characters(const char *input)
{
    std::string result = "";
    size_t unicode_index = 0;

    // Check for opening quotation
    if(*input != '\"') throw json::parsing_error("Expecting opening quotation");
    input++;

    // Loop until closing quotation is found
    while(*input != '"') {
        if(*input != '\\') { // Character is not escaped
            result += *input;
            input++;
        } else { // Character is escaped
            // Move to next character
            input++;

            // Check for next character
            if(EMPTY_STRING(input)) throw json::parsing_error("Expecting character");

            // Check for appropriate control character
            switch (*input)
            {
            case '"':
            case '\\':
            case '/':
                result += *input;
                input++;
                break;
            case 'b':
                result += '\b';
                input++;
                break;
            case 'f':
                result += '\f';
                input++;
                break;
            case 'n':
                result += '\n';
                input++;
                break;
            case 'r':
                result += '\r';
                input++;
                break;
            case 't':
                result += '\t';
                input++;
                break;
            case 'u':
                // Unicode character detected
                // TODO: See issue #20
                // Copy all characters
                result += "\\u";
                input++;
                for(unicode_index = 0; unicode_index < 3; unicode_index++) {
                    if(EMPTY_STRING(input)) throw json::parsing_error("Expecting character");
                    result += *input;
                }
            default:
                throw json::parsing_error("Unexpected escape character");
                break;
            }
        }
    }
    if(*input != '"') throw json::parsing_error("Expecting closing quotation");
    return result;
}

json::parsing::parse_results json::parsing::parse(const char *input)
{
    // Strip white space
    const char *index = json::parsing::tlws(input);

    // Validate input
    if (EMPTY_STRING(index)) throw json::parsing_error("Input was only whitespace");

    // Initialize the output
    json::parsing::parse_results result;

    // Detect the type
    result.type = json::jtype::detect(index);

    // Parse the values
    switch (result.type)
    {
    case json::jtype::jstring:
        // Validate the input
        if (*index != '"') throw json::parsing_error("Expected '\"' as first character");

        // Remove the opening quote
        index++;

        // Copy the string
        while (!EMPTY_STRING(index))
        {
            if (*index != '"' || (result.value.size() > 0 && result.value[result.value.size() - 1] == '\\'))
            {
                result.value.push_back(*index);
                index++;
            }
            else
            {
                break;
            }
        }
        if (EMPTY_STRING(index) || *index != '"') result.type = json::jtype::not_valid;
        else index++;
        break;
    case json::jtype::jnumber:
    {
        const char error[] = "Input did not contain a valid number";

        if (*index == '-')
        {
            result.value.push_back('-');
            index++;
        }

        if (EMPTY_STRING(index)) throw json::parsing_error(error);

        // Read the whole digits
        std::string whole_digits = json::parsing::read_digits(index);

        // Validate the read
        if (whole_digits.length() == 0) throw json::parsing_error(error);

        // Tack on the value
        result.value += whole_digits;
        index += whole_digits.length();

        // Check for decimal number
        if (*index == '.')
        {
            result.value.push_back('.');
            index++;
            std::string decimal_digits = json::parsing::read_digits(index);

            if (decimal_digits.length() == 0) throw json::parsing_error(error);

            result.value += decimal_digits;
            index += decimal_digits.size();
        }

        // Check for exponential number
        if (*index == 'e' || *index == 'E')
        {
            result.value.push_back(*index);
            index++;

            if (EMPTY_STRING(index)) throw json::parsing_error(error);

            if (*index == '+' || *index == '-')
            {
                result.value.push_back(*index);
                index++;
            }

            if (EMPTY_STRING(index)) throw json::parsing_error(error);

            std::string exponential_digits = json::parsing::read_digits(index);

            if (exponential_digits.size() == 0) throw json::parsing_error(error);

            result.value += exponential_digits;
            index += exponential_digits.size();
        }
        break;
    }
    case json::jtype::jobject:
    {
        const char error[] = "Input did not contain a valid object";

        // The first character should be an open bracket
        if (*index != '{') throw json::parsing_error(error);
        result.value += '{';
        index++;
        SKIP_WHITE_SPACE(index);

        // Loop until the closing bracket is encountered
        while (!EMPTY_STRING(index) && *index != '}')
        {
            // Read the key
            json::parsing::parse_results key = json::parsing::parse(index);

            // Validate that the key is a string
            if (key.type != json::jtype::jstring) throw json::parsing_error(error);

            // Store the key
            result.value += json::parsing::escape_characters(key.value.c_str());
            index = json::parsing::tlws(key.remainder);

            // Look for the colon
            if (*index != ':') throw json::parsing_error(error);
            result.value.push_back(':');
            index++;

            // Get the value
            json::parsing::parse_results subvalue = json::parsing::parse(index);

            // Validate the value type
            if (subvalue.type == json::jtype::not_valid) throw json::parsing_error(error);

            // Store the value
            if (subvalue.type == json::jtype::jstring) result.value += json::parsing::escape_characters(subvalue.value.c_str());
            else result.value += subvalue.value;
            index = json::parsing::tlws(subvalue.remainder);

            // Validate format
            if (*index != ',' && *index != '}') throw json::parsing_error(error);

            // Check for next line
            if (*index == ',')
            {
                result.value.push_back(',');
                index++;
            }
        }
        if (*index != '}') throw json::parsing_error(error);
        result.value += '}';
        index++;
        break;
    }
    case json::jtype::jarray:
    {
        const char error[] = "Input did not contain a valid array";
        if (*index != '[') throw json::parsing_error(error);
        result.value += '[';
        index++;
        SKIP_WHITE_SPACE(index);
        if (EMPTY_STRING(index)) throw json::parsing_error(error);
        while (!EMPTY_STRING(index) && *index != ']')
        {
            json::parsing::parse_results array_value = json::parsing::parse(index);
            if (array_value.type == json::jtype::not_valid) throw json::parsing_error(error);
            if (array_value.type == json::jtype::jstring) result.value += json::parsing::escape_characters(array_value.value.c_str());
            else result.value += array_value.value;
            index = json::parsing::tlws(array_value.remainder);
            if (*index != ',' && *index != ']') throw json::parsing_error(error);
            if (*index == ',')
            {
                result.value.push_back(',');
                index++;
            }
        }
        if (*index != ']') throw json::parsing_error(error);
        result.value.push_back(']');
        index++;
        break;
    }
    case json::jtype::jbool:
    {
        if (strncmp(index, "true", 4) == 0)
        {
            result.value += "true";
            index += 4;
        }
        else if (strncmp(index, "false", 4) == 0)
        {
            result.value += "false";
            index += 5;
        }
        else
        {
            throw json::parsing_error("Input did not contain a valid boolean");
        }
        break;
    }
    case json::jtype::jnull:
    {
        if (strncmp(index, "null", 4) == 0)
        {
            result.value += "null";
            index+= 4;
        }
        else
        {
            throw json::parsing_error("Input did not contain a valid null");
        }
        break;
    }
    default:
        throw json::parsing_error("Input did not contain valid json");
        break;
    }

    result.remainder = index;
    return result;
}

std::vector<std::string> json::parsing::parse_array(const char *input)
{
    // Initalize the result
    std::vector<std::string> result;

    const char *index = json::parsing::tlws(input);
    if (*index != '[') throw json::parsing_error("Input was not an array");
    index++;
    SKIP_WHITE_SPACE(index);
    if (*index == ']')
    {
        return result;
    }
    const char error[] = "Input was not properly formated";
    while (!EMPTY_STRING(index))
    {
        SKIP_WHITE_SPACE(index);
        json::parsing::parse_results parse_results = json::parsing::parse(index);
        if (parse_results.type == json::jtype::not_valid) throw json::parsing_error(error);
        result.push_back(parse_results.value);
        index = json::parsing::tlws(parse_results.remainder);
        if (*index == ']') break;
        if (*index == ',') index++;
    }
    if (*index != ']') throw json::parsing_error(error);
    index++;
    return result;
}

json::jobject::entry::operator int() const { return this->get_number<int>(INT_FORMAT); }
json::jobject::entry::operator unsigned int() const { return this->get_number<unsigned int>(UINT_FORMAT); }
json::jobject::entry::operator long() const { return this->get_number<long>(LONG_FORMAT); }
json::jobject::entry::operator unsigned long() const { return this->get_number<unsigned long>(ULONG_FORMAT); }
json::jobject::entry::operator char() const { return this->get_number<char>(CHAR_FORMAT); }
json::jobject::entry::operator float() const { return this->get_number<float>(FLOAT_FORMAT); }
json::jobject::entry::operator double() const { return this->get_number<double>(DOUBLE_FORMAT); }

json::jobject::entry::operator std::vector<int>() const { return this->get_number_array<int>(INT_FORMAT); }
json::jobject::entry::operator std::vector<unsigned int>() const { return this->get_number_array<unsigned int>(UINT_FORMAT); }
json::jobject::entry::operator std::vector<long>() const { return this->get_number_array<long>(LONG_FORMAT); }
json::jobject::entry::operator std::vector<unsigned long>() const { return this->get_number_array<unsigned long>(ULONG_FORMAT); }
json::jobject::entry::operator std::vector<char>() const { return this->get_number_array<char>(CHAR_FORMAT); }
json::jobject::entry::operator std::vector<float>() const { return this->get_number_array<float>(FLOAT_FORMAT); }
json::jobject::entry::operator std::vector<double>() const { return this->get_number_array<double>(DOUBLE_FORMAT); }

void json::jobject::proxy::set_array(const std::vector<std::string> &values, const bool wrap)
{
    std::string value = "[";
    for (size_t i = 0; i < values.size(); i++)
    {
        if (wrap) value += json::parsing::escape_characters(values[i].c_str()) + ",";
        else value += values[i] + ",";
    }
    if(values.size() > 0) value.erase(value.size() - 1, 1);
    value += "]";
    this->sink.set(key, value);
}

json::jobject json::jobject::parse(const char *input)
{
    const char error[] = "Input is not a valid object";
    const char *index = json::parsing::tlws(input);
    json::jobject result;
    switch (*index)
    {
    case '{':
        // Result is already an object
        break;
    case '[':
        result = json::jobject(true);
        break;
    default:
        throw json::parsing_error(error);
        break;
    }
    index++;
    SKIP_WHITE_SPACE(index);
    if (EMPTY_STRING(index)) throw json::parsing_error(error);

    while (!EMPTY_STRING(index) && !END_CHARACTER_ENCOUNTERED(result, index))
    {
        // Get key
        kvp entry;

        if(!result.is_array()) {
            json::parsing::parse_results key = json::parsing::parse(index);
            if (key.type != json::jtype::jstring || key.value == "") throw json::parsing_error(error);
            entry.first = key.value;
            index = key.remainder;

            // Get value
            SKIP_WHITE_SPACE(index);
            if (*index != ':') throw json::parsing_error(error);
            index++;
        }

        SKIP_WHITE_SPACE(index);
        json::parsing::parse_results value = json::parsing::parse(index);
        if (value.type == json::jtype::not_valid) throw json::parsing_error(error);
        if (value.type == json::jtype::jstring) entry.second = "\"" + value.value + "\"";
        else entry.second = value.value;
        index = value.remainder;

        // Clean up
        SKIP_WHITE_SPACE(index);
        if (*index != ',' && !END_CHARACTER_ENCOUNTERED(result, index)) throw json::parsing_error(error);
        if (*index == ',') index++;
        result += entry;

    }
    if (EMPTY_STRING(index) || !END_CHARACTER_ENCOUNTERED(result, index)) throw json::parsing_error(error);
    index++;
    return result;
}

void json::jobject::set(const std::string &key, const std::string &value)
{
    if(this->array_flag) throw json::invalid_key(key);
    for (size_t i = 0; i < this->size(); i++)
    {
        if (this->data.at(i).first == key)
        {
            this->data.at(i).second = value;
            return;
        }
    }
    kvp entry;
    entry.first = key;
    entry.second = value;
    this->data.push_back(entry);
}

void json::jobject::remove(const std::string &key)
{
    for (size_t i = 0; i < this->size(); i++)
    {
        if (this->data.at(i).first == key)
        {
            this->remove(i);
        }
    }
}

json::jobject::operator std::string() const
{
    if (is_array()) {
        if (this->size() == 0) return "[]";
        std::string result = "[";
        for (size_t i = 0; i < this->size(); i++)
        {
            result += this->data.at(i).second + ",";
        }
        result.erase(result.size() - 1, 1);
        result += "]";
        return result;
    } else {
        if (this->size() == 0) return "{}";
        std::string result = "{";
        for (size_t i = 0; i < this->size(); i++)
        {
            result += "\"" + this->data.at(i).first + "\":" + this->data.at(i).second + ",";
        }
        result.erase(result.size() - 1, 1);
        result += "}";
        return result;
    }
}

std::string json::jobject::pretty(unsigned int indent_level) const
{
    std::string result = "";
    for(unsigned int i = 0; i < indent_level; i++) result += "\t";
    if (is_array()) {
        if(this->size() == 0) {
            result += "[]";
            return result;
        }
        result += "[\n";
        for (size_t i = 0; i < this->size(); i++)
        {
            switch(json::jtype::detect(this->data.at(i).second.c_str())) {
                case json::jtype::jarray:
                case json::jtype::jobject:
                    result += json::jobject::parse(this->data.at(i).second).pretty(indent_level + 1);
                    break;
                default:
                    for(unsigned int j = 0; j < indent_level + 1; j++) result += "\t";
                    result += this->data.at(i).second;
                    break;
            }

            result += ",\n";
        }
        result.erase(result.size() - 2, 1);
        for(unsigned int i = 0; i < indent_level; i++) result += "\t";
        result += "]";
    } else {
        if(this->size() == 0) {
            result += "{}";
            return result;
        }
        result += "{\n";
        for (size_t i = 0; i < this->size(); i++)
        {
            for(unsigned int j = 0; j < indent_level + 1; j++) result += "\t";
            result += "\"" + this->data.at(i).first + "\": ";
            switch(json::jtype::detect(this->data.at(i).second.c_str())) {
                case json::jtype::jarray:
                case json::jtype::jobject:
                    result += std::string(json::parsing::tlws(json::jobject::parse(this->data.at(i).second).pretty(indent_level + 1).c_str()));
                    break;
                default:
                    result += this->data.at(i).second;
                    break;
            }

            result += ",\n";
        }
        result.erase(result.size() - 2, 1);
        for(unsigned int i = 0; i < indent_level; i++) result += "\t";
        result += "}";
    }
    return result;
}

void json::jstream::push_case_insensitive(const char value, const char upper, const char lower)
{
    if((value == upper) || (value == lower)) {
        this->key.push(value);
    } else {
        throw json::parsing_error(this->error_str);
    }
}

json::jstream::state_t json::jstream::push(const char input)
{
    switch (this->state)
    {
    case UNINITIALIZED:
        if(std::isspace(input)) return UNINITIALIZED;
        switch (input)
        {
        case '{':
            // Object detected
            this->container = json::jobject(false);
            this->state = OPENED;
            break;
        case '[':
            // Array detected
            this->container = json::jobject(true);
            this->state = OPENED;
            break;
        default:
            throw json::parsing_error("Expecting '{' or '['");
            break;
        }
        break;
    case OPENED:
        if(std::isspace(input)) return OPENED;
        switch (input)
        {
        case '}':
            if(this->container.is_array()) throw json::parsing_error(error_str);
            this->key.clear();
            this->value.clear();
            this->value_type = json::jtype::not_valid;
            this->state = CLOSED;
            break;
        case ']':
            if(!this->container.is_array()) throw json::parsing_error(error_str);
            this->key.clear();
            this->value.clear();
            this->value_type = json::jtype::not_valid;
            this->state = CLOSED;
            break;
        default:
            this->state = this->container.is_array() ? AWAITING_VALUE : AWAITING_KEY;
            return this->push(input);
        }
        break;
    case AWAITING_KEY:
        // This case should not be used for arrays
        assert(!this->container.is_array());
        if(std::isspace(input)) return AWAITING_KEY;
        if(input != '"') throw json::parsing_error("Expecting '\"'");
        key.push(input);
        this->state = STREAMING_KEY;
        break;
    case STREAMING_KEY:
        // This case should not be used for arrays
        assert(!this->container.is_array());
        // Check for end of key
        if(input == '"') {
            // This should not be the opening quotation
            assert(this->key.length() > 0);

            // Verify the UTF8 string
            if(!this->key.is_valid()) throw json::utf8_string::invalid_utf8_string();

            // #todo Handle quotations in keys
            this->key.push(input);
            this->state = AWAITING_SEPERATOR;
        } else {
            this->key.push(input);
        }
        break;
    case AWAITING_SEPERATOR:
        // This case should not be used for arrays
        assert(!this->container.is_array());
        // Check for a seperator
        if(std::isspace(input)) return AWAITING_SEPERATOR;
        if(input != ':') throw json::parsing_error("Expecting ':'");
        this->state = AWAITING_VALUE;
        break;
    case AWAITING_VALUE:
        assert(this->value.size() == 0);
        assert(this->value_type == json::jtype::not_valid);
        if(std::isspace(input)) return AWAITING_VALUE;
        this->value_type = json::jtype::detect(input);
        switch (this->value_type)
        {
        case jtype::jstring:
        case jtype::jnumber:
        case jtype::jnull:
        case jtype::jbool:
            this->value.push(input);
            break;
        case jtype::jobject:
        case jtype::jarray:
            this->sub_object = new jstream();
            this->sub_object->push(input);
            break;
        case jtype::not_valid:
            throw json::parsing_error(error_str);
            break;
        }
        this->state = STREAMING_VALUE;
        break;
    case STREAMING_VALUE:
        switch (this->value_type)
        {
        case jtype::jstring:
            // #todo
            #error
            break;
        case jtype::jnumber:
            // #todo
            #error
            break;
        case jtype::jbool:
            switch (this->value[0])
            {
            case 't':
            case 'T':
                switch (this->value.length())
                {
                case 1:
                    this->push_case_insensitive(input, 'R', 'r');
                    break;
                case 2:
                    this->push_case_insensitive(input, 'U', 'u');
                    break;
                case 3:
                    this->push_case_insensitive(input, 'E', 'e');
                    this->interate();
                    this->state = ENTRY_COMPLETE;
                    break;
                default:
                    // This should not occur
                    assert(false);
                    break;
                }
                break;
            case 'f':
            case 'F':
                switch (this->value.length())
                {
                case 1:
                    this->push_case_insensitive(input, 'A', 'a');
                    break;
                case 2:
                    this->push_case_insensitive(input, 'L', 'l');
                    break;
                case 3:
                    this->push_case_insensitive(input, 'S', 's');
                    break;
                case 4:
                    this->push_case_insensitive(input, 'E', 'e');
                    this->interate();
                    this->state = ENTRY_COMPLETE;
                    break;
                default:
                    // This should not occur
                    assert(false);
                    break;
                }
                break;
            default:
                // This should not occur
                assert(false);
                break;
            }
            this->value.push(input);
            break;
        case jtype::jnull:
            // The first character should be an 'n' or 'N'
            assert((this->value[0] == 'n') || (this->value[0] == 'N'));

            switch (this->key.length())
            {
            case 1:
                if(input == 'u' || input == 'U') {
                    this->value.push(input);
                } else {
                    throw json::parsing_error(error_str);
                }
                break;
            case 2:
                if(input == 'l' || input == 'L') {
                    this->value.push(input);
                } else {
                    throw json::parsing_error(error_str);
                }
                break;
            case 3:
                if(input == 'l' || input == 'L') {
                    this->value.push(input);
                } else {
                    throw json::parsing_error(error_str);
                }
                this->interate();
                this->state = ENTRY_COMPLETE;
                break;
            default:
                // This should never happen
                assert(false);
                break;
            }
            break;
        case jtype::jobject:
        case jtype::jarray:
            this->sub_object->push(input);
            if(this->sub_object->state == CLOSED) {
                this->interate();
                delete this->sub_object;
                this->sub_object = NULL;
                this->state = ENTRY_COMPLETE;
            }
            break;
        case jtype::not_valid:
            throw json::parsing_error(error_str);
            break;
        }
        break;
    case ENTRY_COMPLETE:
        if(std::isspace(input)) return ENTRY_COMPLETE;
        switch (input)
        {
        case ',':
            if(container.size() == 0) throw json::parsing_error("Unexpected character encountered");
            this->key.clear();
            this->value.clear();
            this->value_type = json::jtype::not_valid;
            break;
        case '}':
        case ']':
            this->state = OPENED;
            return this->push(input);
        default:
            throw json::parsing_error("Unexpected character encountered");
            break;
        }
        break;
    case CLOSED:
        // Do nothing
        break;
    }
    return this->state;
}