#ifndef JSON_H
#define JSON_H

#include <string>
#include <vector>
#include <stdexcept>

namespace json
{
	/* Data types */
	namespace jtype
	{
		enum jtype { jstring, jnumber, jobject, jarray, jbool, jnull, not_valid };
		jtype detect(const std::string input);
	}

	class jnumber
	{
	public:
		// Parsers
		static jnumber parse(const std::string input);
		static jnumber parse(const std::string input, std::string& remainder);

		// Casters
		operator int() { return std::stoi(number); }
		operator long int() { return std::stol(number); }
		operator double() { return std::stod(number); }
		operator float() { return std::stof(number); }
		operator std::string() { return this->number; }
	private:
		static std::string read_digits(const std::string input);
		static std::string read_digits(const std::string input, std::string& remainder);
		jnumber(const std::string input);
		std::string number;
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
			for (size_t i = 0; i < this->size(); i++)
			{
				result.push_back(std::stoi(this->at(i)));
			}
			return result;
		}
		operator std::vector<long>()
		{
			std::vector<long> result;
			for (size_t i = 0; i < this->size(); i++)
			{
				result.push_back(std::stol(this->at(i)));
			}
			return result;
		}
		operator std::vector<double>()
		{
			std::vector<double> result;
			for (size_t i = 0; i < this->size(); i++)
			{
				result.push_back(std::stod(this->at(i)));
			}
			return result;
		}
		operator std::vector<float>()
		{
			std::vector<float> result;
			for (size_t i = 0; i < this->size(); i++)
			{
				result.push_back(std::stof(this->at(i)));
			}
			return result;
		}
		operator std::string() { return to_string(); }
		operator jtype::jtype() { return type; }
	private:
		jtype::jtype type;
		std::string to_string(void);
	};

	class jvalue
	{
	public:
		// Parsers
		static jvalue parse(const std::string input);
		static jvalue parse(const std::string input, std::string &remainder);

		// Constructors
		inline jvalue() { this->type = jtype::jnull; }
		inline jvalue(const int value) { this->string = std::to_string(value); this->type = jtype::jnumber; }
		inline jvalue(const long value) { this->string = std::to_string(value); this->type = jtype::jnumber; }
		inline jvalue(const double value) { this->string = std::to_string(value); this->type = jtype::jnumber; }
		inline jvalue(const float value) { this->string = std::to_string(value); this->type = jtype::jnumber; }
		inline jvalue(const bool value) { this->string = std::to_string(value); this->type = jtype::jbool; }
		inline jvalue(const std::string value) { this->string = value; this->type = jtype::jstring; }
		inline jvalue(jarray value) { this->string = (std::string)value; this->type = jtype::jarray; }

		// Casters
		operator jtype::jtype() { return this->type; }
		operator std::string() { return this->string; }
		operator int() { return std::stoi(this->string); }
		operator long() { return std::stol(this->string); }
		operator double() { return std::stod(this->string); }
		operator float() { return std::stof(this->string); }
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
			throw std::bad_cast();
		}
		operator jarray()
		{
			if (this->type == jtype::jarray)
			{
				return jarray::parse(this->string);
			}
			throw std::bad_cast();
		}
		inline jtype::jtype get_type(void) { return this->type; }
		inline bool is_null(void) { return this->type == jtype::jnull; }
		inline bool is_string(void) { return this->type == jtype::jstring; }
	private:
		std::string string;
		jtype::jtype type;
	};

	class key_value_pair
	{
	public:
		// Parsers
		static key_value_pair parse(const std::string input);
		static key_value_pair parse(const std::string input, std::string& remainder);

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

		// Casters
		operator std::string() { return to_string(); }

		// Methods
		std::vector<std::string> get_keys(void);
		bool has_key(const std::string key);
		key_value_pair get_entry(const std::string key);
	private:
		std::string to_string(void);
	};

	std::string remove_leading_spaces(const std::string input);
}

#endif // !JSON_H