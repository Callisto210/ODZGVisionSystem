#include "endpoints.hh"

#include <config_generator.hh>
#include <discover.hh>
#include "rapidjson/stringbuffer.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include <handler.hh>

#include <pistache/net.h>
#include <pistache/http.h>
#include <pistache/peer.h>
#include <pistache/http_headers.h>
#include <pistache/cookie.h>
#include <pistache/endpoint.h>
#include <thread>

using namespace Pistache;
using namespace Rest;
using std::string;
using std::thread;
using rapidjson::Document;
//using namespace Net;
namespace spd = spdlog;

class AccessControlAllowMethods : public Http::Header::Header {
public:

  NAME("Access-Control-Allow-Methods")

  AccessControlAllowMethods() { }

  AccessControlAllowMethods(const char* methods)
    : methods_(methods)
  { }

  AccessControlAllowMethods(const std::string& methods)
    : methods_(methods)
  { }

  void parse(const std::string& data) {}
  void write(std::ostream& os) const {
  	os << methods_;
  }

  std::string methods() const { return methods_; }

private:
  std::string methods_;
};

class AccessControlAllowHeaders : public Http::Header::Header {
public:

  NAME("Access-Control-Allow-Headers")

  AccessControlAllowHeaders() { }

  AccessControlAllowHeaders(const char* headers)
    : headers_(headers)
  { }

  AccessControlAllowHeaders(const std::string& headers)
    : headers_(headers)
  { }

  void parse(const std::string& data) {}
  void write(std::ostream& os) const {
  	os << headers_;
  }

  std::string headers() const { return headers_; }

private:
  std::string headers_;
};

void Endpoints::init(size_t threads) {
    log_rest->info("Starting endpoints.");
    Http::Header::Registry::registerHeader<AccessControlAllowMethods>();
    Http::Header::Registry::registerHeader<AccessControlAllowHeaders>();
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

    Routes::Post(router, "/input", Routes::bind(&Endpoints::put_input_config, this));
    Routes::Options(router, "/input", Routes::bind(&Endpoints::input_options, this));
    Routes::Get(router, "/home", Routes::bind(&Endpoints::home, this));
    Routes::Post(router, "/info", Routes::bind(&Endpoints::discover, this));
    Routes::Get(router, "/now_transcoding", Routes::bind(&Endpoints::transcoding, this));
}

void Endpoints::put_input_config(const Rest::Request &request, Http::ResponseWriter response) {
	string config = request.body();
	log_rest->info("POST: /input -- {}", config);
	response.send(Http::Code::Ok);

	streaming_handler handle_streaming_request(config);
	thread stream(handle_streaming_request);
	stream.detach();
}

void Endpoints::input_options(const Rest::Request &request, Http::ResponseWriter response) {
	auto orig = request.headers().getRaw("Origin");
	auto headers = request.headers().getRaw("Access-Control-Request-Headers");
	
	response.headers().add<Http::Header::AccessControlAllowOrigin>(orig.value());
	response.headers().add<AccessControlAllowHeaders>(headers.value());
	response.headers().add<AccessControlAllowMethods>("POST");
	response.send(Http::Code::Ok);
}

void Endpoints::home(const Rest::Request& request, Http::ResponseWriter response) {
    log_rest->info("GET /home");
    response.send(Http::Code::Ok);
}

void Endpoints::discover(const Rest::Request& request, Http::ResponseWriter response) {
    auto config = request.body();
//    log_rest->info("put /config at id: {}", id);
    log_rest->info("POST: /info -- {}", config);
    Document doc;
    string uri;

    try {
        doc.Parse(config.c_str());
        uri = doc["uri"].GetString();
        discover_uri(response,uri);
    }
    catch (...){
        log_rest->error("Cannot parse json :<");
    }

}

void Endpoints::transcoding(const Rest::Request& request, Http::ResponseWriter response) {
	Document doc;
	Value array(kArrayType);
	Document::AllocatorType& alloc = doc.GetAllocator();

	doc.SetObject();

	streaming_handler::mtx.lock();
	for (std::map<string, config_struct *>::iterator it=streaming_handler::info.begin();
	    it!=streaming_handler::info.end();
	    ++it) {
		Value obj(kObjectType);

		if (!it->second->random.empty()) {
			Value str(kObjectType);
			str.SetString(it->second->random.c_str(), alloc);
			obj.AddMember("random", str, alloc);
		}

		if (!it->second->state.empty()) {
			Value str(kObjectType);
			str.SetString(it->second->state.c_str(), alloc);
			obj.AddMember("state", str, alloc);
		}

		if (!it->second->acodec.empty()) {
			Value str(kObjectType);
			str.SetString(it->second->acodec.c_str(), alloc);
			obj.AddMember("acodec", str, alloc);
		}

		if (!it->second->vcodec.empty()) {
			Value str(kObjectType);
			str.SetString(it->second->vcodec.c_str(), alloc);
			obj.AddMember("vcodec", str, alloc);
		}
		
		if (!it->second->host.empty()) {
			Value str(kObjectType);
			str.SetString(it->second->host.c_str(), alloc);
			obj.AddMember("host", str, alloc);
		}

		if (!it->second->location.empty()) {
			Value str(kObjectType);
			str.SetString(it->second->location.c_str(), alloc);
			obj.AddMember("location", str, alloc);
		}

		if (!it->second->uri.empty()) {
			Value str(kObjectType);
			str.SetString(it->second->uri.c_str(), alloc);
			obj.AddMember("uri", str, alloc);
		}
		
		if (it->second->audio_bitrate != -1) {
			Value str(kObjectType);
			str.SetInt(it->second->audio_bitrate);
			obj.AddMember("audio_bitrate", str, alloc);
		}
		
		if (it->second->video_bitrate != -1) {
			Value str(kObjectType);
			str.SetInt(it->second->video_bitrate);
			obj.AddMember("video_bitrate", str, alloc);
		}
		if (it->second->port != -1) {
			Value str(kObjectType);
			str.SetInt(it->second->port);
			obj.AddMember("port", str, alloc);
		}
		
		if (it->second->width != -1 && it->second->height != -1) {
			Value str(kObjectType);
			str.SetInt(it->second->width);
			obj.AddMember("width", str, alloc);

			str.SetInt(it->second->height);
			obj.AddMember("height", str, alloc);
		}

		array.PushBack(obj, alloc);
	}
	streaming_handler::mtx.unlock();
	
	doc.AddMember("transcoding", array, alloc);

	StringBuffer strbuf;
	Writer<StringBuffer> writer(strbuf);
	doc.Accept(writer);
	g_print("%s\n", strbuf.GetString());
	response.send(Http::Code::Ok, strbuf.GetString());
}
