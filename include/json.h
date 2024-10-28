#ifndef TY_JSON
#define TY_JSON

#include <iostream>
#include <string>
#include <vector>
#include <cassert>
#include <cctype>
#include <unordered_map>

namespace ty
{
    typedef enum
    {
        JSON_SUCCESS,
        JSON_FAIL,

        JSON_PARSE_OK,
        JSON_PARSE_FAIL,
        JSON_PARSE_OUT_OF_RANGE,
        JSON_PARSE_NOT_EXPECT_VALUE,
        JSON_PARSE_NUMBER_ERROR,
        JSON_PARSE_STRING_ERROR,
        JSON_PARSE_UNICODE_ERROR,
        JSON_PARSE_UNICODE_HEX_ERROR,
        JSON_PARSE_ARRAY_ERROR,
        JSON_PARSE_MISS_COMMA_OR_END,
        JSON_PARSE_REDUNDANT_COMMA,
        JSON_PARSE_OBJECT_ERROR,
        JSON_PARSE_OBJECT_KEY_ALREADY_EXISTS,
        JSON_PARSE_ABSENCE_SPLIT_SYMBOL,

        JSON_STRINIFY_OK,
        JSON_STRINIFY_FAIL,
    } json_status;

    typedef enum
    {
        JSON_NULL,
        JSON_TRUE,
        JSON_FALSE,
        JSON_NUMBER,
        JSON_STRING,
        JSON_ARRAY,
        JSON_OBJECT
    } json_type;

    typedef enum
    {
        JSON_FLAG_NONE,
        JSON_FLAG_UNESCAPED_UNICODE
    } json_flag;

    typedef struct json_context json_context;
    struct json_context
    {
        const std::string json;
        size_t index;
        json_context(const std::string json_str) : json(json_str), index(0) {};
    };

    class json
    {
    private:
        typedef double number_t;
        typedef std::string string_t;
        typedef std::vector<json *> array_t;
        typedef std::unordered_map<std::string, json *> object_t;

        json_status err_code;

        struct
        {
            union
            {
                number_t number;
                string_t *string;
                array_t *array;
                object_t *object;
            };

            json_type type = JSON_NULL;
        } value;

    public:
        typedef std::pair<std::string, json> pair;
        json() = default;
        ~json()
        {
            this->free();
        };

        json(const json &v)
        {
            this->operator=(v);
        }

        json(double v)
        {
            this->value.type = JSON_NUMBER;
            this->value.number = v;
        }

        json(const std::string &v)
        {
            this->value.type = JSON_STRING;
            this->value.string = new std::string(v);
        }

        template <typename T, typename std::enable_if<std::is_same<T, const char *>::value, int>::type = 0>
        json(T v)
        {
            this->value.type = JSON_STRING;
            this->value.string = new std::string(v);
        }

        template <typename T, typename std::enable_if<std::is_same<T, bool>::value, int>::type = 0>
        json(T v)
        {
            this->value.type = v ? JSON_TRUE : JSON_FALSE;
            this->value.number = v;
        }

        template <typename T, typename std::enable_if<std::is_same<T, pair>::value, int>::type = 0>
        json(std::initializer_list<T> list)
        {
            this->value.type = JSON_OBJECT;
            this->value.object = new object_t();
            for (auto &item : list)
            {
                this->value.object->emplace(item.first, new json(item.second));
            }
        }

        json(std::initializer_list<json> list)
        {
            this->value.type = JSON_ARRAY;
            this->value.array = new array_t();
            for (auto &item : list)
            {
                this->value.array->push_back(new json(item));
            }
        }

        template <typename T>
        friend typename std::enable_if<std::is_same<T, json>::value, bool>::type
        operator==(const T &lhs, const T &rhs);
        json &operator=(const json &v)
        {
            this->free();
            this->set_type(v.get_type());
            switch (this->get_type())
            {
            case JSON_NUMBER:
                this->value.number = v.value.number;
                break;
            case JSON_STRING:
                this->value.string = new string_t(*v.value.string);
                break;
            case JSON_ARRAY:
                this->value.array = new array_t();
                for (const auto &item : *v.value.array)
                {
                    this->value.array->push_back(new json(*item));
                }
                break;
            case JSON_OBJECT:
                this->value.object = new object_t();
                for (const auto &item : *v.value.object)
                {

                    this->value.object->emplace(item.first, new json(*item.second));
                }
                break;
            }
            return *this;
        }

        json &operator=(double number);
        json &operator=(const char *string);
        json &operator=(const std::string &string)
        {
            return operator=(string.data());
        };
        json &operator[](size_t index);
        const json &operator[](size_t index) const;

        template <typename T, typename std::enable_if<std::is_same<T, const char *>::value, int>::type = 0>
        json &operator[](T key)
        {
            if (this->value.type != JSON_OBJECT)
            {
                throw std::bad_cast();
            }
            return *(this->value.object->operator[](key));
        };

        template <typename T, typename std::enable_if<std::is_same<T, const char *>::value, int>::type = 0>
        const json &operator[](T key) const
        {
            if (this->value.type != JSON_OBJECT)
            {
                throw std::bad_cast();
            }
            return *(this->value.object->operator[](key));
        };

        friend std::ostream &operator<<(std::ostream &os, const json &v);
        operator double() const;
        operator const char *() const;

        static json parse(const std::string json_str);
        const std::string get_error();
        json_type get_type() const;
        size_t size() const;
        void free();

        template <typename T>
        auto get() -> T
        {
            return static_cast<T>(*this);
        }

        std::string stringify(json_flag flag = JSON_FLAG_NONE);

    private:
        static void parse(json_context *c, json *v);
        static json_status parse_value(json_context *c, json *v);
        static json_status parse_value_liberal(json_context *c, json *v, const char *expect_parse_value, size_t size, json_type type);
        static json_status parse_value_number(json_context *c, json *v);
        static json_status parse_value_string(json_context *c, json *v);
        static json_status parse_value_string(json_context *c, std::string *v);
        static json_status parse_value_array(json_context *c, json *v);
        static json_status parse_value_object(json_context *c, json *v);

        static json_status stringify_value(json *v, std::string &string, json_flag flag = JSON_FLAG_NONE);
        static json_status stringify_value_string(json *v, std::string &string, json_flag flag = JSON_FLAG_NONE);
        static json_status stringify_value_string_unicode(json *v, std::string &string, json_flag flag = JSON_FLAG_NONE);

        static json_status hex4_to_uint(unsigned int *u, json_context *c);
        static json_status uint_to_hex4(unsigned int *u, std::string *v);
        static json_status encode_utf8(unsigned int *u, std::string *v);
        static json_status decode_utf8(unsigned int *u, std::string *v, size_t &index);

        static void parse_whitespace(json_context *c);

        void set_type(json_type type);

    public:
        class iterator : public std::iterator<std::input_iterator_tag, json>
        {
        private:
            json *p;
            union
            {
                array_t::iterator a;
                object_t::iterator o;
            };

        public:
            iterator(json *x, bool begin = true) : p(x)
            {
                if (p->get_type() == JSON_ARRAY)
                {
                    a = begin ? p->value.array->begin() : p->value.array->end();
                }
                else if (p->get_type() == JSON_OBJECT)
                {
                    o = begin ? p->value.object->begin() : p->value.object->end();
                }
            }
            iterator(const iterator &it) : p(it.p)
            {
                if (p->get_type() == JSON_ARRAY)
                {
                    a = it.a;
                }
                else if (p->get_type() == JSON_OBJECT)
                {
                    o = it.o;
                }
            }
            iterator &operator++()
            {
                if (p->get_type() == JSON_ARRAY)
                {
                    ++a;
                }
                else if (p->get_type() == JSON_OBJECT)
                {
                    ++o;
                }
                return *this;
            }
            bool operator==(const iterator &rhs) const
            {
                if (p->get_type() == JSON_ARRAY)
                {
                    return a == rhs.a;
                }
                else if (p->get_type() == JSON_OBJECT)
                {
                    return o == rhs.o;
                }
                return false;
            }
            bool operator!=(const iterator &rhs) const
            {
                if (p->get_type() == JSON_ARRAY)
                {
                    return a != rhs.a;
                }
                else if (p->get_type() == JSON_OBJECT)
                {
                    return o != rhs.o;
                }
                return false;
            }
            json &operator*()
            {
                if (p->get_type() == JSON_ARRAY)
                {
                    return **a;
                }
                else if (p->get_type() == JSON_OBJECT)
                {
                    return *o->second;
                }
                return *p;
            }
            const std::string &key()
            {
                return o->first;
            }
            json &value()
            {
                return *o->second;
            }
        };

        class iterator_const : public std::iterator<std::input_iterator_tag, json>
        {
        private:
            const json *p;
            union
            {
                array_t::iterator a;
                object_t::iterator o;
            };

        public:
            iterator_const(const json *x, bool begin = true) : p(x)
            {
                if (p->get_type() == JSON_ARRAY)
                {
                    a = begin ? p->value.array->begin() : p->value.array->end();
                }
                else if (p->get_type() == JSON_OBJECT)
                {
                    o = begin ? p->value.object->begin() : p->value.object->end();
                }
            }
            iterator_const(const iterator_const &it) : p(it.p)
            {
                if (p->get_type() == JSON_ARRAY)
                {
                    a = it.a;
                }
                else if (p->get_type() == JSON_OBJECT)
                {
                    o = it.o;
                }
            }
            iterator_const &operator++()
            {
                if (p->get_type() == JSON_ARRAY)
                {
                    ++a;
                }
                else if (p->get_type() == JSON_OBJECT)
                {
                    ++o;
                }
                return *this;
            }
            bool operator==(const iterator_const &rhs) const
            {
                if (p->get_type() == JSON_ARRAY)
                {
                    return a == rhs.a;
                }
                else if (p->get_type() == JSON_OBJECT)
                {
                    return o == rhs.o;
                }
                return false;
            }
            bool operator!=(const iterator_const &rhs) const
            {
                if (p->get_type() == JSON_ARRAY)
                {
                    return a != rhs.a;
                }
                else if (p->get_type() == JSON_OBJECT)
                {
                    return o != rhs.o;
                }
                return false;
            }
            const json &operator*()
            {
                if (p->get_type() == JSON_ARRAY)
                {
                    return **a;
                }
                else if (p->get_type() == JSON_OBJECT)
                {
                    return *o->second;
                }

                return *p;
            }
            const std::string &key()
            {
                return o->first;
            }
            const json &value()
            {
                return *o->second;
            }
        };

        iterator begin()
        {
            return iterator(this, true);
        }

        iterator end()
        {
            return iterator(this, false);
        }

        const iterator_const begin() const
        {
            return cbegin();
        }

        const iterator_const end() const
        {
            return cend();
        }

        const iterator_const cbegin() const
        {
            return iterator_const(this, true);
        }

        const iterator_const cend() const
        {
            return iterator_const(this, false);
        }
    };

    template <typename T>
    typename std::enable_if<std::is_same<T, json>::value, bool>::type
    operator==(const T &lhs, const T &rhs)
    {
        bool ret = lhs.get_type() == rhs.get_type();
        if (!ret)
            return ret;
        switch (lhs.get_type())
        {
        case JSON_NUMBER:
            ret = lhs.value.number == rhs.value.number;
            break;
        case JSON_STRING:
            ret = *lhs.value.string == *rhs.value.string;
            break;
        case JSON_ARRAY:
            if (lhs.value.array->size() == rhs.value.array->size())
            {
                for (size_t i = 0; i < lhs.value.array->size(); i++)
                {
                    if (!(ret = *lhs.value.array->at(i) == *lhs.value.array->at(i)))
                    {
                        break;
                    }
                }
            }
            else
            {
                ret = false;
            }
            break;
        case JSON_OBJECT:
            if (lhs.value.object->size() == rhs.value.object->size())
            {
                for (auto it = lhs.begin(); it != lhs.end(); ++it)
                {
                    if (!(ret = *lhs.value.object->at(it.key()) == *lhs.value.object->at(it.key())))
                    {
                        break;
                    }
                }
            }
            else
            {
                ret = false;
            }
            break;
        }
        return ret;
    };

}

#endif
