#include "../include/json.h"
#include "json.h"
namespace ty
{

    json json::parse(const std::string json_str)
    {
        json v;
        json_context *c = new json_context(json_str);
        parse(c, &v);
        delete c;
        return v;
    }

    void json::parse(json_context *c, json *v)
    {
        parse_whitespace(c);
        v->err_code = parse_value(c, v);
        if (v->err_code != JSON_PARSE_OK)
        {
            v->set_type(JSON_NULL);
        }
    }

    json_status json::parse_value(json_context *c, json *v)
    {
        json_status status;
        switch (c->json[c->index])
        {
        case 'n':
            status = parse_value_liberal(c, v, "null", 4, JSON_NULL);
            break;
        case 't':
            status = parse_value_liberal(c, v, "true", 4, JSON_TRUE);
            break;
        case 'f':
            status = parse_value_liberal(c, v, "false", 5, JSON_FALSE);
            break;
        case '"':
            status = parse_value_string(c, v);
            break;
        case '[':
            status = parse_value_array(c, v);
            break;
        case '{':
            status = parse_value_object(c, v);
            break;
        default:
            status = parse_value_number(c, v);
            break;
        }
        return status;
    }

    json_status json::parse_value_liberal(json_context *c, json *v, const char *expect_parse_value, size_t size, json_type type)
    {
        for (size_t i = 0; i < size; i++, c->index++)
        {
            if (c->json[c->index] != expect_parse_value[i])
            {
                return JSON_PARSE_NOT_EXPECT_VALUE;
            }
        }

        v->set_type(type);
        return JSON_PARSE_OK;
    }

    json_status json::parse_value_number(json_context *c, json *v)
    {
        try
        {
            const std::string &json = c->json;
            size_t &index = c->index;
            int sign = 1;
            if (json.at(index) == '-')
            {
                ++index;
                sign = -1;
            }

            if (!isdigit(json.at(index)))
            {
                return JSON_PARSE_NUMBER_ERROR;
            }

            double result = 0;
            while (index < json.size() && isdigit(json.at(index)))
            {
                result = result * 10 + (json.at(index) - '0');
                ++index;
            }

            if (index < json.size() && json.at(index) == '.')
            {
                ++index;

                double factor = 0.1;
                while (index < json.size() && isdigit(json.at(index)))
                {
                    result += (json.at(index) - '0') * factor;
                    factor *= 0.1;
                    ++index;
                }
            }

            if (index < json.size() && (json.at(index) == 'e' || json.at(index) == 'E'))
            {
                int e = 0;
                bool e_sign = true;
                ++index;
                if (index < json.size() && json.at(index) == '-')
                {
                    e_sign = false;
                    ++index;
                }
                else if (index < json.size() && json.at(index) == '+')
                {
                    ++index;
                }

                if (!isdigit(json.at(index)))
                {
                    return JSON_PARSE_NUMBER_ERROR;
                }

                while (index < json.size() && isdigit(json.at(index)))
                {
                    e = e * 10 + (json.at(index) - '0');
                    ++index;
                }

                if (e_sign)
                {
                    while (e--)
                        result *= 10;
                }
                else
                {
                    while (e--)
                        result /= 10;
                }
            }

            result *= sign;

            v->set_type(JSON_NUMBER);
            *v = result;
            return JSON_PARSE_OK;
        }
        catch (const std::out_of_range &e)
        {
            return JSON_PARSE_OUT_OF_RANGE;
        }
    }

    json_status json::parse_value_string(json_context *c, json *v)
    {
        v->value.string = new std::string();
        json_status status = parse_value_string(c, v->value.string);
        if (status != JSON_PARSE_OK)
        {
            delete v->value.string;
            v->value.type = JSON_NULL;
        }
        v->value.type = JSON_STRING;
        return status;
    }

    json_status json::parse_value_string(json_context *c, std::string *v)
    {
        const std::string &json = c->json;
        size_t &index = c->index;
        if (json.at(index) != '"')
        {
            return JSON_PARSE_NOT_EXPECT_VALUE;
        }
        ++index;
        if (json.at(index) == '"')
        {
            *v = "";
            return JSON_PARSE_OK;
        }
        unsigned int u = 0;
        json_status status;
        while (index < json.size() && json[index] != '"')
        {
            if (json[index] == '\\')
            {
                ++index;
                char ch = json[index++];
                switch (ch)
                {
                case '\\':
                    v->push_back('\\');
                    break;
                case 'n':
                    v->push_back('\n');
                    break;
                case 'r':
                    v->push_back('\r');
                    break;
                case 't':
                    v->push_back('\t');
                    break;
                case 'f':
                    v->push_back('\f');
                    break;
                case '"':
                    v->push_back('"');
                    break;
                case 'u':
                    if ((status = hex4_to_uint(&u, c)) != JSON_PARSE_OK)
                    {
                        return status;
                    }
                    encode_utf8(&u, v);
                    break;

                default:
                    v->push_back(ch);
                    break;
                }
            }
            else
            {
                v->push_back(json[index++]);
            }
        }

        if (index >= json.size() || json[index] != '"')
            return JSON_PARSE_STRING_ERROR;
        ++index;
        return JSON_PARSE_OK;
    }

    json_status json::parse_value_array(json_context *c, json *v)
    {

        const std::string &json = c->json;
        size_t &index = c->index;
        if (json[index] != '[')
        {
            return JSON_PARSE_ARRAY_ERROR;
        }
        ++index;
        v->value.array = new std::vector<ty::json *>();
        v->set_type(JSON_ARRAY);
        parse_whitespace(c);
        if (json[index] == ']')
        {
            return JSON_PARSE_OK;
        }

        json_status status;
        while (true)
        {
            if (json[index] == ']')
            {
                status = JSON_PARSE_REDUNDANT_COMMA;
                break;
            }

            ty::json *item = new ty::json();
            status = parse_value(c, item);
            if (status != JSON_PARSE_OK)
            {
                delete item;
                break;
            }
            v->value.array->push_back(item);
            parse_whitespace(c);

            if (json[index] == ',')
            {
                ++index;
            }
            else if (json[index] == ']')
            {
                ++index;
                return JSON_PARSE_OK;
            }
            else
            {
                status = JSON_PARSE_MISS_COMMA_OR_END;
                break;
            }
            parse_whitespace(c);
        }

        v->set_type(JSON_NULL);
        for (ty::json *item : *v->value.array)
        {
            delete item;
        }
        v->value.array->clear();

        return status;
    }

    json_status json::parse_value_object(json_context *c, json *v)
    {
        const std::string &json = c->json;
        size_t &index = c->index;
        if (json[index] != '{')
        {
            return JSON_PARSE_OBJECT_ERROR;
        }
        ++index;
        v->value.object = new std::unordered_map<std::string, ty::json *>();
        v->set_type(JSON_OBJECT);
        parse_whitespace(c);
        if (json[index] == '}')
        {
            return JSON_PARSE_OK;
        }

        json_status status;
        while (true)
        {
            if (json[index] == '}')
            {
                status = JSON_PARSE_REDUNDANT_COMMA;
                break;
            }

            std::string key;
            status = parse_value_string(c, &key);
            if (status != JSON_PARSE_OK)
            {
                break;
            }

            if (v->value.object->find(key) != v->value.object->end())
            {
                status = JSON_PARSE_OBJECT_KEY_ALREADY_EXISTS;
                break;
            }

            parse_whitespace(c);

            if (json[index] != ':')
            {
                status = JSON_PARSE_ABSENCE_SPLIT_SYMBOL;
                break;
            }

            ++index;

            parse_whitespace(c);

            ty::json *item = new ty::json();
            status = parse_value(c, item);
            if (status != JSON_PARSE_OK)
            {
                delete item;
                break;
            }
            v->value.object->emplace(key, item);
            parse_whitespace(c);
            if (json[index] == ',')
            {
                ++index;
            }
            else if (json[index] == '}')
            {
                ++index;
                return JSON_PARSE_OK;
            }
            else
            {
                status = JSON_PARSE_MISS_COMMA_OR_END;
                break;
            }
            parse_whitespace(c);
        }

        v->set_type(JSON_NULL);
        for (auto it : *v->value.object)
        {
            delete it.second;
        }
        v->value.object->clear();

        return status;
    }

    json_status json::hex4_to_uint(unsigned int *u, json_context *c)
    {
        const std::string &json = c->json;
        size_t &index = c->index;
        if (index + 4 >= json.size())
        {
            return JSON_PARSE_UNICODE_ERROR;
        }
        *u = 0;
        for (size_t i = 0; i < 4; i++, ++index)
        {
            *u = *u << 4;
            if (json[index] >= '0' && json[index] <= '9')
            {
                *u |= (json[index] - '0');
            }
            else if (json[index] >= 'a' && json[index] <= 'f')
            {
                *u |= (json[index] - 'a') + 10;
            }
            else if (json[index] >= 'A' && json[index] <= 'F')
            {
                *u |= (json[index] - 'A') + 10;
            }
            else
            {
                return JSON_PARSE_UNICODE_HEX_ERROR;
            }
        }

        // 代理对
        if (*u >= 0xD800 && *u <= 0xDBFF)
        {
            unsigned int low = 0;
            if (json[index] != '\\' && json[index + 1] != 'u')
            {
                return JSON_PARSE_UNICODE_ERROR;
            }
            index += 2;
            for (size_t i = 0; i < 4; i++, ++index)
            {
                low = low << 4;
                if (json[index] >= '0' && json[index] <= '9')
                {
                    low += (json[index] - '0');
                }
                else if (json[index] >= 'a' && json[index] <= 'f')
                {
                    low += (json[index] - 'a') + 10;
                }
                else if (json[index] >= 'A' && json[index] <= 'F')
                {
                    low += (json[index] - 'A') + 10;
                }
                else
                {
                    return JSON_PARSE_UNICODE_HEX_ERROR;
                }
            }

            if (low < 0xDC00 || low > 0xDFFF)
            {
                return JSON_PARSE_UNICODE_ERROR;
            }

            *u = (*u - 0xD800) * 0x400 + (low - 0xDC00) + 0x10000;
        }

        return JSON_PARSE_OK;
    }

    json_status ty::json::uint_to_hex4(unsigned int *u, std::string *v)
    {
        static const char hex[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
        if (*u <= 0x7F)
        {
            v->push_back(*u);
        }
        else if (*u >= 0x10000)
        {
            unsigned int h = 0, l = 0;
            h = ((*u - 0x10000) >> 10) + 0xD800;
            l = ((*u - 0x10000) & 0x3FF) + 0xDC00;

            v->push_back('\\');
            v->push_back('u');
            for (size_t i = 4; i > 0; --i)
            {
                v->push_back(hex[(h >> ((i - 1) * 4)) & 0xF]);
            }

            v->push_back('\\');
            v->push_back('u');
            for (size_t i = 4; i > 0; --i)
            {
                v->push_back(hex[(l >> ((i - 1) * 4)) & 0xF]);
            }
        }
        else
        {
            v->push_back('\\');
            v->push_back('u');
            for (size_t i = 4; i > 0; --i)
            {
                v->push_back(hex[(*u >> ((i - 1) * 4)) & 0xF]);
            }
        }

        return JSON_SUCCESS;
    }

    json_status json::encode_utf8(unsigned int *u, std::string *v)
    {
        if (*u <= 0x7F)
        {
            v->push_back(*u);
        }
        else if (*u <= 0x7FF)
        {
            v->push_back(((*u >> 6) & 0x1F) | 0xC0);
            v->push_back(((*u) & 0x3F) | 0x80);
        }
        else if (*u <= 0xFFFF)
        {
            v->push_back(((*u >> 12) & 0xF) | 0xE0);
            v->push_back(((*u >> 6) & 0x3F) | 0x80);
            v->push_back(((*u) & 0x3F) | 0x80);
        }
        else if (*u <= 0x10FFFF)
        {
            v->push_back(((*u >> 18) & 0x7) | 0xF0);
            v->push_back(((*u >> 12) & 0x3F) | 0x80);
            v->push_back(((*u >> 6) & 0x3F) | 0x80);
            v->push_back(((*u) & 0x3F) | 0x80);
        }
        return JSON_SUCCESS;
    }

    json_status ty::json::decode_utf8(unsigned int *u, std::string *v, size_t &index)
    {
        const std::string &json = *v;
        size_t size = v->size();
        *u = 0;
        if (index >= size)
        {
            return JSON_FAIL;
        }

        if ((json[index] & 0xF0) == 0xF0)
        {
            if (index + 3 >= size)
            {
                return JSON_FAIL;
            }

            *u |= (json[index] & 0x7) << 18;
            *u |= (json[++index] & 0x3F) << 12;
            *u |= (json[++index] & 0x3F) << 6;
            *u |= (json[++index] & 0x3F);
        }
        else if ((json[index] & 0xE0) == 0xE0)
        {
            if (index + 2 >= size)
            {
                return JSON_FAIL;
            }

            *u |= (json[index] & 0xF) << 12;
            *u |= (json[++index] & 0x3F) << 6;
            *u |= (json[++index] & 0x3F);
        }
        else if ((json[index] & 0xC0) == 0xC0)
        {
            if (index + 1 >= size)
            {
                return JSON_FAIL;
            }

            *u |= (json[index] & 0x1F) << 6;
            *u |= (json[++index] & 0x3F);
        }
        else if (json[index] <= 0x7F)
        {
            *u = json[index];
        }

        return JSON_SUCCESS;
    }

    std::string json::stringify(json_flag flag)
    {
        std::string json_stringify;
        this->err_code = stringify_value(this, json_stringify, flag);
        if (this->err_code != JSON_STRINIFY_OK)
        {
            return nullptr;
        }
        return json_stringify;
    }

    json_status json::stringify_value(json *v, std::string &string, json_flag flag)
    {
        switch (v->get_type())
        {
        case JSON_NULL:
            string += "null";
            break;
        case JSON_TRUE:
            string += "true";
            break;
        case JSON_FALSE:
            string += "false";
            break;
        case JSON_NUMBER:
            char temp_num[30];
            sprintf(temp_num, "%.15g", v->value.number);
            string += temp_num;
            break;
        case JSON_STRING:
            string.push_back('"');
            v->err_code = stringify_value_string(v, string, flag);
            if (v->err_code != JSON_STRINIFY_OK)
            {
                return v->err_code;
            }
            string.push_back('"');
            break;
        case JSON_ARRAY:
            string.push_back('[');
            for (auto it = v->value.array->begin(); it != v->value.array->end(); ++it)
            {
                if (it != v->value.array->begin())
                {
                    string.push_back(',');
                }
                v->err_code = stringify_value(*it, string, flag);
                if (v->err_code != JSON_STRINIFY_OK)
                {
                    return v->err_code;
                }
            }
            string.push_back(']');
            break;
        case JSON_OBJECT:
            string.push_back('{');
            for (auto it = v->value.object->begin(); it != v->value.object->end(); ++it)
            {
                if (it != v->value.object->begin())
                {
                    string.push_back(',');
                }
                string.push_back('"');
                string += it->first;
                string.push_back('"');
                string.push_back(':');
                v->err_code = stringify_value(it->second, string, flag);
                if (v->err_code != JSON_STRINIFY_OK)
                {
                    return v->err_code;
                }
            }
            string.push_back('}');
            break;
        default:
            return JSON_STRINIFY_FAIL;
            break;
        }
        return JSON_STRINIFY_OK;
    }

    json_status json::stringify_value_string(json *v, std::string &string, json_flag flag)
    {
        if (flag == JSON_FLAG_UNESCAPED_UNICODE)
        {
            string += *v->value.string;
            return JSON_STRINIFY_OK;
        }

        return stringify_value_string_unicode(v, string, flag);
    }

    json_status json::stringify_value_string_unicode(json *v, std::string &string, json_flag flag)
    {
        std::string &string_data = *v->value.string;
        size_t size = v->value.string->size();

        unsigned int u = 0;
        for (size_t i = 0; i < size; ++i)
        {
            if (decode_utf8(&u, &string_data, i) != JSON_SUCCESS)
            {
                return JSON_STRINIFY_FAIL;
            }

            if (uint_to_hex4(&u, &string) != JSON_SUCCESS)
            {
                return JSON_STRINIFY_FAIL;
            }
        }

        return JSON_STRINIFY_OK;
    }

    void json::parse_whitespace(json_context *c)
    {
        while (c->json[c->index] == ' ' || c->json[c->index] == '\t' || c->json[c->index] == '\r' || c->json[c->index] == '\n')
        {
            ++c->index;
        }
    }

    const std::string json::get_error()
    {
        std::string result = "status：";
        result = result + std::to_string(this->err_code);
        return result;
    }

    void json::set_type(json_type type)
    {
        this->value.type = type;
    }

    json_type json::get_type() const
    {
        return this->value.type;
    }

    size_t json::size() const
    {
        switch (this->get_type())
        {
        case JSON_FALSE:
            return 1;
        case JSON_TRUE:
            return 1;
        case JSON_NUMBER:
            return 1;
        case JSON_STRING:
            return this->value.string->size();
        case JSON_ARRAY:
            return this->value.array->size();
        }
        return 0;
    }

    void ty::json::free()
    {
        switch (this->get_type())
        {
        case JSON_OBJECT:
            for (auto &v : *this->value.object)
            {
                delete v.second;
            }
            this->value.object->clear();
            delete this->value.object;
            break;
        case JSON_ARRAY:
            for (auto &v : *this->value.array)
            {
                delete v;
            }
            this->value.array->clear();
            delete this->value.array;
            break;
        case JSON_NUMBER:
            this->value.number = 0;
            break;
        case JSON_STRING:
            delete this->value.string;
            this->value.string = nullptr;
            break;
        }
        this->set_type(JSON_NULL);
    }

    json::operator double() const
    {
        if (this->value.type == JSON_TRUE)
        {
            return true;
        }
        else if (this->value.type == JSON_FALSE)
        {
            return false;
        }
        else if (this->value.type == JSON_NUMBER)
        {
            return this->value.number;
        }
        throw std::bad_cast();
    }

    json::operator const char *() const
    {
        if (this->value.type != JSON_STRING)
        {
            throw std::bad_cast();
        }
        return this->value.string->data();
    }

    json &json::operator=(double number)
    {
        if (this->value.type != JSON_NUMBER)
        {
            this->free();
            this->value.type = JSON_NUMBER;
        }
        this->value.number = number;
        return *this;
    }

    json &json::operator=(const char *string)
    {
        if (this->value.type != JSON_STRING)
        {
            this->free();
            this->value.type = JSON_STRING;
        }
        this->value.string = new std::string(string);
        return *this;
    }

    json &json::operator[](size_t index)
    {
        if (this->value.type != JSON_ARRAY)
        {
            throw std::bad_cast();
        }
        return *(this->value.array->at(index));
    }

    const json &json::operator[](size_t index) const
    {
        if (this->value.type != JSON_ARRAY)
        {
            throw std::bad_cast();
        }
        return *(this->value.array->at(index));
    }

    std::ostream &operator<<(std::ostream &os, const json &v)
    {
        switch (v.get_type())
        {
        case JSON_FALSE:
            os << false;
            break;
        case JSON_TRUE:
            os << true;
            break;
        case JSON_NUMBER:
            os << v.value.number;
            break;
        case JSON_STRING:
            os << *v.value.string;
            break;
        case JSON_ARRAY:
            for (auto it = v.begin(); it != v.end(); ++it)
            {
                os << *it << " ";
            }
            break;
        case JSON_OBJECT:
            for (auto it = v.begin(); it != v.end(); ++it)
            {
                os << "key: " << it.key() << " value: " << it.value() << std::endl;
            }
            break;
        }
        return os;
    }
}