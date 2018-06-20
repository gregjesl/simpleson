#include "json.h"

// Hide pragma warnings on GCC
#ifdef GNUC
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#endif

#pragma region JTYPE

json::jtype::jtype json::jtype::detect(const std::string input)
{
	std::string value = json::remove_leading_spaces(input);
	if (value.size() == 0)
	{
		return json::jtype::not_valid;
	}
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
		if (value.substr(0, 4) == "true" || value.substr(0, 5) == "false")
		{
			return json::jtype::jbool;
		}
		else
		{
			return json::jtype::not_valid;
		}
		break;
	case 'n':
		if (value.substr(0, 4) == "null")
		{
			return json::jtype::jnull;
		}
		else
		{
			return json::jtype::not_valid;
		}
		break;
	default:
		return json::jtype::not_valid;
		break;
	}
}
#pragma endregion

#pragma region JNUMBER

// Private constructor
json::jnumber::jnumber(const std::string input) : basic::istringnumber()
{
	this->string = input;
}

json::jnumber json::jnumber::parse(const std::string input)
{
	std::string remainder = std::string();
	return json::jnumber::parse(input, remainder);
}

json::jnumber json::jnumber::parse(const std::string input, std::string& remainder)
{
	remainder = json::remove_leading_spaces(input);
	if (remainder.length() < 1)
	{
		throw std::invalid_argument("Input did not contain a valid number");
	}

	std::string value = "";
	if (remainder.at(0) == '-')
	{
		value += "-";
		remainder.erase(0, 1);
	}

	if (remainder.length() < 1)
	{
		throw std::invalid_argument("Input did not contain a valid number");
	}

	std::string remainder2;
	std::string digits = json::jnumber::read_digits(remainder, remainder2);
	if (digits.length() < 1)
	{
		throw std::invalid_argument("Input did not contain a valid number");
	}
	value += digits;
	remainder = remainder2;

	// Exit if finished
	if (remainder.length() < 1)
	{
		return json::jnumber(value);
	}

	if (remainder.at(0) == '.')
	{
		value += ".";
		remainder.erase(0, 1);
		digits = json::jnumber::read_digits(remainder, remainder2);
		if (digits.length() < 1)
		{
			throw std::invalid_argument("Input did not contain a valid number");
		}
		value += digits;
		remainder = remainder2;
	}

	// Exit if finished
	if (remainder.length() < 1)
	{
		return json::jnumber(value);
	}
	switch (remainder.at(0))
	{
	case 'e':
	case 'E':
		value += remainder.substr(0, 1);
		remainder.erase(0, 1);
		if (remainder.length() < 1)
		{
			throw std::invalid_argument("Input did not contain a valid number");
		}
		break;
	default:
		return json::jnumber(value);
		break;
	}

	if (remainder.at(0) == '+')
	{
		value += "+";
		remainder.erase(0, 1);
	}
	else if (remainder.at(0) == '-')
	{
		value += "-";
		remainder.erase(0, 1);
	}

	if (remainder.length() < 1)
	{
		throw std::invalid_argument("Input did not contain a valid number");
	}

	digits = json::jnumber::read_digits(remainder, remainder2);
	if (digits.length() < 1)
	{
		throw std::invalid_argument("Input did not contain a valid number");
	}
	value += digits;
	remainder = remainder2;

	return json::jnumber(value);
}

std::string json::jnumber::read_digits(const std::string input)
{
	std::string remainder = std::string();
	return json::jnumber::read_digits(input, remainder);
}

std::string json::jnumber::read_digits(const std::string input, std::string& remainder)
{
	remainder = json::remove_leading_spaces(input);
	if (remainder.size() < 1)
	{
		throw std::invalid_argument("Input string did not contain a valid digit stream");
	}
	std::string value = "";
	while (
		remainder.at(0) == '0' ||
		remainder.at(0) == '1' ||
		remainder.at(0) == '2' ||
		remainder.at(0) == '3' ||
		remainder.at(0) == '4' ||
		remainder.at(0) == '5' ||
		remainder.at(0) == '6' ||
		remainder.at(0) == '7' ||
		remainder.at(0) == '8' ||
		remainder.at(0) == '9'
		)
	{
		value += remainder.substr(0, 1);
		remainder.erase(0, 1);
		if (remainder.length() < 1)
		{
			return value;
		}
	}
	if (value == "")
	{
		throw std::invalid_argument("Input string did not contain a valid digit stream");
	}
	return value;
}

#pragma endregion

#pragma region JARRAY

json::jarray json::jarray::parse(const std::string input)
{
	std::string remainder = std::string();
	return json::jarray::parse(input, remainder);
}

json::jarray json::jarray::parse(const std::string input, std::string& remainder)
{
	remainder = json::remove_leading_spaces(input);

	if (remainder.length() < 1 || remainder.at(0) != '[')
	{
		throw std::invalid_argument("Expected '[', found '" + remainder.substr(0, 1) + "'");
	}
	remainder.erase(0, 1);

	std::string remainder2 = json::remove_leading_spaces(remainder);
	remainder = remainder2;
	if (remainder.length() < 1)
	{
		throw std::invalid_argument("Invalid array string");
	}
	if (remainder.at(0) == ']')
	{
		remainder.erase(0, 1);
		return json::jarray();
	}

	// Create the object
	json::jarray result;

	// Detect the type
	result.type = json::jtype::detect(remainder);

	switch (result.type)
	{
	case json::jtype::jstring:
		break;
	case json::jtype::jnumber:
		result.push_back((std::string)(json::jnumber::parse(remainder, remainder2)));
		break;
	case json::jtype::jobject:
		break;
	case json::jtype::jarray:
		result.push_back((json::jarray::parse(remainder, remainder2)).to_string());
		break;
	case json::jtype::jbool:
		break;
	case json::jtype::jnull:
		// ???
		break;
	case json::jtype::not_valid:
		throw std::invalid_argument("Invalid array string");
		break;
	default:
		break;
	}

	remainder = json::remove_leading_spaces(remainder2);

	while (remainder.at(0) == ',')
	{
		// Move to the next element
		remainder.erase(0, 1);
		remainder2 = json::remove_leading_spaces(remainder);
		remainder = remainder2;
		if (remainder.length() < 1)
		{
			throw std::invalid_argument("Invalid array string");
		}

		// Get the type if not already determined
		if (result.type == json::jtype::jnull)
		{
			result.type = json::jtype::detect(remainder);
		}

		result.push_back(json::jvalue::parse(remainder, remainder2));
		remainder = remainder2;

		// Move to the next one
		remainder2 = json::remove_leading_spaces(remainder);
		remainder = remainder2;
		if (remainder.length() < 1)
		{
			throw std::invalid_argument("Invalid array string");
		}
	}

	// Finish up
	remainder2 = json::remove_leading_spaces(remainder);
	remainder = remainder2;
	if (remainder.length() < 1 || remainder.at(0) != ']')
	{
		throw std::invalid_argument("Invalid array string");
	}
	remainder.erase(0, 1);
	return result;
}

std::string json::jarray::to_string(void)
{
	if (this->size() == 0)
	{
		return "[]";
	}
	std::string result = "[" + this->at(0);
	if (this->size() > 1)
	{
		for (size_t i = 1; i < this->size(); i++)
		{
			result += ", " + this->at(i);
		}
	}
	result += "]";
	return result;
}

#pragma endregion

#pragma region KEY_VALUE_PAIR

json::key_value_pair json::key_value_pair::parse(const std::string input)
{
	std::string remainder = std::string();
	return json::key_value_pair::parse(input, remainder);
}

json::key_value_pair json::key_value_pair::parse(const std::string input, std::string& remainder)
{
	remainder = json::remove_leading_spaces(input);
	if (json::jtype::detect(remainder) != json::jtype::jstring)
	{
		throw std::invalid_argument("Expecting key");
	}

	// Load the key
	json::key_value_pair result;
	std::string remainder2;
	result.key = (std::string)json::jvalue::parse(remainder, remainder2);

	// Search for the colon
	remainder = json::remove_leading_spaces(remainder2);
	if (remainder.length() < 1 || remainder.at(0) != ':')
	{
		throw std::invalid_argument("Expecting colon");
	}
	remainder.erase(0, 1);

	// Get the value
	result.value = json::jvalue::parse(remainder, remainder2);
	remainder = remainder2;
	return result;
}

std::string json::key_value_pair::to_string(void)
{
	std::string result = "\"" + this->key + "\": ";
	if (this->value.is_string())
	{
		result += "\"" + (std::string)value + "\"";
	}
	else
	{
		result += (std::string)value;
	}
	return result;
}

#pragma endregion

#pragma region JOBJECT

json::jobject json::jobject::parse(const std::string input)
{
	std::string remainder = std::string();
	return json::jobject::parse(input, remainder);
}

json::jobject json::jobject::parse(const std::string input, std::string& remainder)
{
	remainder = json::remove_leading_spaces(input);

	if (remainder.length() < 1 || remainder.at(0) != '{')
	{
		throw std::invalid_argument("Invalid input");
	}

	remainder.erase(0, 1);
	std::string remainder2 = json::remove_leading_spaces(remainder);
	remainder = remainder2;

	if (remainder.length() < 1)
	{
		throw std::invalid_argument("Invalid input");
	}

	json::jobject result;

	if (remainder.at(0) == '}')
	{
		remainder.erase(0, 1);
		return result;
	}

	// Get the first key value pair
	json::key_value_pair pair = json::key_value_pair::parse(remainder, remainder2);
	result.push_back(pair);
	remainder = json::remove_leading_spaces(remainder2);

	while (remainder.length() > 0 && remainder.at(0) == ',')
	{
		// Remove the comma
		remainder.erase(0, 1);

		// Get the pair
		pair = json::key_value_pair::parse(remainder, remainder2);
		result.push_back(pair);
		remainder = json::remove_leading_spaces(remainder2);

		if (remainder.length() < 1)
		{
			throw std::invalid_argument("Expecting closing brace");
		}
	}

	// Clean up
	if (remainder.length() < 1 || remainder.at(0) != '}')
	{
		throw std::invalid_argument("Expecting closing brace");
	}
	remainder.erase(0, 1);
	return result;
}

std::string json::jobject::to_string(void)
{
	if (this->size() == 0)
	{
		return "{}";
	}

	std::string result = "{ " + (std::string)this->at(0);

	for (size_t i = 1; i < this->size(); i++)
	{
		result += ", " + (std::string)this->at(i);
	}
	result += " }";

	return result;
}

std::vector<std::string> json::jobject::get_keys(void)
{
	std::vector<std::string> result;
	for (size_t i = 0; i < this->size(); i++)
	{
		result.push_back(this->at(i).key);
	}
	return result;
}

bool json::jobject::has_key(const std::string key)
{
	std::vector<std::string> keys = this->get_keys();
	for (size_t i = 0; i < this->size(); i++)
	{
		if (this->at(0).key == key)
		{
			return true;
		}
	}
	return false;
}

json::key_value_pair json::jobject::get_entry(const std::string key)
{
	std::vector<std::string> keys = this->get_keys();
	for (size_t i = 0; i < this->size(); i++)
	{
		if (key.compare(this->at(i).key) == 0)
		{
			return this->at(i);
		}
	}
	throw std::invalid_argument("Key '" + key + "' does not exist");
}

#pragma endregion

#pragma region JVALUE

json::jvalue json::jvalue::parse(const std::string input)
{
	std::string remainder = std::string();
	return json::jvalue::parse(input, remainder);
}

json::jvalue json::jvalue::parse(const std::string input, std::string& remainder)
{
	json::jvalue result;
	remainder = json::remove_leading_spaces(input);

	if (remainder.length() < 1)
	{
		throw std::invalid_argument("Input was empty");
	}

	// Detect the type
	result.type = json::jtype::detect(remainder);
	if (result.type == json::jtype::not_valid)
	{
		throw std::invalid_argument("Input was not valid");
	}

	std::string value = remainder;

	switch (result.type)
	{
	case json::jtype::jstring:
		// Initialize the string
		result.string = "";

		// Check for an opening quote
		if (remainder.at(0) != '\"')
		{
			throw std::invalid_argument("Expecting '\"', was '" + remainder.substr(0, 1) + "'");
		}

		// Remove the opening quote
		remainder.erase(0, 1);

		// Ensure the string has a closing quote
		if (remainder.length() < 1)
		{
			throw std::invalid_argument("Invalid string");
		}

		// Loop through all characters
		while (remainder.length() > 0 && remainder.at(0) != '\"')
		{
			// Watch out for escaped end quotes
			if (remainder.at(0) == '\\')
			{
				if (remainder.length() < 2)
				{
					throw std::invalid_argument("Invalid string");
				}
				if (remainder.at(1) == '\"')
				{
					result.string += "\\\"";
					remainder.erase(0, 2);
					continue;
				}
			}

			// Move the character from the input to the value
			result.string += remainder.substr(0, 1);
			remainder.erase(0, 1);
		}

		// Check for valid end
		if (remainder.length() < 1 || remainder.at(0) != '\"')
		{
			throw std::invalid_argument("Invalid string");
		}

		// Remove the end quote
		remainder.erase(0, 1);
		return result;
		break;
	case json::jtype::jnumber:
		result.string = (std::string)(json::jnumber::parse(value, remainder));
		break;
	case json::jtype::jobject:
		result.string = (std::string)(json::jobject::parse(value, remainder));
		break;
	case json::jtype::jarray:
		result.string = (std::string)(json::jarray::parse(value, remainder));
		break;
	case json::jtype::jbool:
		if (value.length() >= 4 && value.substr(0, 4) == "true")
		{
			value.erase(0, 4);
			remainder = value;
			result.string = "true";
		}
		else if (value.length() >= 5 && value.substr(0, 4) == "false")
		{
			value.erase(0, 5);
			remainder = value;
			result.string = "false";
		}
		else
		{
			throw std::invalid_argument("Invalid input");
		}
		break;
	case json::jtype::jnull:
		if (value.length() >= 4 && value.substr(0, 4) == "null")
		{
			value.erase(0, 4);
			remainder = value;
			result.string = "null";
		}
		else
		{
			throw std::invalid_argument("Invalid input");
		}
		break;
	case json::jtype::not_valid:
		throw std::invalid_argument("Invalid input");
		break;
	default:
		throw std::invalid_argument("Invalid type");
		break;
	}
	return result;
}

#pragma endregion

std::string json::remove_leading_spaces(const std::string input)
{
	std::string output = input;
	while (output.length() > 0 && (output.at(0) == ' ' || output.at(0) == '\r' || output.at(0) == '\t' || output.at(0) == '\n'))
	{
		output.erase(0, 1);
	}
	return output;
}

#ifdef GNUC
#pragma GCC diagnostic pop
#endif
