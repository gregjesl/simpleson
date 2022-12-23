#ifndef JSON_H
#define JSON_H

/*! \file json.h
 * \brief Simpleson header file
 */

#include <cstdlib>
#include <string>
#include <vector>
#include <map>
#include <list>
#include <cstdio>
#include <utility>
#include <typeinfo>
#include <stdexcept>
#include <cctype>
#include <stdint.h>

/*! \brief Base namespace for simpleson */
namespace json
{
	// Forward declaration
	class jobject;

	// Forward declaration
	class jarray;

	/*! \brief Exception used for invalid JSON keys */
	class invalid_key : public std::runtime_error
	{
	public:
		/*! \brief Constructor
		 *
		 * @param key The key that was not valid
		 */
		inline invalid_key(const std::string &key) : std::runtime_error(key) { }
	};

	/*! \brief Exception used when invalid JSON is encountered */
	class parsing_error : public std::invalid_argument
	{
	public:
		/*! \brief Constructor 
		 *
		 * @param message Details regarding the parsing error
		 */
		inline parsing_error(const char *message) : std::invalid_argument(message) { }

		/*! \brief Destructor */
		inline virtual ~parsing_error() throw() { }
	};

	/*\brief Alias for a list of keys */
	typedef std::vector<std::string> key_list_t;

	/* \brief Namespace for handling of JSON data types */
	namespace jtype
	{
		/*! \brief Descriptor for the type of JSON data */
		enum jtype { 
			jstring, ///< String value
			jnumber, ///< Number value
			jobject, ///< JSON object
			jarray, ///< JSON array
			jbool, ///< Boolean value
			jnull, ///< Null value
			not_valid ///< Value does not conform to JSON standard
		};

		jtype peek(const char input);

		/*! \brief Geven a string, determines the type of value the string contains
		 * 
		 * @param input The string to be tested
		 * @return The type of JSON value encountered
		 * 
		 * \note The function will only determine the type of the first value encountered in the string. 
		 */
		jtype detect(const char *input);
	}

	/*! \brief Interface for data containers */
	class data_container
	{
	public:
		virtual jtype::jtype type() const = 0;
		virtual std::string serialize() const = 0;
	};

	/*! \brief Value reader */
	class reader : public data_container, protected std::string
	{
	public:
		enum push_result
		{
			ACCEPTED, ///< The character was valid. Reading should continue. 
			REJECTED, ///< The character was not valid. Reading should stop.
			WHITESPACE ///< The character was whitespace. Reading should continue but the whtiespace was not stored. 
		};

		/*! \brief Reader constructor */
		inline reader() : std::string(), sub_reader(NULL) { this->clear(); }

		static reader parse(const std::string input)
		{
			reader result;
			for(size_t i = 0; i < input.size(); i++) {
				result.push(input[i]);
			}
			return result;
		}

		/*! \brief Resets the reader */
		virtual void clear();

		/*! \brief Length field exposed */
		using std::string::length;

		#if __GNUC__ && __GNUC__ < 11
		inline char front() const { return this->at(0); }
		inline char back() const { return this->at(this->length() - 1); }
		#else
		/*! \brief Front method exposed */
		using std::string::front;

		/*! \brief Back method exposed */
		using std::string::back;
		#endif

		/*!\ brief Pushes a value to the back of the reader 
		*
		* @param next the value to be pushed
		* \returns `ACCEPTED` if the value was added to the reader, `WHITESPACE` if the input was whitespace that was not stored, and `REJECTED` is the input was invalid for the value type
		*/
		virtual push_result push(const char next);

		/*!\brief Checks the value
		*
		* \returns The type of value stored in the reader, or `not_valid` if no value is stored
		*/
		inline virtual jtype::jtype type() const
		{
			return this->length() > 0 ? jtype::peek(this->front()) : json::jtype::not_valid;
		}

		/*! \brief Checks if the stored value is valid 
		* 
		* \returns `true` if the stored value is valid, `false` otherwise 
		*/
		virtual bool is_valid() const;

		/*! \brief Returns the stored value 
		*
		* \returns A string containing the stored value
		* \warning This method will return the value regardless of the state of the value, valid or not
		*/
		inline virtual std::string serialize() const { return *this; }

		/*! \brief Destructor */
		inline virtual ~reader() { this->clear(); }

	protected:
		/*! \brief The subreader used during reading
		* 
		* Arrays and objects will use a sub reader to store underlying values
		*/
		reader *sub_reader;

		/*! \brief Pushes a character to a string value */
		push_result push_string(const char next);

		/*! \brief Pushes a character to an array value */
		push_result push_array(const char next);

		/*! \brief Pushes a character to an object value */
		push_result push_object(const char next);

		/*! \brief Pushes a character to a number value */
		push_result push_number(const char next);

		/*! \brief Pushes a character to a boolean value */
		push_result push_boolean(const char next);

		/*! \brief Pushes a character to a null value */
		push_result push_null(const char next);

		/*! \brief Returns the stored state 
		* 
		* This template is intended for use with #string_reader_enum, #number_reader_enum, #array_reader_enum, and #object_reader_enum
		*/
		template<typename T>
		T get_state() const
		{
			return static_cast<T>(this->read_state);
		}

		/*! \brief Stores the reader state
		*
		* This template is intended for use with #string_reader_enum, #number_reader_enum, #array_reader_enum, and #object_reader_enum
		*/
		template<typename T>
		void set_state(const T state)
		{
			this->read_state = (char)state;
		}

		/*! \brief Enumeration of the state machine for strings */
		enum string_reader_enum
		{
			STRING_EMPTY = 0, ///< No values have been read
			STRING_OPENING_QUOTE, ///< The opening quote has been read. Equivalant to #STRING_OPEN, but used for debugging the state
			STRING_OPEN, ///< The opening quote has been read and the last character was not an escape character
			STRING_ESCAPED, ///< The last character was an reverse solidus (\), indicating the next character should be a control character 
			STRING_CODE_POINT_START, ///< An encoded unicode character is encountered. Expecting four following hex digits. 
			STRING_CODE_POINT_1, ///< An encoded unicode character is encountered. Expecting three following hex digits (one has already been read). 
			STRING_CODE_POINT_2, ///< An encoded unicode character is encountered. Expecting two following hex digits (two have already been read). 
			STRING_CODE_POINT_3, ///< An encoded unicode character is encountered. Expecting one following hex digit (three has already been read). 
			STRING_CLOSED ///< The closing quote has been read. Reading should cease. 
		};

		enum number_reader_enum
		{
			NUMBER_EMPTY = 0, ///< No values have been read
			NUMBER_OPEN_NEGATIVE, ///< A negative value has been read as the first character
			NUMBER_ZERO, ///< A zero has been read as an integer value
			NUMBER_INTEGER_DIGITS, ///< Integer digits were the last values read
			NUMBER_DECIMAL, ///< A decimal point was the last value read
			NUMBER_FRACTION_DIGITS, ///< A decimal point and subsequent digits were the last values read
			NUMBER_EXPONENT, ///< An exponent indicator has been read
			NUMBER_EXPONENT_SIGN, ///< An exponent sign has been read
			NUMBER_EXPONENT_DIGITS ///< An exponent indicator and subsequent digits were the last values read
		};

		enum array_reader_enum
		{
			ARRAY_EMPTY = 0, ///< No values have been read
			ARRAY_OPEN_BRACKET, ///< The array has been opened
			ARRAY_READING_VALUE, ///< An array value is being read
			ARRAY_AWAITING_NEXT_LINE, ///< An array value has been read and a comma was encountered. Expecting new line. 
			ARRAY_CLOSED ///< The array has been fully read. Reading should stop. 
		};

		enum object_reader_enum
		{
			OBJECT_EMPTY = 0, ///< No values have been read
			OBJECT_OPEN_BRACE, ///< The object has been opened
			OBJECT_READING_ENTRY, ///< An object key value pair is being read
			OBJECT_AWAITING_NEXT_LINE, ///< An object key value pair has been read and a comma was encountered. Expecting new line. 
			OBJECT_CLOSED ///< The object has been fully read. Reading should stop. 
		};
	private:
		/*! \brief Storage for the current state of the reader */
		char read_state;
	};

	/*! \brief Class for reading object key value pairs */
	class kvp_reader : public reader
	{
	public:
		/*! \brief Constructor */
		inline kvp_reader() : reader()
		{ 
			this->clear();
		}

		/*! \brief Resets the reader */
		inline virtual void clear() 
		{ 
			reader::clear();
			this->_key.clear();
			this->_colon_read = false;
		}

		/*!\ brief Pushes a value to the back of the reader 
		*
		* \see reader::push
		*/
		virtual push_result push(const char next);

		/*! \brief Checks if the stored value is valid 
		* 
		* \returns `true` if the both the key and value are valid, `false` otherwise 
		*/
		inline virtual bool is_valid() const
		{
			return reader::is_valid() && this->_key.is_valid();
		}

		/*! \brief Reads out the key value pair
		*
		* \returns JSON-encoded key and JSON-encoded value seperated by a colon (:)
		*/
		virtual std::string serialize() const;

	private:
		/*! \brief Reader for reading the key */
		reader _key;

		/*! \brief Flag for tracking whether the colon has been encountered */
		bool _colon_read;
	};

	/*! \brief Namespace for handling JSON data */
	namespace data
	{
		typedef enum data_format_enum
		{
			UINT8,
			INT8
		} data_format_t;

		#define ABSTRACT_SET_AND_GET(format) 		\
			virtual void set(format) = 0; 			\
			virtual operator format() const = 0;

		#define SET_AND_GET(format) 				\
			virtual void set(format); 				\
			virtual operator format() const;

		#define SET_AND_GET_AND_CONSTRUCT(obj, format)	\
			virtual void set(format); 					\
			virtual operator format() const;

		class data_interface : public data_container
		{
		public:
			virtual void set_null() = 0;
			virtual void set_true() = 0;
			virtual void set_false() = 0;

			ABSTRACT_SET_AND_GET(uint8_t);
			ABSTRACT_SET_AND_GET(int8_t);
			ABSTRACT_SET_AND_GET(uint16_t);
			ABSTRACT_SET_AND_GET(int16_t);
			ABSTRACT_SET_AND_GET(uint32_t);
			ABSTRACT_SET_AND_GET(int32_t);
			ABSTRACT_SET_AND_GET(uint64_t);
			ABSTRACT_SET_AND_GET(int64_t);
			ABSTRACT_SET_AND_GET(float);
			ABSTRACT_SET_AND_GET(double);
			ABSTRACT_SET_AND_GET(std::string);
			ABSTRACT_SET_AND_GET(jarray);
			ABSTRACT_SET_AND_GET(jobject);

			virtual bool is_true() const = 0;
			virtual bool is_null() const = 0;
			virtual std::string as_string() const = 0; 
			virtual jarray as_array() const = 0;
			virtual jobject as_object() const = 0;

			inline bool is_number() const { return this->type() == jtype::jnumber; }
			inline bool is_array() const { return this->type() == jtype::jarray; }
			inline bool is_bool() const { return this->type() == jtype::jbool; }
			inline bool is_object() const { return this->type() == jtype::jobject; }
			inline bool is_string() const { return this->type() == jtype::jstring; }
		};

		class dynamic_data : public data_interface
		{
		public:
			inline dynamic_data() { }
			dynamic_data(const reader &input);
			inline dynamic_data(const dynamic_data &other)
				: __value(other.__value)
			{ }

			virtual inline ~dynamic_data() { }

			void operator= (const reader &input);
			inline void operator=(const dynamic_data &other) { this->__value = other.__value; }

			virtual jtype::jtype type() const;
			virtual void set_null();
			virtual void set_true();
			virtual void set_false();

			SET_AND_GET_AND_CONSTRUCT(dynamic_data, uint8_t);
			SET_AND_GET_AND_CONSTRUCT(dynamic_data, int8_t);
			SET_AND_GET_AND_CONSTRUCT(dynamic_data, uint16_t);
			SET_AND_GET_AND_CONSTRUCT(dynamic_data, int16_t);
			SET_AND_GET_AND_CONSTRUCT(dynamic_data, uint32_t);
			SET_AND_GET_AND_CONSTRUCT(dynamic_data, int32_t);
			SET_AND_GET_AND_CONSTRUCT(dynamic_data, uint64_t);
			SET_AND_GET_AND_CONSTRUCT(dynamic_data, int64_t);
			SET_AND_GET_AND_CONSTRUCT(dynamic_data, float);
			SET_AND_GET_AND_CONSTRUCT(dynamic_data, double);
			SET_AND_GET_AND_CONSTRUCT(dynamic_data, std::string);
			SET_AND_GET_AND_CONSTRUCT(dynamic_data, jarray);
			SET_AND_GET_AND_CONSTRUCT(dynamic_data, jobject);

			template<typename T>
			dynamic_data(const T value) { this->set(value); }

			virtual std::string as_string() const; 
			virtual json::jarray as_array() const;
			virtual json::jobject as_object() const;

			virtual bool is_true() const;
			virtual bool is_null() const;

			virtual std::string serialize() const;
		private:
			std::string __value;
		};

		class link
		{
		public:
			inline link() : __parent(NULL) { }
			link(const link &other);
			link(jobject *parent);
			link& operator= (const link &other);
			virtual inline ~link() { this->detatch(); }
			inline jobject * parent() const { return this->__parent; }
			inline operator jobject* () const { return this->parent(); }
			inline bool attached() const { return this->__parent != NULL; }
			void detatch();
		private:
			void attach();
			jobject * __parent;
		};
	}

	class proxy : public data::data_interface
	{
	public:
		proxy();
		proxy(const proxy &other);
		proxy(jobject *parent, const std::string key);
		proxy(jobject *parent, const char *string);
		proxy& operator=(const proxy &other);

		virtual jtype::jtype type() const;
		virtual void set_null();
		virtual void set_true();
		virtual void set_false();

		SET_AND_GET(uint8_t);
		SET_AND_GET(int8_t);
		SET_AND_GET(uint16_t);
		SET_AND_GET(int16_t);
		SET_AND_GET(uint32_t);
		SET_AND_GET(int32_t);
		SET_AND_GET(uint64_t);
		SET_AND_GET(int64_t);
		SET_AND_GET(float);
		SET_AND_GET(double);
		SET_AND_GET(std::string);
		SET_AND_GET(jarray);
		SET_AND_GET(jobject);

		template<typename T>
		inline proxy& operator=(const T input) { this->set(input); return *this; }

		virtual bool is_true() const;
		virtual bool is_null() const;
		virtual std::string as_string() const; 
		virtual jarray as_array() const;
		virtual jobject as_object() const;
		virtual std::string serialize() const;
	private:
		data::link __parent;
		std::string __key;
	};

	/*! \brief Namespace used for JSON parsing functions */
	namespace parsing
	{
		/*! \brief (t)rims (l)eading (w)hite (s)pace
		 *
		 * \details Given a string, returns a pointer to the first character that is not white space. Spaces, tabs, and carriage returns are all considered white space. 
		 * @param start The string to examine
		 * @return A pointer within the input string that points at the first charactor that is not white space
		 * \note If the string consists of entirely white space, then the null terminator is returned
		 * \warning The behavior of this function with string that is not null-terminated is undefined
		 */
		const char* tlws(const char *start);

		/*! \brief Decodes a string in JSON format
		 *
		 * \details The quotation mark ("), reverse solidus (\), solidus (/), backspace (b), formfeed (f), linefeed (n), carriage return (r), horizontal tab (t), and Unicode character will be unescaped
		 * @param input A string, encapsulated in quotations ("), potentially containing escaped control characters
		 * @return A string with control characters un-escaped
		 * \note This function will strip leading and trailing quotations. 
		 * @see encode_string
		 */
		std::string decode_string(const char * input);

		/*! \brief Encodes a string in JSON format
		 *
		 * \details The quotation mark ("), reverse solidus (\), solidus (/), backspace (b), formfeed (f), linefeed (n), carriage return (r), horizontal tab (t), and Unicode character will be escaped
		 * @param input A string potentially containing control characters
		 * @return A string that has all control characters escaped with a reverse solidus (\)
		 * \note This function will add leading and trailing quotations. 
		 * @see decode_string
		 */
		std::string encode_string(const char *input);

		/*! \brief Structure for capturing the results of parsing */
		struct parse_results
		{
			/*! \brief The type of value encountered while parsing */
			jtype::jtype type; 

			/*! \brief The parsed value encountered */
			std::string value; 

			/*! \brief A pointer to the first character after the parsed value */
			const char *remainder;
		};

		/*! \brief Parses the first value encountered in a JSON string
		 *
		 * @param input The string to be parsed
		 * @return Details regarding the first value encountered 
		 * \exception json::parsing_error Exception thrown when the input is not valid JSON
		 */
		parse_results parse(const char *input);
		
		/*! \brief Template for reading a numeric value 
		 * 
		 * @tparam T The C data type the input will be convered to
		 * @param input The string to conver to a number
		 * @param format The format to use when converting the string to a number
		 * @return The numeric value contained by the input
		 */
		template <typename T>
		T get_number(const char *input, const char* format)
		{
			T result;
			std::sscanf(input, format, &result);
			return result;
		}

		/*! \brief Converts a number to a string
		 * 
		 * @tparam The C data type of the number to be converted
		 * @param number A reference to the number to be converted
		 * @param format The format to be used when converting the number
		 * @return A string representation of the input number
		 */ 
		template <typename T>
		std::string get_number_string(const T &number, const char *format)
		{
			std::vector<char> cstr(6);
			int remainder = std::snprintf(&cstr[0], cstr.size(), format, number);
			if(remainder < 0) {
				return std::string();
			} else if(remainder >= (int)cstr.size()) {
				cstr.resize(remainder + 1);
				std::snprintf(&cstr[0], cstr.size(), format, number);
			}
			std::string result(&cstr[0]);
			return result;
		}

		/*! \brief Parses a JSON array
		 *
		 * \details Converts a serialized JSON array into a vector of the values in the array
		 * @param input The serialized JSON array
		 * @return A vector containing each element of the array with each element being serialized JSON
		 */
		std::vector<std::string> parse_array(const char *input);
	}

	class jarray : public data_container, public std::vector<data::dynamic_data>
	{
	public:
		inline jarray() : std::vector<data::dynamic_data>() { }

		inline jtype::jtype type() const { return jtype::jarray; }

		template<typename T>
		jarray& operator=(const std::vector<T> input)
		{
			this->set(input);
			return *this;
		}

		template<typename T>
		jarray(const std::vector<T> input)
		{
			this->set(input);
		}

		template<typename T>
		void set(const std::vector<T> input)
		{
			this->clear();
			this->reserve(input.size());
			for(size_t i = 0; i < input.size(); i++)
			{
				this->push_back((T)input.at(i));
			}
		}

		static jarray parse(const char *input);
		static inline jarray parse(const std::string &input) { return jarray::parse(input.c_str()); }
		std::string as_string() const;
		virtual inline std::string serialize() const { return this->as_string(); }
		inline operator std::string() const { return this->as_string(); }
		std::string pretty(unsigned int indent_level = 0) const;
		template<typename T>
		operator std::vector<T>() const
		{
			std::vector<T> result;
			for(size_t i = 0; i < this->size(); i++)
			{
				result.push_back((T)this->at(i));
			}
			return result;
		}
	};

	typedef std::pair<std::string, std::string> kvp;
	typedef std::map<std::string, std::string> jmap;

	/*! \class jobject
	 * \brief The class used for manipulating JSON objects and arrays
	 *
	 * \example jobject.cpp
	 * This is a basic of example of using simpleson for manipulating JSON
	 *
	 * \example rootarray.cpp
	 * This is an example of how to handle JSON where the root object is an array
	 * 
	 * \example objectarray.cpp
	 * This is an example of how to handle an array of JSON objects
	 */
	class jobject : public data_container
	{
	private:
		/*! \brief The container used to store the object's data */
		jmap data;

		std::list<data::link*> __proxies;

	public:
		/*! \brief Default constructor
		 */
		inline jobject() { }

		/*! \brief Copy constructor */
		inline jobject(const jobject &other)
			: data(other.data)
		{ 
			// Do not copy proxies
		}

		/*! \brief Destructor */
		virtual ~jobject();

		virtual inline jtype::jtype type() const { return jtype::jobject; }

		/*! \brief Returns the number of entries in the JSON object or array */
		inline size_t size() const { return this->data.size(); }

		/*! \brief Clears the JSON object or array */
		void clear();

		void attach(data::link *prox);

		void detatch(data::link *prox);

		/*! \brief Comparison operator
		 *
		 * \todo Currently, the comparison just seralizes both objects and compares the strings, which is probably not as efficent as it could be
		 */
		bool operator== (const json::jobject other) const { return ((std::string)(*this)) == (std::string)other; }

		/*! \brief Comparison operator */
		bool operator!= (const json::jobject other) const { return ((std::string)(*this)) != (std::string)other; }

		/*! \brief Assignment operator */
		inline jobject& operator=(const jobject rhs)
		{
			this->data = rhs.data;
			return *this;
		}

		/*! \brief Appends a key-value pair to a JSON object
		 *
		 * \exception json::parsing_error Thrown if the key-value is incompatable with the existing object (object/array mismatch)
		 */
		jobject& operator+=(const kvp& other)
		{
			this->data.insert(other);
			return *this;
		}

		/*! \brief Appends one JSON object to another */
		jobject& operator+=(const jobject& other)
		{
			for (json::jmap::const_iterator it = other.data.begin(); it != other.data.end(); ++it)
			{
				if(this->has_key(it->first)) throw json::invalid_key("Key conflict");
				this->data.insert(kvp(it->first, it->second));
			}
			return *this;
		}

		/*! \brief Merges two JSON objects */
		jobject operator+(jobject& other)
		{
			jobject result = *this;
			result += other;
			return result;
		}

		/*! \brief Parses a serialized JSON string
		 *
		 * @param input Serialized JSON string
		 * @return JSON object or array
		 * \exception json::parsing_error Thrown when the input string is not valid JSON
		 */
		static jobject parse(const char *input);

		/*! \brief Parses a serialized JSON string 
		 *
		 * @see json::jobject::parse(const char*)
		 */
		static inline jobject parse(const std::string input) { return parse(input.c_str()); }

		/*! /brief Attempts to parse the input string
		 * 
		 * @param input A serialized JSON object or array
		 * @param[out] output Should the parsing attempt be successful, the resultant JSON object or array
		 * @return True of the parsing attempt was successful and false if the parsing attempt was not successful
		 */
		inline bool static tryparse(const char *input, jobject &output)
		{
			try
			{
				output = parse(input);
			}
			catch(...)
			{
				return false;
			}
			return true;
		}

		/*! \brief Determines if an object contains a key
		 *
		 * @param key The key to check for
		 * @return True if the object contains the provided key and false if the object does not contain the key
		 * \note If the object represents a JSON array, then this function will always return false
		 */
		inline bool has_key(const std::string &key) const
		{
			return this->data.find(key) != this->data.end();
		}

		/*! \brief Returns a list of the object's keys
		 *
		 * @return A list of keys contained in the object. If the object is actionally an array, an empty list will be returned
		 */
		key_list_t list_keys() const;

		/*! \brief Sets the value assocaited with the key
		 *
		 * \details If the key exists, then the value is updated. If the key does not exist, then the key value pair is added to the object. 
		 * @param key The key for the entry
		 * @param value The value for the entry
		 * \exception json::invalid_key Exception thrown if the object actually represents a JSON array
		 */
		void set(const std::string &key, const std::string &value);

		/*! \brief Returns the serialized value associated with a key
		 * 
		 * @param key The key for the desired element
		 * @return A serialized representation of the value associated with the key
		 * \exception json::invalid_key Exception thrown if the key does not exist in the object or the object actually represents a JSON array
		 */
		inline std::string get(const std::string &key) const
		{
			std::map<std::string, std::string>::const_iterator it = this->data.find(key);
			if(it == this->data.end()) throw json::invalid_key(key);
			return it->second;
		}

		/*! \brief Removes the entry associated with the key
		 *
		 * @param key The key of the key value pair to be removed
		 * \note If the key is not found in the object, no action is taken
		 */
		void remove(const std::string &key);

		/*! \brief Returns an element of the JSON object
		 * 
		 * @param key The key of the element to be returned
		 * @return A proxy for the value paired with the key
		 * \exception json::invalid_key Exception thrown if the object is actually a JSON array
		 */
		inline virtual proxy operator[](const std::string key)
		{
			return json::proxy(this, key);
		}

		/*! \see json::jobject::as_string() */
		operator std::string() const;

		/*! \brief Serialzes the object or array 
		 * \note The serialized object or array will be in the most compact form and will not contain any extra white space, even if the serialized string used to generate the object or array contained extra white space. 
		 */
		inline std::string as_string() const
		{
			return this->operator std::string();
		}

		virtual inline std::string serialize() const { return this->as_string(); }

		/*! \brief Returns a pretty (multi-line indented) serialzed representation of the object or array
		 * 
		 * @param indent_level The number of indents (tabs) to start with
		 * @return A "pretty" version of the serizlied object or array
		 */
		std::string pretty(unsigned int indent_level = 0) const;
	};
}

#endif // !JSON_H