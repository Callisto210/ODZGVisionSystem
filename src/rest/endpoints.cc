#include "endpoints.hh"

#include <config_generator.hh>
#include <discover.hh>
#include "rapidjson/stringbuffer.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"


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
    Routes::Post(router, "/info", Routes::bind(&Endpoints::discover, this));
}

void Endpoints::put_input_config(const Rest::Request &request, Http::ResponseWriter response) {
//    auto id = request.param(":id").as<int>();
    auto config = request.body();
//    log_rest->info("put /config at id: {}", id);
    log_rest->info("POST: /input -- {}", config);
    Document doc;
    Elements e;
    config_struct conf;

    conf.fps = -1;
    conf.audio_bitrate = -1;
    conf.video_bitrate = -1;
    conf.width = -1;
    conf.height = -1;

    try {
        doc.Parse(config.c_str());
        conf.source = doc["source"].GetString();
        conf.path = doc["path"].GetString();
        if(doc.HasMember("fps")) {
		if(doc["fps"].IsInt()) {
		    conf.fps = doc["fps"].GetInt();
		}
		else {
		    conf.fps = std::atoi(doc["fps"].GetString());
		}
	}

	if(doc.HasMember("acodec")) {
		if(doc["acodec"].IsString())
        		conf.acodec = doc["acodec"].GetString();
	}

	if(doc.HasMember("vcodec")) {
		if(doc["vcodec"].IsString())
        		conf.vcodec = doc["vcodec"].GetString();
	}

	if(doc.HasMember("sink")) {
		if(doc["sink"].IsString())
        		conf.sink = doc["sink"].GetString();
	}

	if(doc.HasMember("video_bitrate")) {
		if(doc["video_bitrate"].IsInt())
			conf.video_bitrate = doc["video_bitrate"].GetInt();
		else
			conf.video_bitrate = std::atoi(doc["video_bitrate"].GetString());
	}

	if(doc.HasMember("audio_bitrate")) {
		if(doc["audio_bitrate"].IsInt())
			conf.audio_bitrate = doc["audio_bitrate"].GetInt();
		else
			conf.audio_bitrate = std::atoi(doc["audio_bitrate"].GetString());
	}

	if(doc.HasMember("width")) {
		if(doc["width"].IsInt())
			conf.width = doc["width"].GetInt();
		else
			conf.width = std::atoi(doc["width"].GetString());
	}

	if(doc.HasMember("height")) {
		if(doc["height"].IsInt())
			conf.height = doc["height"].GetInt();
		else
			conf.height = std::atoi(doc["height"].GetString());
	}
	//response.send(Http::Code::Ok);
        configure_pipeline(e, response, conf);
        magic(e, WEBM_MUX);
    }catch(...) {
        log_rest->error("Cannot parse json :<");
    }

}

void Endpoints::home(const Rest::Request& request, Http::ResponseWriter response) {
    log_rest->info("GET /home");
    response.send(Http::Code::Ok);
}

void Endpoints::discover(const Rest::Request& request, Http::ResponseWriter response) {
    auto config = request.body();
//    log_rest->info("put /config at id: {}", id);
    log_rest->info("POST: /info -- {}", config);
    //response.send(Http::Code::Ok, "info");
    Document doc;
    string source;
    string uri;

    try {
        doc.Parse(config.c_str());
        source = doc["source"].GetString();
        uri = doc["uri"].GetString();
        discover_uri(response,uri,source);
    }
    catch (...){
        log_rest->error("Cannot parse json :<");
    }

}



