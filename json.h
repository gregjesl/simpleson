#ifndef JSON_H
#define JSON_H

#include <string>
#include <vector>
#include <cstdio>
#include <stdexcept>
#include <cctype>

#define NUMBER_TO_STRING_BUFFER_LENGTH 100

namespace json
{
	namespace parsing
	{
		inline void tlws(std::string &input)
		{
			while (input.size() > 0 && std::isspace(input[0])) input.erase(0, 1);
		}
	}

	/* Data types */
	namespace jtype
	{
		enum jtype { jstring, jnumber, jobject, jarray, jbool, jnull, not_valid };
		inline jtype detect(const std::string input)
		{
			std::string value = input;
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
	}

	namespace parsing
	{
		inline std::string read_digits(std::string &input)
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

		inline std::string escape_quotes(const std::string input)
		{
			std::string parsed;
			for (size_t i = 0; i < input.size(); i++)
			{
				if (input[i] == '\"' && parsed.back() != '\\')
				{
					parsed += '\\';
				}
				parsed += input[i];
			}
			return parsed;
		}

		inline std::string unescape_quotes(const std::string input)
		{
			std::string result;
			std::string value = input;
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

		struct parse_results
		{
			jtype::jtype type;
			std::string value;
		};
		inline parse_results parse(std::string &value)
		{
			// Strip white space
			json::parsing::tlws(value);

			// Validate input
			if (value.size() == 0) throw std::invalid_argument("Input was only whitespace");

			// Initialize the output
			json::parsing::parse_results result;

			// Detect the type
			result.type = json::jtype::detect(value);

			// Parse the values
			switch (result.type)
			{
			case json::jtype::jstring:
				// Validate the input
				if (value[0] != '"') throw std::invalid_argument("Expected '\"' as first character");

				// Remove the opening quote
				value.erase(0, 1);

				// Copy the string
				while (value.size() > 0)
				{
					if (value[0] != '"' || (result.value.size() > 0 && result.value.back() == '\\'))
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
				if (value.at(0) == '-')
				{
					result.value += "-";
					value.erase(0, 1);
				}

				if (value.length() < 1) throw std::invalid_argument("Input did not contain a valid number");

				// Read the whole digits
				std::string whole_digits = json::parsing::read_digits(value);

				// Validate the read
				if (whole_digits.length() == 0) throw std::invalid_argument("Input did not contain a valid number");

				// Tack on the value
				result.value += whole_digits;

				// Check for decimal number
				if (value[0] == '.')
				{
					result.value += ".";
					value.erase(0, 1);
					std::string decimal_digits = json::parsing::read_digits(value);

					if (decimal_digits.length() < 1) throw std::invalid_argument("Input did not contain a valid number");

					result.value += decimal_digits;
				}

				// Check for exponential number
				if (value[0] == 'e' || value[0] == 'E')
				{
					result.value += value[0];
					value.erase(0, 1);

					if (value.size() == 0) throw std::invalid_argument("Input did not contain a valid number");

					if (value[0] == '+' || value[0] == '-')
					{
						result.value += value[0];
						value.erase(0, 1);
					}

					if (value.size() == 0) throw std::invalid_argument("Input did not contain a valid number");

					std::string exponential_digits = json::parsing::read_digits(value);

					if (exponential_digits.size() == 0) throw std::invalid_argument("Input did not contain a valid number");

					result.value += exponential_digits;
				}
				break;
			}
			case json::jtype::jobject:
			{
				// The first character should be an open bracket
				if (value[0] != '{') throw std::invalid_argument("Input did not contain a valid object");
				result.value += '{';
				value.erase(0, 1);
				json::parsing::tlws(value);

				// Loop until the closing bracket is encountered
				while (value.size() > 0 && value[0] != '}')
				{
					// Read the key
					json::parsing::parse_results key = json::parsing::parse(value);

					// Validate that the key is a string
					if (key.type != json::jtype::jstring) throw std::invalid_argument("Input did not contain a valid object");

					// Store the key
					result.value += "\"" + json::parsing::escape_quotes(key.value) + "\"";
					json::parsing::tlws(value);

					// Look for the colon
					if (value[0] != ':') throw std::invalid_argument("Input did not contain a valid object");
					result.value += ':';
					value.erase(0, 1);

					// Get the value
					json::parsing::parse_results subvalue = json::parsing::parse(value);

					// Validate the value type
					if (subvalue.type == json::jtype::not_valid) throw std::invalid_argument("Input did not contain a valid object");

					// Store the value
					if (subvalue.type == json::jtype::jstring) result.value += "\"" + json::parsing::escape_quotes(subvalue.value) + "\"";
					else result.value += subvalue.value;
					json::parsing::tlws(value);

					// Validate format
					if (value[0] != ',' && value[0] != '}') throw std::invalid_argument("Input did not contain a valid object");

					// Check for next line
					if (value[0] == ',')
					{
						result.value += ',';
						value.erase(0, 1);
					}
				}
				if (value.size() == 0 || value[0] != '}') throw std::invalid_argument("Input did not contain a valid object");
				result.value += '}';
				value.erase(0, 1);
				break;
			}
			case json::jtype::jarray:
			{
				if (value[0] != '[') throw std::invalid_argument("Input did not contain a valid array");
				result.value += '[';
				value.erase(0, 1);
				json::parsing::tlws(value);
				if (value.size() == 0) throw std::invalid_argument("Input did not contain a valid array");
				while (value.size() > 0 && value[0] != ']')
				{
					json::parsing::parse_results array_value = json::parsing::parse(value);
					if (array_value.type == json::jtype::not_valid) throw std::invalid_argument("Input did not contain a valid array");
					result.value += array_value.value;
					json::parsing::tlws(value);
					if (value[0] != ',' && value[0] != ']') throw std::invalid_argument("Input did not contain a valid array");
					if (value[0] == ',')
					{
						result.value += ',';
						value.erase(0, 1);
					}
				}
				if (value.size() == 0 || value[0] != ']') throw std::invalid_argument("Input did not contain a valid array");
				result.value += ']';
				value.erase(0, 1);
				break;
			}
			case json::jtype::jbool:
			{
				if (value.size() < 4) throw std::invalid_argument("Input did not contain a valid boolean");
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
					throw std::invalid_argument("Input did not contain a valid boolean");
				}
				break;
			}
			case json::jtype::jnull:
			{
				if (value.size() < 4) throw std::invalid_argument("Input did not contain a valid null");
				if (value.substr(0, 4) == "null")
				{
					result.value += "null";
					value.erase(0, 4);
				}
				else
				{
					throw std::invalid_argument("Input did not contain a valid null");
				}
				break;
			}
			default:
				throw std::invalid_argument("Input did not contain valid json");
				break;
			}

			return result;
		}

		template <typename T>
		T get_number(const std::string number, const char* format)
		{
			T result;
			std::sscanf(number.c_str(), format, &result);
			return result;
		}

		template <typename T>
		std::string get_number_string(const T number, const char *format)
		{
			char cstr[NUMBER_TO_STRING_BUFFER_LENGTH];
			std::sprintf(cstr, format, number);
			return std::string(cstr);
		}

		inline std::vector<std::string> parse_array(std::string value)
		{
			json::parsing::tlws(value);
			if (value[0] != '[') throw std::invalid_argument("Input was not an array");
			value.erase(0, 1);
			std::vector<std::string> result;
			while (value.size() > 0)
			{
				json::parsing::tlws(value);
				json::parsing::parse_results parse_results = json::parsing::parse(value);
				if (parse_results.type == json::jtype::not_valid) throw std::invalid_argument("Input was not properly formated");
				result.push_back(parse_results.value);
				json::parsing::tlws(value);
				if (value[0] == ']') break;
				if (value[0] == ',') value.erase(0, 1);
			}
			if (value[0] != ']') throw std::invalid_argument("Input was not properly formated");
			value.erase(0, 1);
			return result;
		}
	}

	class jobject : protected std::vector<std::vector<std::string> >
	{
		class proxy
		{
			jobject &source;
			std::string key;
		protected:
			template<typename T>
			inline T get_number(const char* format)
			{
				return json::parsing::get_number<T>(this->source.get(key), format);
			}
			template<typename T>
			inline void set_number(const T value, const char* format)
			{
				this->source.set(key, json::parsing::get_number_string(value, format));
			}

			inline void set_array(const std::vector<std::string> values, const bool wrap = false)
			{
				std::string value = "[";
				for (size_t i = 0; i < values.size(); i++)
				{
					if (wrap) value += "\"" + json::parsing::escape_quotes(values[i]) + "\"";
					else value += values[i] + ",";
				}
				value.pop_back();
				value += "]";
				this->source.set(key, value);
			}

			template<typename T>
			inline std::vector<T> get_number_array(const char* format)
			{
				std::string value = this->source.get(key);
				std::vector<std::string> numbers = json::parsing::parse_array(value);
				std::vector<T> result;
				for (size_t i = 0; i < numbers.size(); i++)
				{
					result.push_back(json::parsing::get_number<T>(numbers[i], format));
				}
				return result;
			}
			template<typename T>
			inline void set_number_array(const std::vector<T> values, const char* format)
			{
				std::vector<std::string> numbers;
				for (size_t i = 0; i < values.size(); i++)
				{
					numbers.push_back(json::parsing::get_number_string(values[i], format));
				}
				this->set_array(numbers);
			}
		public:
			proxy(jobject &source, std::string key) : source(source), key(key) {}
			// Strings
			inline void operator= (const std::string value)
			{
				this->source.set(this->key, "\"" + json::parsing::escape_quotes(value) + "\"");
			}
			operator std::string() {
				return json::parsing::unescape_quotes(json::parsing::parse(source.get(key)).value);
			}
			bool operator== (const std::string other) { return ((std::string)(*this)) == other; }
			bool operator!= (const std::string other) { return !(((std::string)(*this)) == other); }

			// Numbers
			operator int() { return this->get_number<int>("%i"); }
			operator unsigned int() { return this->get_number<unsigned int>("%u"); }
			operator long() { return this->get_number<long>("%li"); }
			operator unsigned long() { return this->get_number<unsigned long>("%lu"); }
			operator char() { return this->get_number<char>("%c"); }
			operator float() { return this->get_number<float>("%f"); }
			operator double() { return this->get_number<double>("%lf"); }
			void operator=(const int input) { this->set_number(input, "%i"); }
			void operator=(const unsigned int input) { this->set_number(input, "%u"); }
			void operator=(const long input) { this->set_number(input, "%li"); }
			void operator=(const unsigned long input) { this->set_number(input, "%lu"); }
			void operator=(const char input) { this->set_number(input, "%c"); }
			void operator=(const double input) { this->set_number(input, "%e"); }
			void operator=(const float input) { this->set_number(input, "%e"); }

			// Objects
			inline operator json::jobject()
			{
				return json::jobject::parse(this->source.get(key));
			}
			void operator=(json::jobject input)
			{
				this->source.set(key, (std::string)input);
			}

			// Arrays
			operator std::vector<int>() { return this->get_number_array<int>("%i"); }
			operator std::vector<unsigned int>() { return this->get_number_array<unsigned int>("%u"); }
			operator std::vector<long>() { return this->get_number_array<long>("%li"); }
			operator std::vector<unsigned long>() { return this->get_number_array<unsigned long>("%lu"); }
			operator std::vector<char>() { return this->get_number_array<char>("%c"); }
			operator std::vector<float>() { return this->get_number_array<float>("%f"); }
			operator std::vector<double>() { return this->get_number_array<double>("%f"); }
			operator std::vector<json::jobject>()
			{
				std::vector<std::string> objs = json::parsing::parse_array(this->source.get(key));
				std::vector<json::jobject> results;
				for (size_t i = 0; i < objs.size(); i++) results.push_back(json::jobject::parse(objs[i]));
				return results;
			}
			operator std::vector<std::string>() { return json::parsing::parse_array(this->source.get(key)); }
			void operator=(const std::vector<int> input) { this->set_number_array(input, "%i"); }
			void operator=(const std::vector<unsigned int> input) { this->set_number_array(input, "%u"); }
			void operator=(const std::vector<long> input) { this->set_number_array(input, "%li"); }
			void operator=(const std::vector<unsigned long> input) { this->set_number_array(input, "%lu"); }
			void operator=(const std::vector<char> input) { this->set_number_array(input, "%c"); }
			void operator=(const std::vector<float> input) { this->set_number_array(input, "%e"); }
			void operator=(const std::vector<double> input) { this->set_number_array(input, "%e"); }
			void operator=(const std::vector<std::string> input) { this->set_array(input, true); }
			void operator=(std::vector<json::jobject> input)
			{
				std::vector<std::string> objs;
				for (size_t i = 0; i < input.size(); i++)
				{
					objs.push_back((std::string)input[i]);
				}
				this->set_array(objs, false);
			}

			// Boolean
			inline void set_boolean(const bool value)
			{
				if (value) this->source.set(key, "true");
				else this->source.set(key, "false");
			}
			inline bool is_true()
			{
				json::parsing::parse_results result = json::parsing::parse(this->source.get(key));
				return (result.type == json::jtype::jbool && result.value == "true");
			}

			// Null
			inline void set_null()
			{
				this->source.set(key, "null");
			}
			inline bool is_null()
			{
				json::parsing::parse_results result = json::parsing::parse(this->source.get(key));
				return result.type == json::jtype::jnull;
			}

		};
	public:
		inline static jobject parse(std::string &input)
		{
			json::parsing::tlws(input);
			if (input[0] != '{') throw std::invalid_argument("Input is not a valid object");
			input.erase(0, 1);
			json::parsing::tlws(input);
			if (input.size() == 0) throw std::invalid_argument("Input is not a valid object");
			json::jobject result;
			while (input.size() > 0 && input[0] != '}')
			{
				// Get key
				std::vector<std::string> kvp(2);
				json::parsing::parse_results key = json::parsing::parse(input);
				if (key.type != json::jtype::jstring || key.value == "") throw std::invalid_argument("Input is not a valid object");
				kvp[0] = key.value;

				// Get value
				json::parsing::tlws(input);
				if (input[0] != ':') throw std::invalid_argument("Input is not a valid object");
				input.erase(0, 1);
				json::parsing::tlws(input);
				json::parsing::parse_results value = json::parsing::parse(input);
				if (value.type == json::jtype::not_valid) throw std::invalid_argument("Input is not a valid object");
				if (value.type == json::jtype::jstring) kvp[1] = "\"" + value.value + "\"";
				else kvp[1] = value.value;

				// Clean up
				json::parsing::tlws(input);
				if (input[0] != ',' && input[0] != '}') throw std::invalid_argument("Input is not a valid object");
				if (input[0] == ',') input.erase(0, 1);
				result.push_back(kvp);

			}
			if (input.size() == 0 || input[0] != '}') throw std::invalid_argument("Input did not contain a valid object");
			input.erase(0, 1);
			return result;
		}
		inline bool has_key(const std::string key)
		{
			for (size_t i = 0; i < this->size(); i++) if (this->at(i).at(0) == key) return true;
			return false;
		}
		inline void set(const std::string key, const std::string value)
		{
			for (size_t i = 0; i < this->size(); i++)
			{
				if (this->at(i).at(0) == key)
				{
					this->at(i)[1] = value;
					return;
				}
			}
			std::vector<std::string> kvp(2);
			kvp[0] = key;
			kvp[1] = value;
			this->push_back(kvp);
		}

		inline std::string get(const std::string key)
		{
			for (size_t i = 0; i < this->size(); i++) if (this->at(i).at(0) == key) return this->at(i).at(1);
			throw std::invalid_argument("Key not found");
		}

		inline virtual jobject::proxy operator[](const std::string key)
		{
			return jobject::proxy(*this, key);
		}

		operator std::string()
		{
			if (this->size() == 0) return "{}";
			std::string result = "{";
			for (size_t i = 0; i < this->size(); i++)
			{
				result += "\"" + this->at(i)[0] + "\":" + this->at(i)[1] + ",";
			}
			result.pop_back();
			result += "}";
			return result;
		}
	};
}

#endif // !JSON_H