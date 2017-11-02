#ifndef __REST_HH
#define __REST_HH

#include <iostream>
#include <string>

#include <pistache/peer.h>
#include <pistache/http.h>
#include <pistache/router.h>

#include <spdlog/spdlog.h>
#include <pistache/endpoint.h>

//using namespace Net;
using namespace Pistache;
namespace spd = spdlog;


class Endpoints {
public:
    Endpoints(Address addr)
        : httpEndpoint(std::make_shared<Http::Endpoint>(addr)) {
            log_rest = spdlog::stdout_color_mt("rest");
            log_rest->set_level(spdlog::level::debug);
	}

    void init(size_t threads = 1);

    void start();

    void shutdown();

private:
    void setup_routes();
    void put_config(const Rest::Request& request, Http::ResponseWriter response);
    void home(const Rest::Request& request, Http::ResponseWriter response);
    void info(const Rest::Request& request, Http::ResponseWriter response);
    std::shared_ptr<spdlog::logger> log_rest;
    std::shared_ptr<Http::Endpoint> httpEndpoint;
    Rest::Router router;
};

#endif