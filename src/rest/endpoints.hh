#ifndef __REST_HH
#define __REST_HH

#include <iostream>
#include <string>
#include <unordered_map>

#include <pthread.h>
#include <pistache/peer.h>
#include <pistache/http.h>
#include <pistache/router.h>
#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>
#include <spdlog/spdlog.h>
#include <pistache/endpoint.h>
#include "pistache/net.h"
#include "codec_module.hh"

//using namespace Net;
using namespace Pistache;
namespace spd = spdlog;


class Endpoints {
public:
    Endpoints(Address addr)
        : httpEndpoint(std::make_shared<Http::Endpoint>(addr)) {
            log_rest = spdlog::get("rest");//spdlog::stdout_color_mt("rest");
            log_rest->set_level(spdlog::level::debug);
	}

    void init(int threads = 1);

    void start();

    void shutdown();

private:
    void setup_routes();
    void put_input_config(const Rest::Request &request, Http::ResponseWriter response);
    void input_options(const Rest::Request &request, Http::ResponseWriter response);
    void info_options(const Rest::Request &request, Http::ResponseWriter response);
    void now_transcoding_options(const Rest::Request &request, Http::ResponseWriter response);
    void home(const Rest::Request& request, Http::ResponseWriter response);
    void discover(const Rest::Request& request, Http::ResponseWriter response);
    void transcoding(const Rest::Request& request, Http::ResponseWriter response);
    void kill(const Rest::Request& request, Http::ResponseWriter response);
    void kill_options(const Rest::Request& request, Http::ResponseWriter response);
    void handle_streaming_request(std::string config);
    void path(const Rest::Request& request, Http::ResponseWriter response);
    std::shared_ptr<spdlog::logger> log_rest;
    std::shared_ptr<Http::Endpoint> httpEndpoint;
    Rest::Router router;
    std::unordered_map<std::string, pthread_t> threads;
};

#endif
