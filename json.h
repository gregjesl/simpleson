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
		const char* tlws(const char *start);
	}

	/* Data types */
	namespace jtype
	{
		enum jtype { jstring, jnumber, jobject, jarray, jbool, jnull, not_valid };
		jtype detect(const char *input);
	}

	namespace parsing
	{
		std::string read_digits(const char *input);
		std::string escape_quotes(const char *input);
		std::string unescape_quotes(const char *input);

		struct parse_results
		{
			jtype::jtype type;
			std::string value;
			const char *remainder;
		};

		parse_results parse(const char *input);
		
		template <typename T>
		T get_number(const char *input, const char* format)
		{
			T result;
			std::sscanf(input, format, &result);
			return result;
		}

		template <typename T>
		std::string get_number_string(const T &number, const char *format)
		{
			char cstr[NUMBER_TO_STRING_BUFFER_LENGTH];
			std::sprintf(cstr, format, number);
			return std::string(cstr);
		}

		std::vector<std::string> parse_array(const char *input);
	}

	typedef std::pair<std::string, std::string> kvp;

	class jobject
	{
	private:
		std::vector<kvp> data;
		bool array_flag;

		class entry
		{
		protected:
			virtual const std::string& ref() const = 0;

			template<typename T>
			inline T get_number(const char* format) const
			{
				return json::parsing::get_number<T>(this->ref().c_str(), format);
			}

			template<typename T>
			inline std::vector<T> get_number_array(const char* format) const
			{
				std::vector<std::string> numbers = json::parsing::parse_array(this->ref().c_str());
				std::vector<T> result;
				for (size_t i = 0; i < numbers.size(); i++)
				{
					result.push_back(json::parsing::get_number<T>(numbers[i].c_str(), format));
				}
				return result;
			}

		public:
			inline std::string as_string() const
			{
				return json::parsing::unescape_quotes(
					json::parsing::parse(this->ref().c_str()).value.c_str()
					);
			}

			inline operator std::string() const 
			{
				return this->as_string();
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
			inline json::jobject as_object() const
			{
				return json::jobject::parse(this->ref().c_str());
			}

			inline operator json::jobject() const
			{
				return this->as_object();
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
				const std::vector<std::string> objs = json::parsing::parse_array(this->ref().c_str());
				std::vector<json::jobject> results;
				for (size_t i = 0; i < objs.size(); i++) results.push_back(json::jobject::parse(objs[i].c_str()));
				return results;
			}
			operator std::vector<std::string>() const { return json::parsing::parse_array(this->ref().c_str()); }

			template<typename T>
			inline std::vector<T> as_array() const
			{
				return (std::vector<T>)(*this);
			}

			// Boolean
			inline bool is_true() const
			{
				json::parsing::parse_results result = json::parsing::parse(this->ref().c_str());
				return (result.type == json::jtype::jbool && result.value == "true");
			}

			// Null
			inline bool is_null() const
			{
				return json::parsing::parse(this->ref().c_str()).type == json::jtype::jnull;
			}
		};

		class const_value : public entry
		{
		private:
			std::string data;

		protected:
			inline const std::string& ref() const 
			{
				return this->data;
			}
		
		public:
			inline const_value(std::string value)
			: data(value)
			{ }

			inline const_value get(const std::string &key) const
			{
				return const_value(json::jobject::parse(this->data).get(key));
			}

			inline const_value array(const size_t index) const
			{
				return const_value(json::jobject::parse(this->data).get(index));
			}
		};

		class const_proxy : public entry
		{
		private:
			const jobject &source;

		protected:
			const std::string key;

			inline const std::string& ref() const 
			{
				for (size_t i = 0; i < this->source.size(); i++) if (this->source.data.at(i).first == key) return this->source.data.at(i).second;
				throw json::invalid_key(key);
			}

		public:
			const_proxy(const jobject &source, const std::string key) : source(source), key(key) { }

			const_value array(size_t index) const
			{
				const char *value = this->ref().c_str();
				if(json::jtype::detect(value) != json::jtype::jarray)
					throw std::invalid_argument("Input is not an array");
				const std::vector<std::string> values = json::parsing::parse_array(value);
				return const_value(values[index]);
			}
		};

		class proxy : public json::jobject::const_proxy
		{
		private:
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
				this->sink.set(this->key, "\"" + json::parsing::escape_quotes(value.c_str()) + "\"");
			}

			inline void operator= (const char* value)
			{
				this->operator=(std::string(value));
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
		inline jobject(bool array = false)
			: array_flag(array) 
			{ }

		inline jobject(const jobject &other)
			: data(other.data),
			array_flag(other.array_flag)
		{ }

		inline virtual ~jobject() { }

		bool is_array() const { return this->array_flag; }

		inline size_t size() const { return this->data.size(); }

		inline void clear() { this->data.resize(0); }

		bool operator== (const json::jobject other) const { return ((std::string)(*this)) == (std::string)other; }
		bool operator!= (const json::jobject other) const { return ((std::string)(*this)) != (std::string)other; }

		inline jobject& operator=(const jobject rhs)
		{
			this->array_flag = rhs.array_flag;
			this->data = rhs.data;
			return *this;
		}

		jobject& operator+=(const kvp& other)
		{
			if (this->has_key(other.first)) throw json::parsing_error("Key conflict");
			this->data.push_back(other);
			return *this;
		}

		jobject& operator+=(jobject& other)
		{
			for (size_t i = 0; i < other.size(); i++) this->data.push_back(other.data.at(i));
			return *this;
		}

		jobject& operator+=(const jobject& other)
		{
			json::jobject copy(other);
			for (size_t i = 0; i < copy.size(); i++) this->data.push_back(other.data.at(i));
			return *this;
		}

		jobject operator+(jobject& other)
		{
			jobject result = *this;
			result += other;
			return result;
		}

		static jobject parse(const char *input);
		static inline jobject parse(const std::string input) { return parse(input.c_str()); }

		// Returns true if a json parsing error occured
		inline bool static tryparse(const char *input, jobject &output)
		{
			try
			{
				output = parse(input);
			}
			catch(...)
			{
				return true;
			}
			return false;
		}

		inline bool has_key(const std::string &key) const
		{
			if(this->array_flag) return false;
			for (size_t i = 0; i < this->size(); i++) if (this->data.at(i).first == key) return true;
			return false;
		}

		void set(const std::string &key, const std::string &value);

		inline std::string get(const size_t index) const
		{
			return this->data.at(index).second;
		}

		inline std::string get(const std::string &key) const
		{
			for (size_t i = 0; i < this->size(); i++) if (this->data.at(i).first == key) return this->get(i);
			throw json::invalid_key(key);
		}

		void remove(const std::string &key);
		void remove(const size_t index)
		{
			this->data.erase(this->data.begin() + index);
		}

		inline virtual jobject::proxy operator[](const std::string key)
		{
			if(this->array_flag) throw json::invalid_key(key);
			return jobject::proxy(*this, key);
		}

		inline virtual const jobject::const_proxy operator[](const std::string key) const
		{
			if(this->array_flag) throw json::invalid_key(key);
			return jobject::const_proxy(*this, key);
		}

		inline const jobject::const_value array(const size_t index) const
		{
			return jobject::const_value(this->data.at(index).second);
		}

		operator std::string() const;

		inline std::string as_string() const
		{
			return this->operator std::string();
		}
	};
}

#endif // !JSON_H