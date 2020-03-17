#include "json.h"

void json::parsing::tlws(std::string &input)
{
    while (input.size() > 0 && std::isspace(input[0])) input.erase(0, 1);
}

json::jtype::jtype json::jtype::detect(const std::string &input)
{
    std::string value(input);
    json::parsing::tlws(value);
    if (value.size() == 0) return json::jtype::not_valid;
    switch (value.at(0))
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
        return (value.substr(0, 4) == "true" || value.substr(0, 5) == "false") ? json::jtype::jbool : json::jtype::not_valid;
        break;
    case 'n':
        return (value.substr(0, 4) == "null") ? json::jtype::jnull : json::jtype::not_valid;
        break;
    default:
        return json::jtype::not_valid;
        break;
    }
}

std::string json::parsing::read_digits(std::string &input)
{
    // Trim leading white space
    json::parsing::tlws(input);

    // Initialize the result
    std::string result;

    // Loop until all digits are read
    while (
        input.size() > 0 &&
        (
            input.at(0) == '0' ||
            input.at(0) == '1' ||
            input.at(0) == '2' ||
            input.at(0) == '3' ||
            input.at(0) == '4' ||
            input.at(0) == '5' ||
            input.at(0) == '6' ||
            input.at(0) == '7' ||
            input.at(0) == '8' ||
            input.at(0) == '9'
            )
        )
    {
        result += input[0];
        input.erase(0, 1);
    }

    // Return the result
    return result;
}

std::string json::parsing::escape_quotes(const std::string &input)
{
    std::string parsed;
    for (size_t i = 0; i < input.size(); i++)
    {
        if (input[i] == '\"' && parsed[parsed.size() - 1] != '\\')
        {
            parsed += '\\';
        }
        parsed += input[i];
    }
    return parsed;
}

std::string json::parsing::unescape_quotes(const std::string &input)
{
    std::string result;
    std::string value(input);
    while (value.size() > 0)
    {
        if (value.size() > 1 && value[0] == '\\' && value[1] == '\"')
        {
            result += '\"';
            value.erase(0, 2);
        }
        else
        {
            result += value[0];
            value.erase(0, 1);
        }
    }
    return result;
}

json::parsing::parse_results json::parsing::parse(std::string &value)
{
    // Strip white space
    json::parsing::tlws(value);

    // Validate input
    if (value.size() == 0) throw json::parsing_error("Input was only whitespace");

    // Initialize the output
    json::parsing::parse_results result;

    // Detect the type
    result.type = json::jtype::detect(value);

    // Parse the values
    switch (result.type)
    {
    case json::jtype::jstring:
        // Validate the input
        if (value[0] != '"') throw json::parsing_error("Expected '\"' as first character");

        // Remove the opening quote
        value.erase(0, 1);

        // Copy the string
        while (value.size() > 0)
        {
            if (value[0] != '"' || (result.value.size() > 0 && result.value[result.value.size() - 1] == '\\'))
            {
                result.value.push_back(value[0]);
                value.erase(0, 1);
            }
            else
            {
                break;
            }
        }
        if (value.size() == 0 || value[0] != '"') result.type = json::jtype::not_valid;
        else value.erase(0, 1);
        break;
    case json::jtype::jnumber:
    {
        const char error[] = "Input did not contain a valid number";

        if (value.at(0) == '-')
        {
            result.value += "-";
            value.erase(0, 1);
        }

        if (value.length() < 1) throw json::parsing_error(error);

        // Read the whole digits
        std::string whole_digits = json::parsing::read_digits(value);

        // Validate the read
        if (whole_digits.length() == 0) throw json::parsing_error(error);

        // Tack on the value
        result.value += whole_digits;

        // Check for decimal number
        if (value[0] == '.')
        {
            result.value += ".";
            value.erase(0, 1);
            std::string decimal_digits = json::parsing::read_digits(value);

            if (decimal_digits.length() < 1) throw json::parsing_error(error);

            result.value += decimal_digits;
        }

        // Check for exponential number
        if (value[0] == 'e' || value[0] == 'E')
        {
            result.value += value[0];
            value.erase(0, 1);

            if (value.size() == 0) throw json::parsing_error(error);

            if (value[0] == '+' || value[0] == '-')
            {
                result.value += value[0];
                value.erase(0, 1);
            }

            if (value.size() == 0) throw json::parsing_error(error);

            std::string exponential_digits = json::parsing::read_digits(value);

            if (exponential_digits.size() == 0) throw json::parsing_error(error);

            result.value += exponential_digits;
        }
        break;
    }
    case json::jtype::jobject:
    {
        const char error[] = "Input did not contain a valid object";

        // The first character should be an open bracket
        if (value[0] != '{') throw json::parsing_error(error);
        result.value += '{';
        value.erase(0, 1);
        json::parsing::tlws(value);

        // Loop until the closing bracket is encountered
        while (value.size() > 0 && value[0] != '}')
        {
            // Read the key
            json::parsing::parse_results key = json::parsing::parse(value);

            // Validate that the key is a string
            if (key.type != json::jtype::jstring) throw json::parsing_error(error);

            // Store the key
            result.value += "\"" + json::parsing::escape_quotes(key.value) + "\"";
            json::parsing::tlws(value);

            // Look for the colon
            if (value[0] != ':') throw json::parsing_error(error);
            result.value += ':';
            value.erase(0, 1);

            // Get the value
            json::parsing::parse_results subvalue = json::parsing::parse(value);

            // Validate the value type
            if (subvalue.type == json::jtype::not_valid) throw json::parsing_error(error);

            // Store the value
            if (subvalue.type == json::jtype::jstring) result.value += "\"" + json::parsing::escape_quotes(subvalue.value) + "\"";
            else result.value += subvalue.value;
            json::parsing::tlws(value);

            // Validate format
            if (value[0] != ',' && value[0] != '}') throw json::parsing_error(error);

            // Check for next line
            if (value[0] == ',')
            {
                result.value += ',';
                value.erase(0, 1);
            }
        }
        if (value.size() == 0 || value[0] != '}') throw json::parsing_error(error);
        result.value += '}';
        value.erase(0, 1);
        break;
    }
    case json::jtype::jarray:
    {
        const char error[] = "Input did not contain a valid array";
        if (value[0] != '[') throw json::parsing_error(error);
        result.value += '[';
        value.erase(0, 1);
        json::parsing::tlws(value);
        if (value.size() == 0) throw json::parsing_error(error);
        while (value.size() > 0 && value[0] != ']')
        {
            json::parsing::parse_results array_value = json::parsing::parse(value);
            if (array_value.type == json::jtype::not_valid) throw json::parsing_error(error);
            if (array_value.type == json::jtype::jstring) result.value += "\"" + json::parsing::escape_quotes(array_value.value) + "\"";
            else result.value += array_value.value;
            json::parsing::tlws(value);
            if (value[0] != ',' && value[0] != ']') throw json::parsing_error(error);
            if (value[0] == ',')
            {
                result.value += ',';
                value.erase(0, 1);
            }
        }
        if (value.size() == 0 || value[0] != ']') throw json::parsing_error(error);
        result.value += ']';
        value.erase(0, 1);
        break;
    }
    case json::jtype::jbool:
    {
        if (value.size() < 4) throw json::parsing_error("Input did not contain a valid boolean");
        if (value.substr(0, 4).compare("true") == 0)
        {
            result.value += "true";
            value.erase(0, 4);
        }
        else if (value.size() > 4 && value.substr(0, 5).compare("false") == 0)
        {
            result.value += "false";
            value.erase(0, 5);
        }
        else
        {
            throw json::parsing_error("Input did not contain a valid boolean");
        }
        break;
    }
    case json::jtype::jnull:
    {
        if (value.size() < 4) throw json::parsing_error("Input did not contain a valid null");
        if (value.substr(0, 4) == "null")
        {
            result.value += "null";
            value.erase(0, 4);
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

    return result;
}

std::vector<std::string> json::parsing::parse_array(std::string value)
{
    json::parsing::tlws(value);
    if (value[0] != '[') throw json::parsing_error("Input was not an array");
    value.erase(0, 1);
    json::parsing::tlws(value);
    if (value[0] == ']')
    {
        // Empty array
        value.erase(0, 1);
        return std::vector<std::string>();
    }
    std::vector<std::string> result;
    const char error[] = "Input was not properly formated";
    while (value.size() > 0)
    {
        json::parsing::tlws(value);
        json::parsing::parse_results parse_results = json::parsing::parse(value);
        if (parse_results.type == json::jtype::not_valid) throw json::parsing_error(error);
        result.push_back(parse_results.value);
        json::parsing::tlws(value);
        if (value[0] == ']') break;
        if (value[0] == ',') value.erase(0, 1);
    }
    if (value[0] != ']') throw json::parsing_error(error);
    value.erase(0, 1);
    return result;
}

void json::jobject::proxy::set_array(const std::vector<std::string> &values, const bool wrap)
{
    std::string value = "[";
    for (size_t i = 0; i < values.size(); i++)
    {
        if (wrap) value += "\"" + json::parsing::escape_quotes(values[i]) + "\",";
        else value += values[i] + ",";
    }
    if(values.size() > 0) value.erase(value.size() - 1, 1);
    value += "]";
    this->sink.set(key, value);
}

json::jobject json::jobject::parse(const std::string &input)
{
    std::string temp(input);
    const char error[] = "Input is not a valid object";
    json::parsing::tlws(temp);
    if (temp[0] != '{') throw json::parsing_error(error);
    temp.erase(0, 1);
    json::parsing::tlws(temp);
    if (temp.size() == 0) throw json::parsing_error(error);
    json::jobject result;
    while (temp.size() > 0 && temp[0] != '}')
    {
        // Get key
        kvp entry;
        json::parsing::parse_results key = json::parsing::parse(temp);
        if (key.type != json::jtype::jstring || key.value == "") throw json::parsing_error(error);
        entry.first = key.value;

        // Get value
        json::parsing::tlws(temp);
        if (temp[0] != ':') throw json::parsing_error(error);
        temp.erase(0, 1);
        json::parsing::tlws(temp);
        json::parsing::parse_results value = json::parsing::parse(temp);
        if (value.type == json::jtype::not_valid) throw json::parsing_error(error);
        if (value.type == json::jtype::jstring) entry.second = "\"" + value.value + "\"";
        else entry.second = value.value;

        // Clean up
        json::parsing::tlws(temp);
        if (temp[0] != ',' && temp[0] != '}') throw json::parsing_error(error);
        if (temp[0] == ',') temp.erase(0, 1);
        result += entry;

    }
    if (temp.size() == 0 || temp[0] != '}') throw json::parsing_error(error);
    temp.erase(0, 1);
    return result;
}

void json::jobject::set(const std::string &key, const std::string &value)
{
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
            this->data.erase(this->data.begin() + i, this->data.begin() + i + 1);
        }
    }
}

json::jobject::operator std::string() const
{
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