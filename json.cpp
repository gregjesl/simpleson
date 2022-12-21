/*! \file json.cpp
 * \brief Simpleson source file
 */

#include "json.h"
#include <string.h>
#include <assert.h>

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

/*! \brief Determines if the supplied character is a digit
 *
 * @param input The character to be tested
 */
#define IS_DIGIT(input) (input >= '0' && input <= '9')

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

const char* json::parsing::tlws(const char *input)
{
    const char *output = input;
    while(!EMPTY_STRING(output) && std::isspace(*output)) output++;
    return output;
}

json::jtype::jtype json::jtype::peek(const char input)
{
    switch (input)
    {
    case '[':
        return json::jtype::jarray;
    case '"':
        return json::jtype::jstring;
    case '{':
        return json::jtype::jobject;
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
    case 'n':
        return json::jtype::jnull;
    default:
        return json::jtype::not_valid;
    }
}

json::jtype::jtype json::jtype::detect(const char *input)
{
    const char *start = json::parsing::tlws(input);
    return json::jtype::peek(*start);
}

void json::reader::clear()
{
    std::string::clear(); 
    if(this->sub_reader != NULL) {
        delete this->sub_reader;
        this->sub_reader = NULL;
    }
    this->read_state = 0;
}

json::reader::push_result json::reader::push(const char next)
{
    // Check for opening whitespace
    if(this->length() == 0 && std::isspace(next)) return reader::ACCEPTED;

    // Get the type
    const json::jtype::jtype type = json::jtype::peek(this->length() > 0 ? this->front() : next);

    // Store the return
    reader::push_result result = reader::REJECTED;

    #if DEBUG
    const size_t start_length = this->length();
    #endif

    switch(type)
    {
    case json::jtype::jarray:
        result = this->push_array(next);
        break;
    case json::jtype::jbool:
        result = this->push_boolean(next);
        assert(result != WHITESPACE);
        break;
    case json::jtype::jnull:
        result = this->push_null(next);
        assert(result != WHITESPACE);
        break;
    case json::jtype::jnumber:
        result = this->push_number(next);
        assert(result != WHITESPACE);
        break;
    case json::jtype::jobject:
        result = this->push_object(next);
        break;
    case json::jtype::jstring:
        result = this->push_string(next);
        assert(result != WHITESPACE);
        break;
    case json::jtype::not_valid:
        result = reader::REJECTED;
        break;
    }

    // Verify the expected length change
    #if DEBUG
    if(result == ACCEPTED) assert(this->length() - start_length == 1);
    else assert(this->length() == start_length);
    #endif

    // Return the result
    return result;
}

bool json::reader::is_valid() const
{
    switch (this->type())
    {
    case jtype::jarray:
        return this->get_state<array_reader_enum>() == ARRAY_CLOSED;
    case jtype::jbool:
        if(this->length() < 4) return false;
        if(this->length() == 4 && *this == "true") return true;
        if(this->length() == 5 && *this == "false") return true;
        return false;
    case jtype::jnull:
        return (this->length() == 4 && *this == "null");
    case jtype::jnumber:
        switch (this->get_state<number_reader_enum>())
        {
        case NUMBER_ZERO:
        case NUMBER_INTEGER_DIGITS:
        case NUMBER_FRACTION_DIGITS:
        case NUMBER_EXPONENT_DIGITS:
            return true;
        default:
            return false;
        }
    case jtype::jobject:
        return this->get_state<object_reader_enum>() == OBJECT_CLOSED;
    case jtype::jstring:
        return this->get_state<string_reader_enum>() == STRING_CLOSED;
    case jtype::not_valid:
        return false;
    }
    throw std::logic_error("Unexpected return");
}

bool is_control_character(const char input)
{
    switch (input)
    {
    case 'b':
    case 'f':
    case 'n':
    case 'r':
    case 't':
    case '"':
    case '\\':
        return true;
    default:
        return false;
    }
}

bool is_hex_digit(const char input)
{
    return IS_DIGIT(input) || (input >= 'a' && input <= 'f') || (input >= 'A' && input <= 'F');
}

json::reader::push_result json::reader::push_string(const char next)
{
    const string_reader_enum state = this->get_state<string_reader_enum>();
    switch (state)
    {
    case STRING_EMPTY:
        assert(this->length() == 0);
        if(next == '"') {
            assert(this->length() == 0);
            this->push_back(next);
            this->set_state(STRING_OPENING_QUOTE);
            return ACCEPTED;
        }
        return REJECTED;
    case STRING_OPENING_QUOTE:
        assert(this->length() == 1);
        this->set_state(STRING_OPEN);
        // Fall through deliberate
    case STRING_OPEN:
        assert(this->length() > 0);
        switch (next)
        {
        case '\\':
            this->set_state(STRING_ESCAPED);
            break;
        case '"':
            this->set_state(STRING_CLOSED);
            break;
        default:
            // No state change
            break;
        }
        this->push_back(next);
        return ACCEPTED;
    case STRING_ESCAPED:
        if(is_control_character(next)) {
            this->set_state(STRING_OPEN);
            this->push_back(next);
            return ACCEPTED;
        } else if(next == 'u') {
            this->set_state(STRING_CODE_POINT_START);
            this->push_back(next);
            return ACCEPTED;
        }
        return REJECTED;
    case STRING_CODE_POINT_START:
        assert(this->back() == 'u');
        if(!is_hex_digit(next)) return REJECTED;
        this->push_back(next);
        this->set_state(STRING_CODE_POINT_1);
        return ACCEPTED;
    case STRING_CODE_POINT_1:
        assert(is_hex_digit(this->back()));
        if(!is_hex_digit(next)) return REJECTED;
        this->push_back(next);
        this->set_state(STRING_CODE_POINT_2);
        return ACCEPTED;
    case STRING_CODE_POINT_2:
        assert(is_hex_digit(this->back()));
        if(!is_hex_digit(next)) return REJECTED;
        this->push_back(next);
        this->set_state(STRING_CODE_POINT_3);
        return ACCEPTED;
    case STRING_CODE_POINT_3:
        assert(is_hex_digit(this->back()));
        if(!is_hex_digit(next)) return REJECTED;
        this->push_back(next);
        this->set_state(STRING_OPEN);
        return ACCEPTED;
    case STRING_CLOSED:
        return REJECTED;
    }
    throw std::logic_error("Unexpected return");
}

json::reader::push_result json::reader::push_array(const char next)
{
    const array_reader_enum state = this->get_state<array_reader_enum>();

    switch (state)
    {
    case ARRAY_EMPTY:
        assert(this->sub_reader == NULL);
        if(next == '[') {
            this->set_state(ARRAY_OPEN_BRACKET);
            this->push_back(next);
            return ACCEPTED;
        }
        return REJECTED;
    case ARRAY_OPEN_BRACKET:
        assert(this->sub_reader == NULL);
        if(std::isspace(next)) return WHITESPACE;
        if(next == ']') {
            this->set_state(ARRAY_CLOSED);
            this->push_back(next);
            return ACCEPTED;
        }
        begin_reading_value:
        if(json::jtype::peek(next) == json::jtype::not_valid) return REJECTED;
        this->sub_reader = new reader();
        this->set_state(ARRAY_READING_VALUE);
        // Fall-through deliberate
    case ARRAY_READING_VALUE:
        assert(this->sub_reader != NULL);
        if(this->sub_reader->is_valid() && std::isspace(next)) return WHITESPACE;
        switch (this->sub_reader->push(next))
        {
        case ACCEPTED:
            return ACCEPTED;
        case WHITESPACE:
            return WHITESPACE;
        case REJECTED:
            switch (next)
            {
            case ']':
                if(!this->sub_reader->is_valid()) return REJECTED;
                this->append(this->sub_reader->serialize());
                delete this->sub_reader;
                this->sub_reader = NULL;
                this->push_back(next);
                this->set_state(ARRAY_CLOSED);
                return ACCEPTED;
            case ',':
                if(!this->sub_reader->is_valid()) return REJECTED;
                this->append(this->sub_reader->serialize());
                delete this->sub_reader;
                this->sub_reader = NULL;
                this->push_back(next);
                this->set_state(ARRAY_AWAITING_NEXT_LINE);
                return ACCEPTED;
            default:
                return REJECTED;
            }
        }
        // This point should not be reached
        break;
    case ARRAY_AWAITING_NEXT_LINE:
        if(std::isspace(next)) return WHITESPACE;
        goto begin_reading_value;
    case ARRAY_CLOSED:
        return REJECTED;
    }
    throw std::logic_error("Unexpected return");
}

json::reader::push_result json::reader::push_object(const char next)
{
    const object_reader_enum state = this->get_state<object_reader_enum>();

    switch (state)
    {
    case OBJECT_EMPTY:
        assert(this->sub_reader == NULL);
        if(next == '{') {
            this->set_state(OBJECT_OPEN_BRACE);
            this->push_back(next);
            return ACCEPTED;
        }
        return REJECTED;
    case OBJECT_OPEN_BRACE:
        assert(this->sub_reader == NULL);
        if(next == '}') {
            this->set_state(OBJECT_CLOSED);
            this->push_back(next);
            return ACCEPTED;
        }
        // Fall-through deliberate
    case OBJECT_AWAITING_NEXT_LINE:
        if(std::isspace(next)) return WHITESPACE;
        if(next != '"') return REJECTED;
        this->sub_reader = new kvp_reader();
        #if DEBUG
        assert(
        #endif
        this->sub_reader->push(next)
        #if DEBUG
        == ACCEPTED);
        #else
        ;
        #endif
        this->set_state(OBJECT_READING_ENTRY);
        return ACCEPTED;
    case OBJECT_READING_ENTRY:
        assert(this->sub_reader != NULL);
        switch (this->sub_reader->push(next))
        {
        case ACCEPTED:
            return ACCEPTED;
        case WHITESPACE:
            return WHITESPACE;
        case REJECTED:
            if(!this->sub_reader->is_valid()) return REJECTED;
            if(std::isspace(next)) return WHITESPACE;
            switch (next)
            {
            case '}':
                this->append(this->sub_reader->serialize());
                delete this->sub_reader;
                this->sub_reader = NULL;
                this->push_back(next);
                this->set_state(OBJECT_CLOSED);
                return ACCEPTED;
            case ',':
                this->append(this->sub_reader->serialize());
                delete this->sub_reader;
                this->sub_reader = NULL;
                this->push_back(next);
                this->set_state(OBJECT_AWAITING_NEXT_LINE);
                return ACCEPTED;
            default:
                return REJECTED;
            }
        }
        // This point should never be reached
        break;
    case OBJECT_CLOSED:
        return REJECTED;
    }
    throw std::logic_error("Unexpected return");
}

json::reader::push_result json::reader::push_number(const char next)
{
    const number_reader_enum state = this->get_state<number_reader_enum>();
    switch (state)
    {
    case NUMBER_EMPTY:
        assert(this->length() == 0);
        if(next == '-') {
            this->set_state(NUMBER_OPEN_NEGATIVE);
            this->push_back(next);
            return ACCEPTED;
        } else if(IS_DIGIT(next)) {
            this->set_state(next == '0' ? NUMBER_ZERO : NUMBER_INTEGER_DIGITS);
            this->push_back(next);
            return ACCEPTED;
        }
        return REJECTED;
    case NUMBER_OPEN_NEGATIVE:
        if(IS_DIGIT(next)) {
            this->set_state(next == '0' ? NUMBER_ZERO : NUMBER_INTEGER_DIGITS);
            this->push_back(next);
            return ACCEPTED;
        }
        return REJECTED;
    case NUMBER_INTEGER_DIGITS:
        assert(IS_DIGIT(this->back()));
        if(IS_DIGIT(next)) {
            this->push_back(next);
            return ACCEPTED;
        }
        // Fall-through deliberate
    case NUMBER_ZERO:
        switch (next)
        {
        case '.':
            this->set_state(NUMBER_DECIMAL);
            this->push_back(next);
            return ACCEPTED;
        case 'e':
        case 'E':
            this->set_state(NUMBER_EXPONENT);
            this->push_back(next);
            return ACCEPTED;
        default:
            return REJECTED;
        }
    case NUMBER_DECIMAL:
        assert(this->back() == '.');
        if(IS_DIGIT(next)) {
            this->set_state(NUMBER_FRACTION_DIGITS);
            this->push_back(next);
            return ACCEPTED;
        }
        return REJECTED;
    case NUMBER_FRACTION_DIGITS:
        assert(IS_DIGIT(this->back()));
        if(IS_DIGIT(next)) {
            this->push_back(next);
            return ACCEPTED;
        } else if(next == 'e' || next == 'E') {
            this->set_state(NUMBER_EXPONENT);
            this->push_back(next);
            return ACCEPTED;
        }
        return REJECTED;
    case NUMBER_EXPONENT:
        assert(this->back() == 'e' || this->back() == 'E');
        if(next == '+' || next == '-') {
            this->set_state(NUMBER_EXPONENT_SIGN);
            this->push_back(next);
            return ACCEPTED;
        }
        // Fall-through deliberate
    case NUMBER_EXPONENT_SIGN:
    case NUMBER_EXPONENT_DIGITS:
        if(IS_DIGIT(next)) {
            this->set_state(NUMBER_EXPONENT_DIGITS);
            this->push_back(next);
            return ACCEPTED;
        }
        return REJECTED;
    }
    throw std::logic_error("Unexpected return");
}

json::reader::push_result json::reader::push_boolean(const char next)
{
    const char *str_true = "true";
    const char *str_false = "false";
    const char *str = NULL;

    if(this->length() == 0) {
        switch (next)
        {
        case 't':
        case 'f':
            this->push_back(next);
            return ACCEPTED;
        default:
            return REJECTED;
        }
    }

    // Determine which string to use
    switch (this->at(0))
    {
    case 't':
        str = str_true;
        break;
    case 'f':
        str = str_false;
        break;
    default:
        throw json::parsing_error("Unexpected state");
    }
    assert(str == str_true || str == str_false);

    // Push the value
    if(this->length() < strlen(str) && str[this->length()] == next) {
        this->push_back(next);
        return ACCEPTED;
    }
    return REJECTED;
}

json::reader::push_result json::reader::push_null(const char next)
{    
    switch (this->length())
    {
    case 0:
        if(next == 'n') {
            this->push_back(next);
            return ACCEPTED;
        }
        return REJECTED;
    case 1:
        if(next == 'u') {
            this->push_back(next);
            return ACCEPTED;
        }
        return REJECTED;
    case 2:
    case 3:
        if(next == 'l') {
            this->push_back(next);
            return ACCEPTED;
        }
        // Fall through
    case 4:
        return REJECTED;
    default:
        throw json::parsing_error("Unexpected state");
    }
}

json::reader::push_result json::kvp_reader::push(const char next)
{
    if(this->_key.length() == 0) {
        if(std::isspace(next)) return WHITESPACE;
        if(next == '"') {
            this->_key.push(next);
            assert(this->_key.type() == json::jtype::jstring);
            assert(this->_key.length() == 1);
            return ACCEPTED;
        }
        return REJECTED;
    } else if (!this->_key.is_valid()) {
        return this->_key.push(next);
    }

    // At this point the key should be valid
    assert(this->_key.is_valid());

    if(!this->_colon_read) {
        if(std::isspace(next)) return WHITESPACE;
        if(next == ':') {
            this->_colon_read = true;
            return ACCEPTED;
        }
        return REJECTED;
    }

    // At this point the colon should be read
    assert(this->_colon_read);

    // Check for a fresh start
    if(reader::length() == 0 && std::isspace(next))
    {
        assert(reader::get_state<char>() == 0);
        return WHITESPACE;
    }
    return reader::push(next);
}

std::string json::kvp_reader::serialize() const
{
    return this->_key.serialize() + ":" + reader::serialize();
}

std::string json::parsing::decode_string(const char *input)
{
    const char *index = input;
    std::string result;

    if(*index != '"') throw json::parsing_error("Expecting opening quote");
    index++;
    bool escaped = false;
    // Loop until the end quote is found
    while(!(!escaped && *index == '"'))
    {
        if(escaped)
        {
            switch (*index)
            {
            case '"':
            case '\\':
            case '/':
                result += *index;
                break;
            case 'b':
                result += '\b';
                break;
            case 'f':
                result += '\f';
                break;
            case 'n':
                result += '\n';
                break;
            case 'r':
                result += '\r';
                break;
            case 't':
                result += '\t';
                break;
            case 'u':
                // #todo Unicode support
                index += 4;
                break;
            default:
                throw json::parsing_error("Expected control character");
            }
            escaped = false;
        } else if(*index == '\\') {
            escaped = true;
        } else {
            result += *index;
        }
        index++;
    }
    return result;
}

std::string json::parsing::encode_string(const char *input)
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

json::parsing::parse_results json::parsing::parse(const char *input)
{
    // Strip white space
    const char *index = json::parsing::tlws(input);

    // Validate input
    if (EMPTY_STRING(index)) throw json::parsing_error("Input was only whitespace");

    // Initialize the output
    json::parsing::parse_results result;
    result.type = json::jtype::not_valid;

    // Initialize the reader
    json::reader stream;

    // Iterate
    while(!EMPTY_STRING(input) && stream.push(*index) != json::reader::REJECTED)
    {
        index++;
    }

    if(stream.is_valid()) {
        result.value = stream.serialize();
        result.type = stream.type();
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
        if(parse_results.type == json::jtype::jstring) {
            result.push_back(json::parsing::decode_string(parse_results.value.c_str()));
        } else {
            result.push_back(parse_results.value);
        }
        index = json::parsing::tlws(parse_results.remainder);
        if (*index == ']') break;
        if (*index == ',') index++;
    }
    if (*index != ']') throw json::parsing_error(error);
    index++;
    return result;
}

json::jarray json::jarray::parse(const char *input)
{
    // Check for valid input
    if(input == NULL) throw std::invalid_argument(__FUNCTION__);
    const char *index = json::parsing::tlws(input);
    if(EMPTY_STRING(index) || *index != '[') throw std::invalid_argument(__FUNCTION__);

    // Initalize the result
    json::jarray result;
    index++;
    SKIP_WHITE_SPACE(index);

    // Check for empty array
    if (*index == ']') return result;

    // Initialize the reader
    json::reader entry;

    // Iterate over values
    next_value:

    // Verify an empty entry
    assert(entry.length() == 0);

    // Read the data
    while(entry.push(*index) != reader::REJECTED) index++;
    
    // Verify a valid read
    if(!entry.is_valid()) throw std::invalid_argument(__FUNCTION__);

    // Store the value
    result.push_back(entry);

    // Clear the reader
    entry.clear();

    // Trim any whitespace
    SKIP_WHITE_SPACE(index);

    // Check for remaining value
    if(EMPTY_STRING(index)) throw std::invalid_argument(__FUNCTION__);

    // Check for next entry
    if(*index == ',') 
    {
        index++;
        SKIP_WHITE_SPACE(index);
        goto next_value;
    }

    // At this point the array should be closed
    if(*index != ']') throw std::invalid_argument(__FUNCTION__);

    index++;
    return result;
}

std::string json::jarray::as_string() const
{
    if(this->size() == 0) return "[]";
    std::string result = "[";
    for(size_t i = 0; i < this->size() - 1; i++)
    {
        result.append(this->at(i).serialize());
        result.push_back(',');
    }
    result.append(this->back().serialize());
    result.push_back(']');
    return result;
}

json::jobject json::jobject::parse(const char *input)
{
    const char error[] = "Input is not a valid object";
    const char *index = json::parsing::tlws(input);
    if(*index != '{') throw std::invalid_argument(__FUNCTION__);
    index++;
    json::jobject result;
    json::reader stream;
    SKIP_WHITE_SPACE(index);
    if (EMPTY_STRING(index)) throw json::parsing_error(error);

    while (!EMPTY_STRING(index) && *index != '}')
    {
        // Get key
        kvp entry;

        json::parsing::parse_results key = json::parsing::parse(index);
        if (key.type != json::jtype::jstring || key.value == "") throw json::parsing_error(error);
        entry.first = json::parsing::decode_string(key.value.c_str());
        index = key.remainder;

        // Get value
        SKIP_WHITE_SPACE(index);
        if (*index != ':') throw json::parsing_error(error);
        index++;

        SKIP_WHITE_SPACE(index);
        json::parsing::parse_results value = json::parsing::parse(index);
        if (value.type == json::jtype::not_valid) throw json::parsing_error(error);
        entry.second = value.value;
        index = value.remainder;

        // Clean up
        SKIP_WHITE_SPACE(index);
        if (*index != ',' && *index != '}') throw json::parsing_error(error);
        if (*index == ',') index++;
        if(result.has_key(entry.first)) throw json::parsing_error("Key collision");
        result.data.insert(entry);

    }
    if (EMPTY_STRING(index) || *index != '}') throw json::parsing_error(error);
    index++;
    return result;
}

json::jobject::~jobject()
{
    while(!this->__proxies.empty()) {
        this->__proxies.front()->detatch();
        this->__proxies.pop_front();
    }
}

void json::jobject::attach(json::data::link *prox)
{
    this->detatch(prox);
    this->__proxies.push_back(prox);
}

void json::jobject::detatch(json::data::link *prox)
{
    this->__proxies.remove(prox);
}

json::key_list_t json::jobject::list_keys() const
{
    // Initialize the result
    key_list_t result;

    for (json::jmap::const_iterator it = this->data.begin(); it != this->data.end(); ++it)
    {
        result.push_back(it->first);
    }
    return result;
}

void json::jobject::set(const std::string &key, const std::string &value)
{
    json::jmap::iterator it = this->data.find(key);
    if(it != this->data.end()) {
        it->second = value;
    } else {
        this->data.insert(kvp(key, value));
    }
}

void json::jobject::remove(const std::string &key)
{
    this->data.erase(key);
}

json::jobject::operator std::string() const
{
    if (this->size() == 0) return "{}";
    std::string result = "{";
    for (json::jmap::const_iterator it = this->data.begin(); it != this->data.end(); ++it)
    {
        result += json::parsing::encode_string(it->first.c_str()) + ":" + it->second + ",";
    }
    result.erase(result.size() - 1, 1);
    result += "}";
    return result;
}

std::string json::jobject::pretty(unsigned int indent_level) const
{
    std::string result = "";
    for(unsigned int i = 0; i < indent_level; i++) result += "\t";
    if(this->size() == 0) {
        result += "{}";
        return result;
    }
    result += "{\n";
    for (json::jmap::const_iterator it = this->data.begin(); it != this->data.end(); ++it)
    {
        for(unsigned int j = 0; j < indent_level + 1; j++) result += "\t";
        result += "\"" + it->first + "\": ";
        switch(json::jtype::peek(*it->second.c_str())) {
            case json::jtype::jarray:
                result += std::string(json::parsing::tlws(json::jarray::parse(it->second).pretty(indent_level + 1).c_str()));
                break;
            case json::jtype::jobject:
                result += std::string(json::parsing::tlws(json::jobject::parse(it->second).pretty(indent_level + 1).c_str()));
                break;
            default:
                result += it->second;
                break;
        }

        result += ",\n";
    }
    result.erase(result.size() - 2, 1);
    for(unsigned int i = 0; i < indent_level; i++) result += "\t";
    result += "}";
    return result;
}

json::data::dynamic_data::dynamic_data(const json::reader &input)
{
    this->operator=(input);
}

void json::data::dynamic_data::operator=(const json::reader &input)
{
    if(!input.is_valid()) throw std::invalid_argument(__FUNCTION__);
    this->__value = input.serialize();
}

json::jtype::jtype json::data::dynamic_data::type() const
{
    return this->__value.length() > 0 ? 
        json::jtype::peek(this->__value.at(0)) : 
        json::jtype::not_valid;
}

void json::data::dynamic_data::set_null()
{
    this->__value = "null";
}

void json::data::dynamic_data::set_true()
{
    this->__value = "true";
}

void json::data::dynamic_data::set_false()
{
    this->__value = "false";
}

template<typename T>
T cast_uint(const std::string &input, const T max_value)
{
    const char *overflow = __FUNCTION__; 
    assert(input.size() > 0);
    if(input == "0") return 0;
    T result = input.at(0) - (T)'0';
    const T ninety_percent = max_value / 10;
    for(size_t i = 1; i < input.size(); i++)
    {
        if(result > ninety_percent) throw std::overflow_error(overflow);
        result *= 10;
        assert(IS_DIGIT(input.at(i)));
        const T digit = input.at(i) - (T)'0';
        if(result > max_value - digit) throw std::overflow_error(overflow);
        result += digit;
    }
    return result;
}

template<typename T>
T cast_int(const std::string &input, const T min_value, const T max_value)
{
    assert(input.size() > 0);
    T result = 0;
    if(input.at(0) == '-') {
        result = cast_uint(input.substr(1, input.size() - 1), min_value);
        result *= -1;
    } else {
        result = cast_uint(input, max_value);
    }
    return result;
}

template<typename T>
std::string from_uint(const T input)
{
    T remainder = input;
    if(input == 0) return "0";
    std::string result;
    while(remainder != 0)
    {
        const char digit = (remainder % 10) + '0';
        result.insert(result.begin(), digit);
        remainder /= 10;
    }
    return result;
}

template<typename T>
std::string from_int(const T input)
{
    std::string result = from_uint<T>(input);
    if(input < 0) {
        result.insert(result.begin(), '-');
    }
    return result;
}

#define DYNAMIC_UINT_SOURCE(format, max)                    \
void json::data::dynamic_data::set(const format value)      \
{                                                           \
    this->__value = from_uint(value);                       \
}                                                           \
                                                            \
json::data::dynamic_data::operator format() const           \
{                                                           \
    return cast_uint<format>(this->__value, max);           \
}

#define DYNAMIC_INT_SOURCE(format, min, max)                \
void json::data::dynamic_data::set(const format value)      \
{                                                           \
    this->__value = from_int(value);                        \
}                                                           \
                                                            \
json::data::dynamic_data::operator format() const           \
{                                                           \
    return cast_int<format>(this->__value, min, max);       \
}

DYNAMIC_UINT_SOURCE(uint8_t, UINT8_MAX);
DYNAMIC_INT_SOURCE(int8_t, INT8_MIN, INT8_MAX);
DYNAMIC_UINT_SOURCE(uint16_t, UINT16_MAX);
DYNAMIC_INT_SOURCE(int16_t, INT16_MIN, INT16_MAX);
DYNAMIC_UINT_SOURCE(uint32_t, UINT32_MAX);
DYNAMIC_INT_SOURCE(int32_t, INT32_MIN, INT32_MAX);
DYNAMIC_UINT_SOURCE(uint64_t, UINT64_MAX);
DYNAMIC_INT_SOURCE(int64_t, INT64_MIN, INT64_MAX);

void json::data::dynamic_data::set(const float value)
{
    this->__value = json::parsing::get_number_string<float>(value, FLOAT_FORMAT);
}

json::data::dynamic_data::operator float() const
{
    return json::parsing::get_number<float>(this->__value.c_str(), FLOAT_FORMAT);
}

void json::data::dynamic_data::set(const double value)
{
    this->__value = json::parsing::get_number_string<double>(value, DOUBLE_FORMAT);
}

json::data::dynamic_data::operator double() const
{
    return json::parsing::get_number<float>(this->__value.c_str(), DOUBLE_FORMAT);
}

void json::data::dynamic_data::set(const std::string value)
{
    this->__value = json::parsing::encode_string(value.c_str());
}

json::data::dynamic_data::operator std::string() const
{
    return json::parsing::decode_string(this->__value.c_str());
}

void json::data::dynamic_data::set(const jarray value)
{
    this->__value = value.serialize();
}

json::data::dynamic_data::operator json::jarray() const
{
    return json::jarray::parse(this->__value);
}

void json::data::dynamic_data::set(const jobject value)
{
    this->__value = value.as_string();
}

json::data::dynamic_data::operator json::jobject() const
{
    return json::jobject::parse(this->__value);
}

bool json::data::dynamic_data::is_true() const
{
    return this->__value == "true";
}

bool json::data::dynamic_data::is_null() const
{
    return this->__value == "null";
}

std::string json::data::dynamic_data::as_string() const
{
    assert(this->__value.size() > 0);
    switch (this->type())
    {
    case json::jtype::not_valid:
        throw std::bad_cast();
    case json::jtype::jstring:
        return json::parsing::decode_string(this->__value.c_str());    
    default:
        return this->__value;
    }
}

std::string json::data::dynamic_data::serialize() const
{
    assert(this->__value.size() > 0);
    return this->__value;
}

json::jarray json::data::dynamic_data::as_array() const
{
    return json::jarray::parse(this->__value);
}

json::jobject json::data::dynamic_data::as_object() const
{
    return json::jobject::parse(this->__value);
}

json::data::link::link(json::jobject *parent)
    : __parent(parent)
{
    this->attach();
}

json::data::link::link(const json::data::link &other)
    : __parent(other.__parent)
{
    this->attach();
}

json::data::link& json::data::link::operator= (const json::data::link &other)
{
    this->detatch();
    this->__parent = other.__parent;
    this->attach();
    return *this;
}

void json::data::link::attach()
{
    if(this->__parent != NULL) {
        this->__parent->attach(this);
    }
}

void json::data::link::detatch()
{
    if(this->__parent != NULL) {
        this->__parent->detatch(this);
        this->__parent = NULL;
    }
}

std::string json::jarray::pretty(unsigned int indent_level) const
{
    std::string result = "";
    for(unsigned int i = 0; i < indent_level; i++) result += "\t";
    if(this->size() == 0) {
        result += "[]";
        return result;
    }
    result += "[\n";
    for (size_t i = 0; i < this->size(); i++)
    {
        switch(json::jtype::peek(*this->at(i).as_string().c_str())) {
            case json::jtype::jarray:
                result += json::jarray::parse(this->at(i).as_string()).pretty(indent_level + 1);
                break;
            case json::jtype::jobject:
                result += json::jobject::parse(this->at(i).as_string()).pretty(indent_level + 1);
                break;
            default:
                for(unsigned int j = 0; j < indent_level + 1; j++) result += "\t";
                result += this->at(i).serialize();
                break;
        }

        result += ",\n";
    }
    result.erase(result.size() - 2, 1);
    for(unsigned int i = 0; i < indent_level; i++) result += "\t";
    result += "]";
    return result;
}

json::proxy::proxy()
{ 
    // Do nothing
}

json::proxy::proxy(const json::proxy &other)
    : __parent(other.__parent),
    __key(other.__key)
{
    // Do nothing
}

json::proxy::proxy(json::jobject *parent, const std::string key)
    : __parent(json::data::link(parent)),
    __key(key)
{
    if(key.length() == 0) throw std::invalid_argument(__FUNCTION__);
}

json::proxy::proxy(json::jobject *parent, const char *key)
    : __parent(json::data::link(parent)),
    __key(key)
{
    if(strlen(key) == 0) throw std::invalid_argument(__FUNCTION__);
}

json::proxy& json::proxy::operator=(const json::proxy &other)
{
    if(this == &other) return *this;

    this->__parent = other.__parent;
    this->__key = other.__key;
    return *this;
}

json::jtype::jtype json::proxy::type() const
{
    if(!this->__parent.attached() || this->__key.size() == 0) throw std::runtime_error(__FUNCTION__);
    const json::jobject *parent = this->__parent.parent();
    if(!parent->has_key(this->__key)) return json::jtype::not_valid;
    return json::jtype::peek(parent->get(this->__key)[0]);
}

void json::proxy::set_null()
{
    if(!this->__parent.attached() || this->__key.size() == 0) throw std::runtime_error(__FUNCTION__);
    this->__parent.parent()->set(this->__key, "null");
}

void json::proxy::set_true()
{
    if(!this->__parent.attached() || this->__key.size() == 0) throw std::runtime_error(__FUNCTION__);
    this->__parent.parent()->set(this->__key, "true");
}

void json::proxy::set_false()
{
    if(!this->__parent.attached() || this->__key.size() == 0) throw std::runtime_error(__FUNCTION__);
    this->__parent.parent()->set(this->__key, "false");
}

template<typename T>
T get_obj_value(const json::jobject * obj, const std::string &key)
{
    if(obj == NULL || key.size() == 0) throw std::runtime_error(__FUNCTION__);
    if(!obj->has_key(key)) throw json::invalid_key(key);
    const std::string raw = obj->get(key);
    const json::reader stream = json::reader::parse(raw);
    return json::data::dynamic_data(stream);
}

template<typename T>
void set_obj_value(json::jobject * obj, const std::string &key, const T input)
{
    if(obj == NULL || key.size() == 0) throw std::runtime_error(__FUNCTION__);
    json::data::dynamic_data value;
    value = input;
    obj->set(key, value.serialize());
}

#define JSON_PROXY_GET(format)              \
json::proxy::operator format() const  \
{ return get_obj_value<format>(this->__parent.parent(), this->__key); }

JSON_PROXY_GET(uint8_t);
JSON_PROXY_GET(int8_t);
JSON_PROXY_GET(uint16_t);
JSON_PROXY_GET(int16_t);
JSON_PROXY_GET(uint32_t);
JSON_PROXY_GET(int32_t);
JSON_PROXY_GET(uint64_t);
JSON_PROXY_GET(int64_t);
JSON_PROXY_GET(float);
JSON_PROXY_GET(double);

#define JSON_PROXY_SET(format)                  \
void json::proxy::set(const format input) \
{ return set_obj_value<format>(this->__parent.parent(), this->__key, input); }

JSON_PROXY_SET(uint8_t);
JSON_PROXY_SET(int8_t);
JSON_PROXY_SET(uint16_t);
JSON_PROXY_SET(int16_t);
JSON_PROXY_SET(uint32_t);
JSON_PROXY_SET(int32_t);
JSON_PROXY_SET(uint64_t);
JSON_PROXY_SET(int64_t);
JSON_PROXY_SET(float);
JSON_PROXY_SET(double);

void json::proxy::set(const std::string value)
{
    if(!this->__parent.attached() || this->__key.size() == 0) throw std::runtime_error(__FUNCTION__);
    json::jobject *parent = this->__parent.parent();
    parent->set(this->__key, json::parsing::encode_string(value.c_str()));
}

void json::proxy::set(const json::jarray value)
{
    set_obj_value(this->__parent.parent(), this->__key, value);
}

void json::proxy::set(const json::jobject value)
{
    set_obj_value(this->__parent.parent(), this->__key, value);
}

bool json::proxy::is_true() const
{
    if(!this->__parent.attached() || this->__key.size() == 0) throw std::runtime_error(__FUNCTION__);
    const json::jobject *parent = this->__parent.parent();
    if(!parent->has_key(this->__key)) throw json::invalid_key(this->__key);
    return strcmp(parent->get(this->__key).c_str(), "true") == 0;
}

bool json::proxy::is_null() const
{
    if(!this->__parent.attached() || this->__key.size() == 0) throw std::runtime_error(__FUNCTION__);
    const json::jobject *parent = this->__parent.parent();
    if(!parent->has_key(this->__key)) throw json::invalid_key(this->__key);
    return strcmp(parent->get(this->__key).c_str(), "null") == 0;
}

std::string json::proxy::as_string() const
{
    if(!this->__parent.attached() || this->__key.size() == 0) throw std::runtime_error(__FUNCTION__);
    const json::jobject *parent = this->__parent.parent();
    if(!parent->has_key(this->__key)) throw json::invalid_key(this->__key);
    return this->type() == json::jtype::jstring ? 
        json::parsing::decode_string(parent->get(this->__key).c_str())
        : parent->get(this->__key).c_str();
}

json::proxy::operator std::string() const
{
    return this->as_string();
}

json::jarray json::proxy::as_array() const
{
    if(!this->__parent.attached() || this->__key.size() == 0) throw std::runtime_error(__FUNCTION__);
    const json::jobject *parent = this->__parent.parent();
    if(!parent->has_key(this->__key)) throw json::invalid_key(this->__key);
    return json::jarray::parse(parent->get(this->__key));
}

json::proxy::operator json::jarray() const
{
    return this->as_array();
}

json::jobject json::proxy::as_object() const
{
    if(!this->__parent.attached() || this->__key.size() == 0) throw std::runtime_error(__FUNCTION__);
    const json::jobject *parent = this->__parent.parent();
    if(!parent->has_key(this->__key)) throw json::invalid_key(this->__key);
    return json::jobject::parse(parent->get(this->__key));
}

json::proxy::operator json::jobject() const
{
    return this->as_object();
}

std::string json::proxy::serialize() const
{
    if(!this->__parent.attached() || this->__key.size() == 0) throw std::runtime_error(__FUNCTION__);
    const json::jobject *parent = this->__parent.parent();
    if(!parent->has_key(this->__key)) throw json::invalid_key(this->__key);
    return parent->get(this->__key);
}