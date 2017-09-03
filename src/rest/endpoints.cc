#include "endpoints.hh"

using std::string;
//using namespace Net;
namespace spd = spdlog;




void Endpoints::init(size_t threads) {
    log_rest->info("Starting endpoints.");
    auto opts = Http::Endpoint::options()
        .threads(threads)
        .flags(Tcp::Options::InstallSignalHandler);
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

    Routes::Post(router, "/config/:id", Routes::bind(&Endpoints::put_config, this));
    Routes::Get(router, "/home", Routes::bind(&Endpoints::home, this));
    Routes::Get(router, "/info", Routes::bind(&Endpoints::info, this));
}

void Endpoints::put_config(const Rest::Request& request, Http::ResponseWriter response) {
    auto id = request.param(":id").as<int>();
    auto config = request.body();
    log_rest->info("put /config at id: {}", id);
    log_rest->info("PUT: /config -- {}", config);
    response.send(Http::Code::Ok, config);
}

void Endpoints::home(const Rest::Request& request, Http::ResponseWriter response) {
    log_rest->info("GET /home");
    response.send(Http::Code::Ok);
}

void Endpoints::info(const Rest::Request& request, Http::ResponseWriter response) {
    log_rest->info("GET /info");
    response.send(Http::Code::Ok, "info");
}



