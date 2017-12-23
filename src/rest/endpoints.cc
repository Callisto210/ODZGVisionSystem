#include "endpoints.hh"

#include <config_generator.hh>
#include "json_handler.hh"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include <pthread.h>
#include <string>
#include <vector>
#include <cstdlib>
#include <stdexcept>
#include <csignal>
#include "path.hh"



using std::string;
using namespace rapidjson;
//using namespace Net;
namespace spd = spdlog;
using std::atoi;
using std::vector;
using std::string;

void Endpoints::init(int threads) {
    if (threads < 0) {
        throw std::invalid_argument("Threads count must be positive.\n");
    }
    log_rest->info("Starting endpoints.");
    auto opts = Http::Endpoint::options()
        .threads(threads)
        .flags(Tcp::Options::InstallSignalHandler | Tcp::Options::ReuseAddr);
    try {
        httpEndpoint->init(opts);
    }catch(std::runtime_error& e) {
        log_rest->error(e.what());
        throw e;
    }
    setup_routes();
}

void Endpoints::start() {
    log_rest->info("Endpoints::start");
    httpEndpoint->setHandler(router.handler());
    try{
        httpEndpoint->serveThreaded();
    }catch(std::runtime_error& e) {
        log_rest->error(e.what());
        kill(getpid(), SIGINT);
    }
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
    Routes::Post(router, "/path", Routes::bind(&Endpoints::path, this));
}

void Endpoints::put_input_config(const Rest::Request &request, Http::ResponseWriter response) {
    auto config = request.body();
    log_rest->debug("POST /input : {}", config);
    Document doc;
    Elements e;
    string acodec, vcodec, source, path;
    config_struct conf;
    JsonHandler json(&doc);

    conf.fps = -1;
    conf.audio_bitrate = -1;
    conf.video_bitrate = -1;
    conf.width = -1;
    conf.height = -1;

    try {
        doc.Parse(config.c_str());
        source = json.get_string_param("source");
        path = json.get_string_param("path");
        conf.fps = json.get_int_param("fps");
        acodec = json.get_string_param("acodec");
        vcodec = json.get_string_param("vcodec");
        conf.video_bitrate = json.get_int_param("video_bitrate");
        conf.audio_bitrate = json.get_int_param("audio_bitrate");
        conf.width = json.get_int_param("width");
        conf.height = json.get_int_param("height");

        response.send(Http::Code::Ok);
            configure_pipeline(e, source, path, acodec, vcodec, response, conf);
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


void Endpoints::path(const Rest::Request& request, Http::ResponseWriter response)
{
    log_rest->debug("GET /path");
    auto body = request.body();
    Document doc;
    string pathname;
    std::vector<string> ext;
    JsonHandler json(&doc);
    int depth = 3;
    try {
        doc.Parse(body.data());
        if (doc.HasParseError()) {
            log_rest->warn("Error when parsing path from {}", body);
            response.send(Http::Code::Bad_Request, "<p>Malformated json file</p>", MIME(Text, Html));
            return;
        }
        if (doc.HasMember("path") && doc["path"].IsString()) {
            pathname = doc["path"].GetString();
            log_rest->debug("Path name {} .", pathname);
        } else {
            response.send(Http::Code::No_Content, "<h1>No path parameter in Json</h1>",MIME(Text, Html));
            return;
        }
        if (doc.HasMember("ext") && doc["ext"].IsArray()) {
            rapidjson::SizeType ext_size = doc["ext"].Size();
            rapidjson::Value& ext_val = doc["ext"];
            for(SizeType i = 0; i < ext_size; i++) {
                if(ext_val[i].IsString())
                    ext.push_back(ext_val[i].GetString());
            }
        }
        if (doc.HasMember("depth")) {
            depth = json.get_int_param("depth");
        }

        log_rest->info("Finished parsing args");
        log_rest->debug("Ext list size: {}", ext.size());
    } catch(std::exception& e) {
        log_rest->error(e.what());
        response.send(Http::Code::Bad_Request, "Malformated json file", MIME(Text, Html));
        return;
    }

    if (!pathname.empty()) {
        PathDoc path_doc;
        Document ret_doc;
        Document::AllocatorType& alloc = ret_doc.GetAllocator();

        path_doc.create_list(pathname, ext, depth);
        vector<string> paths = path_doc.get_paths();
        log_rest->info("Creating list finished.");

        ret_doc.SetArray();
        for(string name: paths) {
            Value path_val(name.c_str(), alloc);
            ret_doc.PushBack(path_val, alloc);
        }

        StringBuffer str_buffer;
        Writer<StringBuffer> writer(str_buffer);
        ret_doc.Accept(writer);

        log_rest->debug("Response ready {}", str_buffer.GetString());
        response.send(Http::Code::Ok, str_buffer.GetString());
    } else {
        response.send(Http::Code::Bad_Request, "Bad request: Empty path");
    }
}


