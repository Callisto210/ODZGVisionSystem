#include "json_handler.hh"
#include <cstdlib>

JsonHandler::JsonHandler(rapidjson::Document* doc)
{
    _doc = doc;
}

std::string JsonHandler::get_string_param(const char* key)
{
    if (_doc->HasMember(key)) {
        if ( (*_doc)[key].IsString()) {
            return (*_doc)[key].GetString();
        }
    }
    throw std::runtime_error(std::string(key) + "not found in JSON");
}

int JsonHandler::get_int_param(const char* key)
{
    if (_doc->HasMember(key)) {
        if  ((*_doc)[key].IsInt()) {
            return (*_doc)[key].GetInt();
        }
        else if ((*_doc)[key].IsString()) {
            return std::atoi((*_doc)[key].GetString());
        }
    }
}

