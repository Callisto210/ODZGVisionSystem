//
// Created by misiek on 03.11.17.
//

#ifndef ODZGVISIONSYSTEM_APP_CONFIG_HH
#define ODZGVISIONSYSTEM_APP_CONFIG_HH

#include <spdlog/spdlog.h>

void init_sinks();

void set_loggers();

void set_global_level(spdlog::level::level_enum);
#endif //ODZGVISIONSYSTEM_APP_CONFIG_HH
