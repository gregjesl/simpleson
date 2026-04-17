/*! \file json.cpp
 * \brief Simpleson source file
 */

#include "json.h"
#include <string.h>
#include <assert.h>

namespace {
/*! \brief Safe whitespace test for signed char / UTF-8 bytes (avoids UB in std::isspace). */
inline bool json_isspace(char c) noexcept
{
	return std::isspace(static_cast<unsigned char>(c)) != 0;
}

/*! \brief True if C string is empty (first char is \\0). @warning \p str must not be null. */
inline bool empty_c_string(const char* str) noexcept
{
	return *str == '\0';
}

/*! \brief Advance \p str past leading whitespace (see json::parsing::tlws). */
inline void skip_white_space(const char*& str) noexcept
{
	str = json::parsing::tlws(str);
}

/*! \brief Closing bracket/brace for array vs object parse. */
inline bool end_character_encountered(const json::jobject& obj, const char* index) noexcept
{
	return obj.is_array() ? *index == ']' : *index == '}';
}

inline bool is_json_digit(char input) noexcept
{
	return input >= '0' && input <= '9';
}
}

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
    while(!empty_c_string(output) && json_isspace(*output)) output++;
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
    if(this->sub_reader != nullptr) {
        this->sub_reader.reset();
    }
    this->read_state = 0;
}

json::reader::push_result json::reader::push(const char next)
{
    // Check for opening whitespace
    if(this->length() == 0 && json_isspace(next)) return reader::ACCEPTED;

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
    return is_json_digit(input) || (input >= 'a' && input <= 'f') || (input >= 'A' && input <= 'F');
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

    const auto handle_reading_value = [this, next]() -> push_result {
        assert(this->sub_reader != nullptr);
        if(this->sub_reader->is_valid() && json_isspace(next)) return WHITESPACE;
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
                {
                    std::string piece;
                    this->sub_reader->readout(piece);
                    this->reserve(this->size() + piece.size());
                    this->append(std::move(piece));
                }
                this->sub_reader.reset();
                this->push_back(next);
                this->set_state(ARRAY_CLOSED);
                return ACCEPTED;
            case ',':
                if(!this->sub_reader->is_valid()) return REJECTED;
                {
                    std::string piece;
                    this->sub_reader->readout(piece);
                    this->reserve(this->size() + piece.size());
                    this->append(std::move(piece));
                }
                this->sub_reader.reset();
                this->push_back(next);
                this->set_state(ARRAY_AWAITING_NEXT_LINE);
                return ACCEPTED;
            default:
                return REJECTED;
            }
        }
        return REJECTED;
    };

    switch (state)
    {
    case ARRAY_EMPTY:
        assert(this->sub_reader == nullptr);
        if(next == '[') {
            this->set_state(ARRAY_OPEN_BRACKET);
            this->push_back(next);
            return ACCEPTED;
        }
        return REJECTED;
    case ARRAY_OPEN_BRACKET:
        assert(this->sub_reader == nullptr);
        if(json_isspace(next)) return WHITESPACE;
        if(next == ']') {
            this->set_state(ARRAY_CLOSED);
            this->push_back(next);
            return ACCEPTED;
        }
        if(json::jtype::peek(next) == json::jtype::not_valid) return REJECTED;
        this->sub_reader = std::make_unique<reader>();
        this->set_state(ARRAY_READING_VALUE);
        return handle_reading_value();
    case ARRAY_READING_VALUE:
        return handle_reading_value();
    case ARRAY_AWAITING_NEXT_LINE:
        if(json_isspace(next)) return WHITESPACE;
        if(json::jtype::peek(next) == json::jtype::not_valid) return REJECTED;
        this->sub_reader = std::make_unique<reader>();
        this->set_state(ARRAY_READING_VALUE);
        return handle_reading_value();
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
        assert(this->sub_reader == nullptr);
        if(next == '{') {
            this->set_state(OBJECT_OPEN_BRACE);
            this->push_back(next);
            return ACCEPTED;
        }
        return REJECTED;
    case OBJECT_OPEN_BRACE:
        assert(this->sub_reader == nullptr);
        if(next == '}') {
            this->set_state(OBJECT_CLOSED);
            this->push_back(next);
            return ACCEPTED;
        }
        // Fall-through deliberate
    case OBJECT_AWAITING_NEXT_LINE:
        if(json_isspace(next)) return WHITESPACE;
        if(next != '"') return REJECTED;
        this->sub_reader = std::make_unique<kvp_reader>();
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
        assert(this->sub_reader != nullptr);
        switch (this->sub_reader->push(next))
        {
        case ACCEPTED:
            return ACCEPTED;
        case WHITESPACE:
            return WHITESPACE;
        case REJECTED:
            if(!this->sub_reader->is_valid()) return REJECTED;
            if(json_isspace(next)) return WHITESPACE;
            switch (next)
            {
            case '}':
                {
                    std::string piece;
                    this->sub_reader->readout(piece);
                    this->reserve(this->size() + piece.size());
                    this->append(std::move(piece));
                }
                this->sub_reader.reset();
                this->push_back(next);
                this->set_state(OBJECT_CLOSED);
                return ACCEPTED;
            case ',':
                {
                    std::string piece;
                    this->sub_reader->readout(piece);
                    this->reserve(this->size() + piece.size());
                    this->append(std::move(piece));
                }
                this->sub_reader.reset();
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
        } else if(is_json_digit(next)) {
            this->set_state(next == '0' ? NUMBER_ZERO : NUMBER_INTEGER_DIGITS);
            this->push_back(next);
            return ACCEPTED;
        }
        return REJECTED;
    case NUMBER_OPEN_NEGATIVE:
        if(is_json_digit(next)) {
            this->set_state(next == '0' ? NUMBER_ZERO : NUMBER_INTEGER_DIGITS);
            this->push_back(next);
            return ACCEPTED;
        }
        return REJECTED;
    case NUMBER_INTEGER_DIGITS:
        assert(is_json_digit(this->back()));
        if(is_json_digit(next)) {
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
        if(is_json_digit(next)) {
            this->set_state(NUMBER_FRACTION_DIGITS);
            this->push_back(next);
            return ACCEPTED;
        }
        return REJECTED;
    case NUMBER_FRACTION_DIGITS:
        assert(is_json_digit(this->back()));
        if(is_json_digit(next)) {
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
        if(is_json_digit(next)) {
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
    const char *str = nullptr;

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
        if(json_isspace(next)) return WHITESPACE;
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
        if(json_isspace(next)) return WHITESPACE;
        if(next == ':') {
            this->_colon_read = true;
            return ACCEPTED;
        }
        return REJECTED;
    }

    // At this point the colon should be read
    assert(this->_colon_read);

    // Check for a fresh start
    if(reader::length() == 0 && json_isspace(next))
    {
        assert(reader::get_state<char>() == 0);
        return WHITESPACE;
    }
    return reader::push(next);
}

void json::kvp_reader::readout(std::string& out) const
{
    this->_key.readout(out);
    out.reserve(out.size() + 1 + this->length());
    out.push_back(':');
    out.append(static_cast<const std::string&>(static_cast<const reader&>(*this)));
}

std::string json::kvp_reader::readout() const
{
    std::string s;
    readout(s);
    return s;
}

std::string json::parsing::read_digits(const char *input)
{
    const char *index = json::parsing::tlws(input);
    const char *const digit_start = index;
    while (
        !empty_c_string(index) &&
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
        index++;
    }
    std::string result;
    const size_t n = static_cast<size_t>(index - digit_start);
    result.reserve(n);
    for (const char *p = digit_start; p < index; ++p)
    {
        result.push_back(*p);
    }
    return result;
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

    while (!empty_c_string(input))
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
    if (empty_c_string(index)) throw json::parsing_error("Input was only whitespace");

    // Initialize the output
    json::parsing::parse_results result;
    result.type = json::jtype::not_valid;

    // Initialize the reader
    json::reader stream;

    // Iterate
    while(/*!empty_c_string(input) && */stream.push(*index) != json::reader::REJECTED)
    {
        index++;
    }

    if(stream.is_valid()) {
        stream.readout(result.value);
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
    skip_white_space(index);
    if (*index == ']')
    {
        return result;
    }
    const char error[] = "Input was not properly formated";
    while (!empty_c_string(index))
    {
        skip_white_space(index);
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
        if (wrap) value += json::parsing::encode_string(values[i].c_str()) + ",";
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
    /*json::reader stream;*/
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
    skip_white_space(index);
    if (empty_c_string(index)) throw json::parsing_error(error);

    while (!empty_c_string(index) && !end_character_encountered(result, index))
    {
        // Get key
        kvp entry;

        if(!result.is_array()) {
            json::parsing::parse_results key = json::parsing::parse(index);
            if (key.type != json::jtype::jstring || key.value == "") throw json::parsing_error(error);
            entry.first = json::parsing::decode_string(key.value.c_str());
            index = key.remainder;

            // Get value
            skip_white_space(index);
            if (*index != ':') throw json::parsing_error(error);
            index++;
        }

        skip_white_space(index);
        json::parsing::parse_results value = json::parsing::parse(index);
        if (value.type == json::jtype::not_valid) throw json::parsing_error(error);
        entry.second = value.value;
        index = value.remainder;

        // Clean up
        skip_white_space(index);
        if (*index != ',' && !end_character_encountered(result, index)) throw json::parsing_error(error);
        if (*index == ',') index++;
        result += entry;

    }
    if (empty_c_string(index) || !end_character_encountered(result, index)) throw json::parsing_error(error);
    index++;
    return result;
}

json::key_list_t json::jobject::list_keys() const
{
    // Initialize the result
    key_list_t result;

    // Return an empty list if the object is an array
    if(this->is_array()) return result;

    for(size_t i = 0; i < this->data.size(); i++)
    {
        result.push_back(this->data.at(i).first);
    }
    return result;
}

void json::jobject::set(const std::string &key, const std::string &value)
{
    if(this->array_flag) throw json::invalid_key(key);
    size_t slot{};
    if (this->find_key_index(key, slot))
    {
        this->data.at(slot).second = value;
        return;
    }
    kvp entry;
    entry.first = key;
    entry.second = value;
    this->data.push_back(entry);
    this->key_index.emplace(jobject::key_hash_of(key), this->data.size() - 1);
}

void json::jobject::remove(const std::string &key)
{
    if (!this->array_flag)
    {
        size_t idx{};
        if (this->find_key_index(key, idx))
            this->remove(idx);
        return;
    }
    for (size_t i = 0; i < this->size(); i++)
    {
        if (this->data.at(i).first == key)
        {
            this->remove(i);
        }
    }
}

void json::jobject::remove(const size_t index)
{
    if (!this->array_flag)
    {
        const std::string erased_key = this->data.at(index).first;
        const std::uint64_t h = jobject::key_hash_of(erased_key);
        const auto range = this->key_index.equal_range(h);
        for (auto it = range.first; it != range.second; ++it)
        {
            if (it->second == index)
            {
                this->key_index.erase(it);
                break;
            }
        }
        this->data.erase(this->data.begin() + static_cast<std::ptrdiff_t>(index));
        for (auto &e : this->key_index)
        {
            if (e.second > index)
                --e.second;
        }
    }
    else
    {
        this->data.erase(this->data.begin() + static_cast<std::ptrdiff_t>(index));
    }
}

json::jobject::operator std::string() const
{
    if (is_array()) {
        if (this->size() == 0) return "[]";
        size_t total = 2;
        for (size_t i = 0; i < this->size(); i++)
            total += this->data.at(i).second.size();
        total += this->size() > 0 ? this->size() - 1 : 0;
        std::string result;
        result.reserve(total);
        result.push_back('[');
        for (size_t i = 0; i < this->size(); i++)
        {
            if (i != 0) result.push_back(',');
            result.append(this->data.at(i).second);
        }
        result.push_back(']');
        return result;
    } else {
        if (this->size() == 0) return "{}";
        size_t est = 2;
        for (size_t i = 0; i < this->size(); i++)
        {
            const std::string &k = this->data.at(i).first;
            const std::string &v = this->data.at(i).second;
            est += 2 + k.size() * 2 + 1 + v.size() + 1;
        }
        std::string result;
        result.reserve(est);
        result.push_back('{');
        for (size_t i = 0; i < this->size(); i++)
        {
            if (i != 0) result.push_back(',');
            result.append(json::parsing::encode_string(this->data.at(i).first.c_str()));
            result.push_back(':');
            result.append(this->data.at(i).second);
        }
        result.push_back('}');
        return result;
    }
}

std::string json::jobject::pretty(unsigned int indent_level) const
{
    std::string result;
    size_t est = static_cast<size_t>(indent_level) + 64;
    for (size_t i = 0; i < this->size(); i++)
    {
        est += this->data.at(i).first.size() + this->data.at(i).second.size();
        est += static_cast<size_t>(indent_level) * 8 + 32;
    }
    result.reserve(est);

    result.append(indent_level, '\t');
    if (is_array()) {
        if(this->size() == 0) {
            result.append("[]");
            return result;
        }
        result.append("[\n");
        for (size_t i = 0; i < this->size(); i++)
        {
            switch(json::jtype::peek(*this->data.at(i).second.c_str())) {
                case json::jtype::jarray:
                case json::jtype::jobject:
                    result.append(json::jobject::parse(this->data.at(i).second).pretty(indent_level + 1));
                    break;
                default:
                    result.append(indent_level + 1, '\t');
                    result.append(this->data.at(i).second);
                    break;
            }

            result.append(",\n");
        }
        result.erase(result.size() - 2, 1);
        result.append(indent_level, '\t');
        result.push_back(']');
    } else {
        if(this->size() == 0) {
            result.append("{}");
            return result;
        }
        result.append("{\n");
        for (size_t i = 0; i < this->size(); i++)
        {
            result.append(indent_level + 1, '\t');
            result.push_back('"');
            result.append(this->data.at(i).first);
            result.append("\": ");
            switch(json::jtype::peek(*this->data.at(i).second.c_str())) {
                case json::jtype::jarray:
                case json::jtype::jobject:
                    result.append(json::parsing::tlws(json::jobject::parse(this->data.at(i).second).pretty(indent_level + 1).c_str()));
                    break;
                default:
                    result.append(this->data.at(i).second);
                    break;
            }

            result.append(",\n");
        }
        result.erase(result.size() - 2, 1);
        result.append(indent_level, '\t');
        result.push_back('}');
    }
    return result;
}