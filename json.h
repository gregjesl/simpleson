#ifndef JSON_H
#define JSON_H

#include <string>
#include <vector>
#include <cstdio>
#include <utility>
#include <stdexcept>
#include <cctype>

#define NUMBER_TO_STRING_BUFFER_LENGTH 100

namespace json
{
	class invalid_key : public std::exception
	{
	public:
		const std::string key;
		inline invalid_key(const std::string &key) : key(key) { }
		inline virtual ~invalid_key() throw() { }
		virtual const char* what() const throw()
		{
			return key.c_str();
		}
	};

	class parsing_error : public std::invalid_argument
	{
	public:
		inline parsing_error(const char *message) : std::invalid_argument(message) { }
		inline virtual ~parsing_error() throw() { }
	};

	namespace parsing
	{
		void tlws(std::string &input);
	}

	/* Data types */
	namespace jtype
	{
		enum jtype { jstring, jnumber, jobject, jarray, jbool, jnull, not_valid };
		jtype detect(const std::string &input);
	}

	namespace parsing
	{
		std::string read_digits(std::string &input);
		std::string escape_quotes(const std::string &input);
		std::string unescape_quotes(const std::string &input);

		struct parse_results
		{
			jtype::jtype type;
			std::string value;
		};

		parse_results parse(std::string &value);
		
		template <typename T>
		T get_number(const std::string &number, const char* format)
		{
			T result;
			std::sscanf(number.c_str(), format, &result);
			return result;
		}

		template <typename T>
		std::string get_number_string(const T &number, const char *format)
		{
			char cstr[NUMBER_TO_STRING_BUFFER_LENGTH];
			std::sprintf(cstr, format, number);
			return std::string(cstr);
		}

		std::vector<std::string> parse_array(std::string value);
	}

	typedef std::pair<std::string, std::string> kvp;

	class jobject : protected std::vector<kvp>
	{
		class const_proxy
		{
		private:
			const jobject &source;
		protected:
			const std::string key;
			template<typename T>
			inline T get_number(const char* format) const
			{
				return json::parsing::get_number<T>(this->source.get(key), format);
			}
			template<typename T>
			inline std::vector<T> get_number_array(const char* format) const
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
		public:
			const_proxy(const jobject &source, const std::string key) : source(source), key(key) { }

			operator std::string() const 
			{
				std::string value = source.get(key);
				return json::parsing::unescape_quotes(json::parsing::parse(value).value);
			}

			bool operator== (const std::string other) const { return ((std::string)(*this)) == other; }
			bool operator!= (const std::string other) const { return !(((std::string)(*this)) == other); }

			// Numbers
			operator int() const { return this->get_number<int>("%i"); }
			operator unsigned int() const { return this->get_number<unsigned int>("%u"); }
			operator long() const { return this->get_number<long>("%li"); }
			operator unsigned long() const { return this->get_number<unsigned long>("%lu"); }
			operator char() const { return this->get_number<char>("%c"); }
			operator float() const { return this->get_number<float>("%f"); }
			operator double() const { return this->get_number<double>("%lf"); }

			// Objects
			inline operator json::jobject() const
			{
				std::string value = this->source.get(key);
				return json::jobject::parse(value);
			}

			// Arrays
			operator std::vector<int>() const { return this->get_number_array<int>("%i"); }
			operator std::vector<unsigned int>() const { return this->get_number_array<unsigned int>("%u"); }
			operator std::vector<long>() const { return this->get_number_array<long>("%li"); }
			operator std::vector<unsigned long>() const { return this->get_number_array<unsigned long>("%lu"); }
			operator std::vector<char>() const { return this->get_number_array<char>("%c"); }
			operator std::vector<float>() const { return this->get_number_array<float>("%f"); }
			operator std::vector<double>() const { return this->get_number_array<double>("%f"); }
			operator std::vector<json::jobject>() const
			{
				std::vector<std::string> objs = json::parsing::parse_array(this->source.get(key));
				std::vector<json::jobject> results;
				for (size_t i = 0; i < objs.size(); i++) results.push_back(json::jobject::parse(objs[i]));
				return results;
			}
			operator std::vector<std::string>() const { return json::parsing::parse_array(this->source.get(key)); }

			// Boolean
			inline bool is_true() const
			{
				std::string value = this->source.get(key);
				json::parsing::parse_results result = json::parsing::parse(value);
				return (result.type == json::jtype::jbool && result.value == "true");
			}

			// Null
			inline bool is_null() const
			{
				std::string value = this->source.get(key);
				json::parsing::parse_results result = json::parsing::parse(value);
				return result.type == json::jtype::jnull;
			}
		};

		class proxy : public json::jobject::const_proxy
		{
			jobject &sink;
		protected:
			template<typename T>
			inline void set_number(const T value, const char* format)
			{
				this->sink.set(key, json::parsing::get_number_string(value, format));
			}

			void set_array(const std::vector<std::string> &values, const bool wrap = false);

			template<typename T>
			inline void set_number_array(const std::vector<T> &values, const char* format)
			{
				std::vector<std::string> numbers;
				for (size_t i = 0; i < values.size(); i++)
				{
					numbers.push_back(json::parsing::get_number_string(values[i], format));
				}
				this->set_array(numbers);
			}
		public:
			proxy(jobject &source, const std::string key) 
				: json::jobject::const_proxy(source, key),
				sink(source)
			{ }

			// Strings
			inline void operator= (const std::string value)
			{
				this->sink.set(this->key, "\"" + json::parsing::escape_quotes(value) + "\"");
			}

			// Numbers
			void operator=(const int input) { this->set_number(input, "%i"); }
			void operator=(const unsigned int input) { this->set_number(input, "%u"); }
			void operator=(const long input) { this->set_number(input, "%li"); }
			void operator=(const unsigned long input) { this->set_number(input, "%lu"); }
			void operator=(const char input) { this->set_number(input, "%c"); }
			void operator=(const double input) { this->set_number(input, "%e"); }
			void operator=(const float input) { this->set_number(input, "%e"); }

			// Objects
			void operator=(json::jobject input)
			{
				this->sink.set(key, (std::string)input);
			}

			// Arrays
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
				if (value) this->sink.set(key, "true");
				else this->sink.set(key, "false");
			}

			// Null
			inline void set_null()
			{
				this->sink.set(key, "null");
			}

			inline void clear()
			{
				this->sink.remove(key);
			}
		};
	public:
		inline size_t size() const { return std::vector<kvp>::size(); }

		inline void clear() { this->resize(0); }

		jobject& operator+=(const kvp& other)
		{
			if (this->has_key(other.first)) throw json::parsing_error("Key conflict");
			this->push_back(other);
			return *this;
		}

		jobject& operator+=(jobject& other)
		{
			for (size_t i = 0; i < other.size(); i++) *this += other.at(i);
			return *this;
		}

		jobject& operator+=(const jobject& other)
		{
			json::jobject copy(other);
			for (size_t i = 0; i < copy.size(); i++) *this += copy.at(i);
			return *this;
		}

		jobject operator+(jobject& other)
		{
			jobject result = *this;
			result += other;
			return result;
		}

		static jobject parse(const std::string &input);

		inline static jobject parse(const char* input)
		{
			return parse(std::string(input));
		}

		// Returns true if a json parsing error occured
		inline bool static tryparse(const std::string &input, jobject &output)
		{
			try
			{
				output = parse(input);
			}
			catch(std::exception ex)
			{
				return true;
			}
			return false;
		}

		inline bool static tryparse(const char* input, jobject &output)
		{
			return tryparse(std::string(input), output);
		}

		inline bool has_key(const std::string &key) const
		{
			for (size_t i = 0; i < this->size(); i++) if (this->at(i).first == key) return true;
			return false;
		}

		void set(const std::string &key, const std::string &value);

		inline std::string get(const std::string &key) const
		{
			for (size_t i = 0; i < this->size(); i++) if (this->at(i).first == key) return this->at(i).second;
			throw json::invalid_key(key);
		}

		void remove(const std::string &key);

		inline virtual jobject::proxy operator[](const std::string key)
		{
			return jobject::proxy(*this, key);
		}

		inline virtual const jobject::const_proxy operator[](const std::string key) const
		{
			return jobject::const_proxy(*this, key);
		}

		operator std::string() const;
	};
}

#endif // !JSON_H