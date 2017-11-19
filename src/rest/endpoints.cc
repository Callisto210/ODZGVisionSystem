#include "endpoints.hh"

#include <config_generator.hh>
#include "rapidjson/stringbuffer.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include <pthread.h>


using std::string;
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
//    auto id = request.param(":id").as<int>();
    auto config = request.body();
//    log_rest->info("put /config at id: {}", id);
    log_rest->info("POST: /input -- {}", config);
    Document doc;
    Elements e;
    string acodec, vcodec, source, path;
    int fps;
//    configure_pipeline(config.c_str());
    try {
        doc.Parse(config.c_str());
        if(doc.HasParseError()){
             response.send(Http::Code::Bad_Request);
             return;
        }
        if(doc.HasMember("source") && doc["source"].IsString())
            source = doc["source"].GetString();
        else{
            response.send(Http::Code::Bad_Request);
            return;
        }
        if(source == "file") {
            if(doc.HasMember("path") && doc["path"].IsString()) {
                path = doc["path"].GetString();
            }else {
                response.send(Http::Code::Bad_Request);
                return;
            }
            if(doc.HasMember("fps") && doc["fps"].IsInt()) {
                fps = doc["fps"].GetInt();
            }
            else {
                log_rest->debug("fps is not an int: {}", doc["fps"].GetString());
                fps = std::atoi(doc["fps"].GetString());
            }
        }
        if(doc.HasMember("acodec") && doc["acodec"].IsString())
            acodec = doc["acodec"].GetString();
        if(doc.HasMember("vcodec") && doc["vcodec"].IsString())
            vcodec = doc["vcodec"].GetString();
        response.send(Http::Code::Ok);
        configure_pipeline(e, source, path, fps, acodec, vcodec);
        magic(e, ICECAST, WEBM_MUX);
        log_rest->debug("Parsing json completed successfully.");
        log_rest->debug("Do the magic");
    }catch(...) {
        log_rest->error("Cannot parse json :<");
        response.send(Http::Code::Bad_Request);
        return;
    }

}

void Endpoints::home(const Rest::Request& request, Http::ResponseWriter response) {
    log_rest->info("GET /home");
    response.send(Http::Code::Ok);
}

void Endpoints::info(const Rest::Request& request, Http::ResponseWriter response) {
    log_rest->info("GET /info");
    response.send(Http::Code::Ok, "info");
}



