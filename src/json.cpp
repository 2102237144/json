#include "../include/json.h"
namespace ty
{
    json_value *json::parse(const std::string json_str)
    {
        json_value *v = new json_value();
        if (parse_value(json_str, 0, v) != JSON_PARSE_OK)
        {
            return nullptr;
        }
        return v;
    }

    json_parse_status json::parse_value(const std::string json_str, size_t index, json_value *v)
    {
        json_parse_status status;
        switch (json_str[index])
        {
        case 'n':
            status = parse_value_null(json_str, index, v);
            break;

        default:
            break;
        }
        return JSON_PARSE_OK;
    }

    json_parse_status json::parse_value_null(const std::string json_str, size_t index, json_value *v)
    {
        if (index + 4 > json_str.size())
        {
            return JSON_PARSE_OUT_RANGE;
        }
        if (json_str[index] != 'n' || json_str[index + 1] != 'u' || json_str[index + 2] != 'l' || json_str[index + 3] != 'l')
        {
            return JSON_PARSE_FAIL;
        }
        v->type = JSON_NULL;
        return JSON_PARSE_OK;
    }

}
