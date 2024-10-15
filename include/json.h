#ifndef TY_JSON
#define TY_JSON

#include <string>

namespace ty
{
    typedef enum
    {
        JSON_PARSE_OK,
        JSON_PARSE_FAIL,
        JSON_PARSE_OUT_RANGE,
    } json_parse_status;

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

    typedef struct json_value json_value;
    struct json_value
    {
        json_type type;
    };

    class json
    {
    public:
        json() = default;
        ~json() = default;
        json_value *parse(const std::string json_str);
        json_parse_status parse_value(const std::string json_str, size_t index, json_value* v);
        json_parse_status parse_value_null(const std::string json_str, size_t index, json_value* v);

    };
}

#endif