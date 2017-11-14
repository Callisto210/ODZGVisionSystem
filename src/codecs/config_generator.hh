//
// Created by misiek on 03.11.17.
//

#ifndef ODZGVISIONSYSTEM_CONFIG_GENERATOR_HH
#define ODZGVISIONSYSTEM_CONFIG_GENERATOR_HH

#include <string>
#include <vector>
#include <spdlog/spdlog.h>
#ifdef __cplusplus
extern "C" {
#endif

#include "codec_module.h"
#ifdef __cplusplus
};

#endif
using std::string;

#include "rapidjson/stringbuffer.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include <pistache/http.h>

void configure_pipeline(Elements &e, string source, string path, int fps, string acodec, string vcodec, Pistache::Http::ResponseWriter &resp);


#endif //ODZGVISIONSYSTEM_CONFIG_GENERATOR_HH
