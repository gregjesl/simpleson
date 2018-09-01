#ifndef JSON_H
#define JSON_H

#include <string>
#include <vector>
#include <cstdio>
#include <stdexcept>

#define NUMBER_TO_STRING_BUFFER_LENGTH 100

namespace json
{
	namespace parsing
	{
		void tlws(std::string &input);
	}

	/* Data types */
	namespace jtype
	{
		enum jtype { jstring, jnumber, jobject, jarray, jbool, jnull, not_valid };
		jtype detect(const std::string input);
	}

	namespace parsing
	{
		std::string read_digits(std::string &input);

		struct parse_results
		{
			jtype::jtype type;
			std::string value;
		};
		parse_results parse(std::string &input);

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
			if(value[0] != ']') throw std::invalid_argument("Input was not properly formated");
			value.erase(0, 1);
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
	}

	class jdictionary : public std::vector<std::vector<std::string> >
	{
	public:
		static jdictionary parse(std::string &input);
		inline virtual bool has_key(const std::string key)
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
	};

	class jobject : protected jdictionary
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
				json::jobject result;
				json::jdictionary dict = json::jdictionary::parse(this->source.get(key));
				for (size_t i = 0; i < dict.size(); i++)
				{
					result.push_back(dict[i]);
				}
				return result;
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
				for (size_t i = 0; i < objs.size(); i++)
				{
					json::jobject result;
					json::jdictionary dict = json::jdictionary::parse(this->source.get(key));
					for (size_t i = 0; i < dict.size(); i++)
					{
						result.push_back(dict[i]);
					}
					results.push_back(result);
				}
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
	/*
	namespace basic
	{
		class istringvalue
		{
		protected:
			std::string string;
		};

		class istringnumber : public istringvalue
		{
		public:
			inline istringnumber() { }
			inline istringnumber(const int value) { *this = value; }
			inline istringnumber(const unsigned int value) { *this = value; }
			inline istringnumber(const long value) { *this = value; }
			inline istringnumber(const unsigned long value) { *this = value; }
			inline istringnumber(const char value) { *this = value; }
			inline istringnumber(const double value) { *this = value; }
			inline istringnumber(const float value) { *this = value; }

			// To basic type
			operator int() { int result; std::sscanf(string.c_str(), "%i", &result); return result; }
			operator unsigned int() { unsigned int result; std::sscanf(string.c_str(), "%u", &result); return result; }
			operator long() { long result; std::sscanf(string.c_str(), "%li", &result); return result; }
			operator unsigned long() { unsigned long result; std::sscanf(string.c_str(), "%lu", &result); return result; }
			operator char() { char result; std::sscanf(string.c_str(), "%c", &result); return result; }
			operator double() { double result; std::sscanf(string.c_str(), "%lf", &result); return result; }
			operator float() { float result; std::sscanf(string.c_str(), "%f", &result); return result; }
			operator std::string() { return this->string; }

			// From basic type
			void operator=(const int input) { char cstr[NUMBER_TO_STRING_BUFFER_LENGTH]; std::sprintf(cstr, "%i", input); string = std::string(cstr); }
			void operator=(const unsigned int input) { char cstr[NUMBER_TO_STRING_BUFFER_LENGTH]; std::sprintf(cstr, "%u", input); string = std::string(cstr); }
			void operator=(const long input) { char cstr[NUMBER_TO_STRING_BUFFER_LENGTH]; std::sprintf(cstr, "%li", input); string = std::string(cstr); }
			void operator=(const unsigned long input) { char cstr[NUMBER_TO_STRING_BUFFER_LENGTH]; std::sprintf(cstr, "%lu", input); string = std::string(cstr); }
			void operator=(const char input) { char cstr[NUMBER_TO_STRING_BUFFER_LENGTH]; std::sprintf(cstr, "%c", input); string = std::string(cstr); }
			void operator=(const double input) { char cstr[NUMBER_TO_STRING_BUFFER_LENGTH]; std::sprintf(cstr, "%lf", input); string = std::string(cstr); }
			void operator=(const float input) { char cstr[NUMBER_TO_STRING_BUFFER_LENGTH]; std::sprintf(cstr, "%f", input); string = std::string(cstr); }
		};
	}
	*/
	/*
	class jnumber : public basic::istringnumber
	{
	public:
		// Parsers
		static jnumber parse(const std::string input);
		static jnumber parse(const std::string input, std::string& remainder);
	private:
		static std::string read_digits(const std::string input);
		static std::string read_digits(const std::string input, std::string& remainder);
		jnumber(const std::string input);
	};

	class jarray : public std::vector<std::string>
	{
	public:
		// Parsers
		static jarray parse(const std::string input);
		static jarray parse(const std::string input, std::string& remainder);

		// Casters
		operator std::vector<int>()
		{
			std::vector<int> result;
			int value;

			for (size_t i = 0; i < this->size(); i++)
			{
				std::sscanf(this->at(i).c_str(), "%i", &value);
				result.push_back(value);
			}
			return result;
		}
		operator std::vector<unsigned int>()
		{
			std::vector<unsigned int> result;
			unsigned int value;

			for (size_t i = 0; i < this->size(); i++)
			{
				std::sscanf(this->at(i).c_str(), "%u", &value);
				result.push_back(value);
			}
			return result;
		}
		operator std::vector<long>()
		{
			std::vector<long> result;
			long value;

			for (size_t i = 0; i < this->size(); i++)
			{
				std::sscanf(this->at(i).c_str(), "%li", &value);
				result.push_back(value);
			}
			return result;
		}
		operator std::vector<unsigned long>()
		{
			std::vector<unsigned long> result;
			unsigned long value;

			for (size_t i = 0; i < this->size(); i++)
			{
				std::sscanf(this->at(i).c_str(), "%lu", &value);
				result.push_back(value);
			}
			return result;
		}
		operator std::vector<char>()
		{
			std::vector<char> result;
			char value;

			for (size_t i = 0; i < this->size(); i++)
			{
				std::sscanf(this->at(i).c_str(), "%c", &value);
				result.push_back(value);
			}
			return result;
		}
		operator std::vector<double>()
		{
			std::vector<double> result;
			double value;

			for (size_t i = 0; i < this->size(); i++)
			{
				std::sscanf(this->at(i).c_str(), "%lf", &value);
				result.push_back(value);
			}
			return result;
		}
		operator std::vector<float>()
		{
			std::vector<float> result;
			float value;

			for (size_t i = 0; i < this->size(); i++)
			{
				std::sscanf(this->at(i).c_str(), "%f",&value);
				result.push_back(value);
			}
			return result;
		}
		operator std::string() { return to_string(); }
		operator jtype::jtype() { return type; }
	private:
		jtype::jtype type;
		std::string to_string(void);
	};
	*/
	/*
	class jvalue : public basic::istringnumber
	{
	public:
		// Parsers
		static jvalue parse(const std::string input);
		static jvalue parse(const std::string input, std::string &remainder);

		static jvalue jbool(const bool input);

		// Constructors
		inline jvalue() { this->type = jtype::jnull; }
		inline jvalue(const std::string value) { this->string = value; this->type = jtype::jstring; }
		inline jvalue(jarray value) { this->string = (std::string)value; this->type = jtype::jarray; }

		inline jvalue(const int value) : istringnumber(value) { type = jtype::jnumber; }
		inline jvalue(const unsigned int value) : istringnumber(value) { type = jtype::jnumber; }
		inline jvalue(const long value) : istringnumber(value) { type = jtype::jnumber; }
		inline jvalue(const unsigned long value) : istringnumber(value) { type = jtype::jnumber; }
		inline jvalue(const char value) : istringnumber(value) { type = jtype::jnumber; }
		inline jvalue(const float value) : istringnumber(value) { type = jtype::jnumber; }
		inline jvalue(const double value) : istringnumber(value) { type = jtype::jnumber; }

		// Operators
		void operator=(const std::string input) { string = input; type = jtype::jstring; }
		void operator=(jarray input) { string = (std::string)input; type = jtype::jarray; }
		// void operator=(bool input) { if (input) { string = "true"; } else { string = "false"; } type = jtype::jbool; }

		// Casters
		operator jtype::jtype() { return this->type; }
		operator bool()
		{
			if (this->type == jtype::jbool)
			{
				return this->string == "true";
			}
			if (this->type == jtype::jnumber)
			{
				if (this->string == "1")
				{
					return true;
				}
				if (this->string == "0")
				{
					return false;
				}
			}
			throw std::runtime_error("Value is not a bool");
		}
		operator jarray()
		{
			if (this->type == jtype::jarray)
			{
				return jarray::parse(this->string);
			}
			throw std::invalid_argument("RHS was not an array");
		}
		operator std::vector<int>() { return (jarray)*this; }
		operator std::vector<unsigned int>() { return (jarray)*this; }
		operator std::vector<long>() { return (jarray)*this; }
		operator std::vector<unsigned long>() { return (jarray)*this; }
		operator std::vector<char>() { return (jarray)*this; }
		operator std::vector<double>() { return (jarray)*this; }
		operator std::vector<float>() { return (jarray)*this; }

		inline jtype::jtype get_type(void) { return this->type; }
		inline bool is_null(void) { return this->type == jtype::jnull; }
		inline bool is_string(void) { return this->type == jtype::jstring; }
	private:
		jtype::jtype type;
	};

	class key_value_pair
	{
	public:
		// Parsers
		static key_value_pair parse(const std::string input);
		static key_value_pair parse(const std::string input, std::string& remainder);

		// Constructors
		inline key_value_pair() { }
		inline key_value_pair(const std::string key) { this->key = key; this->value.parse("null"); }
		inline key_value_pair(const std::string key, const jvalue value) { this->key = key; this->value = value; }

		// Operators
		operator std::string() { return to_string(); }

		// Properties
		std::string key;
		jvalue value;
	private:
		std::string to_string(void);
	};

	class jobject : public std::vector<key_value_pair>
	{
	public:
		// Parsers
		static jobject parse(const std::string input);
		static jobject parse(const std::string input, std::string& remainder);

		// Constructors
		inline jobject() { }
		inline jobject(const std::string str) { *this = jobject::parse(str); }

		// Casters
		operator std::string() { return to_string(); }
		operator jvalue() { return jvalue::parse(this->to_string()); }
		void operator=(std::string value) { *this = jobject::parse((std::string)value); }

		// Methods
		std::vector<std::string> get_keys(void);
		bool has_key(const std::string key);
		key_value_pair get_entry(const std::string key);
		void remove_entry(const std::string key);
	private:
		std::string to_string(void);
	};

	std::string remove_leading_spaces(const std::string input);
	*/
}

#endif // !JSON_H