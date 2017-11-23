#include "endpoints.hh"

#include <config_generator.hh>
#include "rapidjson/stringbuffer.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include <handler.hh>

#include <thread>

using std::string;
using std::thread;
using rapidjson::Document;
//using namespace Net;
namespace spd = spdlog;

void Endpoints::init(size_t threads) {
    log_rest->info("Starting endpoints.");
    auto opts = Http::Endpoint::options()
        .threads(threads)
        .flags(Tcp::Options::InstallSignalHandler | Tcp::Options::ReuseAddr);
        httpEndpoint->init(opts);
        setup_routes();
}

void Endpoints::start() {
    log_rest->info("Endpoints::start");
    httpEndpoint->setHandler(router.handler());
    httpEndpoint->serveThreaded();
}

void Endpoints::shutdown() {
    log_rest->info("Shutting down http endpoint.");
    httpEndpoint->shutdown();
}


void Endpoints::setup_routes() {
    using namespace Rest;

    Routes::Post(router, "/input", Routes::bind(&Endpoints::put_input_config, this));
    Routes::Get(router, "/home", Routes::bind(&Endpoints::home, this));
    Routes::Get(router, "/info", Routes::bind(&Endpoints::info, this));
}

void Endpoints::put_input_config(const Rest::Request &request, Http::ResponseWriter response) {
	string config = request.body();
	log_rest->info("POST: /input -- {}", config);
	response.send(Http::Code::Ok);

	streaming_handler handle_streaming_request(config);
	thread stream(handle_streaming_request);
	stream.detach();
}

void Endpoints::home(const Rest::Request& request, Http::ResponseWriter response) {
    log_rest->info("GET /home");
    response.send(Http::Code::Ok);
}

void Endpoints::info(const Rest::Request& request, Http::ResponseWriter response) {
    log_rest->info("GET /info");
    response.send(Http::Code::Ok, "info");
}
