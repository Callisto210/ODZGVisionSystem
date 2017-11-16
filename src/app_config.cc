//
// Created by misiek on 03.11.17.
//

#include "app_config.hh"

static std::vector<spdlog::sink_ptr> sinks;
static std::vector<std::string> log_names;

void init_log_sinks()
{
    log_names = {"main", "filter", "config"};
    sinks.push_back(std::make_shared<spdlog::sinks::ansicolor_stderr_sink_mt>());
    sinks.push_back(std::make_shared<spdlog::sinks::daily_file_sink_mt>("logfile.txt", 15, 29));
}

void set_loggers()
{
    for(std::string& name: log_names) {
        auto l = std::make_shared<spdlog::logger>(name, sinks.begin(), sinks.end());
        spdlog::register_logger(l);
    }
}


void set_global_level(spdlog::level::level_enum log_level)
{
    for(std::string& name: log_names) {
        spdlog::get(name)->set_level(log_level);
    }
}