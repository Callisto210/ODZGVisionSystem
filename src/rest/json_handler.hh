#ifndef JSON_HANDLER_HH
#define JSON_HANDLER_HH
#include <string>
#include "rapidjson/document.h"

class JsonHandler
{
public:
    JsonHandler(rapidjson::Document* doc);
    std::string get_string_param(const char*);
    int get_int_param(const char*);

    JsonHandler() = default;
    JsonHandler(const JsonHandler&) = default;
    JsonHandler(JsonHandler&&) = default;
    JsonHandler& operator=(const JsonHandler&) = default;
    JsonHandler& operator=(JsonHandler&&) = default;
    virtual ~JsonHandler() = default;
private:
    rapidjson::Document* _doc;
};

#endif // JSON_HANDLER_HH
