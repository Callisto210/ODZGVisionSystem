#include "endpoints.hh"

#include <config_generator.hh>
#include "json_handler.hh"
#include "discover.hh"
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
#include "handler.hh"


#include <pistache/net.h>
#include <pistache/http.h>
#include <pistache/peer.h>
#include <pistache/http_headers.h>
#include <pistache/cookie.h>
#include <pistache/endpoint.h>
#include <thread>
#include <pthread.h>

using namespace Pistache;
using namespace Rest;
using namespace rapidjson;
using std::string;
using std::thread;

//using namespace Net;
namespace spd = spdlog;
using std::atoi;
using std::vector;
using std::string;

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
void Endpoints::init(int threads) {
    if (threads < 0) {
        throw std::invalid_argument("Threads count must be positive.\n");
    }
    log_rest->info("Starting endpoints.");
    Http::Header::Registry::registerHeader<AccessControlAllowMethods>();
    Http::Header::Registry::registerHeader<AccessControlAllowHeaders>();
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
        ::kill(getpid(), SIGINT);
    }
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
    Routes::Options(router, "/info", Routes::bind(&Endpoints::info_options, this));
    Routes::Get(router, "/now_transcoding", Routes::bind(&Endpoints::transcoding, this));
    Routes::Options(router, "/now_transcoding", Routes::bind(&Endpoints::now_transcoding_options, this));
    Routes::Post(router, "/kill", Routes::bind(&Endpoints::kill, this));
    Routes::Options(router, "/kill", Routes::bind(&Endpoints::kill_options, this));
    Routes::Post(router, "/path", Routes::bind(&Endpoints::path, this));
}

void Endpoints::put_input_config(const Rest::Request &request, Http::ResponseWriter response) {
	string config = request.body();
	log_rest->info("POST: /input -- {}", config);
	response.send(Http::Code::Ok);

	Document doc;
	string random;

        doc.Parse(config.c_str());
        random = doc["random"].GetString();

	streaming_handler handle_streaming_request(config);
	thread stream(handle_streaming_request);
	threads[random] = stream.native_handle();
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

void Endpoints::info_options(const Rest::Request &request, Http::ResponseWriter response) {
	auto orig = request.headers().getRaw("Origin");
	auto headers = request.headers().getRaw("Access-Control-Request-Headers");
	
	response.headers().add<Http::Header::AccessControlAllowOrigin>(orig.value());
	response.headers().add<AccessControlAllowHeaders>(headers.value());
	response.headers().add<AccessControlAllowMethods>("POST");
	response.send(Http::Code::Ok);
}

void Endpoints::now_transcoding_options(const Rest::Request &request, Http::ResponseWriter response) {
	auto orig = request.headers().getRaw("Origin");
	auto headers = request.headers().getRaw("Access-Control-Request-Headers");
	
	response.headers().add<Http::Header::AccessControlAllowOrigin>(orig.value());
	response.headers().add<AccessControlAllowHeaders>(headers.value());
	response.headers().add<AccessControlAllowMethods>("GET");
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
    auto orig = request.headers().getRaw("Origin");
    response.headers().add<Http::Header::AccessControlAllowOrigin>(orig.value());
    Document doc;
    string uri;
    JsonHandler json(&doc);

    try {
        doc.Parse(config.c_str());
	uri = json.get_string_param("uri");	

	 discover_uri(response,uri);
    }catch(...) {
        log_rest->error("Cannot parse json :<");
        response.send(Http::Code::Bad_Request);
        return;
    }

}

void Endpoints::transcoding(const Rest::Request& request, Http::ResponseWriter response) {
	Document doc;
	Value array(kArrayType);
	Document::AllocatorType& alloc = doc.GetAllocator();
    auto orig = request.headers().getRaw("Origin");
    response.headers().add<Http::Header::AccessControlAllowOrigin>(orig.value());
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

		if (!it->second->audio.acodec.empty()) {
			Value str(kObjectType);
			str.SetString(it->second->audio.acodec.c_str(), alloc);
			obj.AddMember("acodec", str, alloc);
		}

		if (!it->second->video.vcodec.empty()) {
			Value str(kObjectType);
			str.SetString(it->second->video.vcodec.c_str(), alloc);
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
		
		if (it->second->audio.audio_bitrate != -1) {
			Value str(kObjectType);
			str.SetInt(it->second->audio.audio_bitrate);
			obj.AddMember("audio_bitrate", str, alloc);
		}
		
		if (it->second->video.video_bitrate != -1) {
			Value str(kObjectType);
			str.SetInt(it->second->video.video_bitrate);
			obj.AddMember("video_bitrate", str, alloc);
		}
		if (it->second->port != -1) {
			Value str(kObjectType);
			str.SetInt(it->second->port);
			obj.AddMember("port", str, alloc);
		}
		
		if (it->second->video.width != -1 && it->second->video.height != -1) {
			Value str(kObjectType);
			str.SetInt(it->second->video.width);
			obj.AddMember("width", str, alloc);

			str.SetInt(it->second->video.height);
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


void Endpoints::kill(const Rest::Request& request, Http::ResponseWriter response) {
    auto config = request.body();
    log_rest->info("POST: /kill -- {}", config);
    response.send(Http::Code::Ok);
    
    Document doc;
    string random;

   streaming_handler::mtx.lock();

    try {
        doc.Parse(config.c_str());
        random = doc["random"].GetString();
	pthread_cancel(threads[random]);
	threads.erase(random);
	streaming_handler::info.erase(random);
	
    }
    catch (...){
        log_rest->error("Cannot parse json :<");
    }

    streaming_handler::mtx.unlock();

}

void Endpoints::kill_options(const Rest::Request &request, Http::ResponseWriter response) {
	auto orig = request.headers().getRaw("Origin");
	auto headers = request.headers().getRaw("Access-Control-Request-Headers");
	
	response.headers().add<Http::Header::AccessControlAllowOrigin>(orig.value());
	response.headers().add<AccessControlAllowHeaders>(headers.value());
	response.headers().add<AccessControlAllowMethods>("POST");
	response.send(Http::Code::Ok);
}
