#include <handler.hh>
#include <cxxabi.h>

map<string, config_struct *> streaming_handler::info;
mutex streaming_handler::mtx;

void streaming_handler::operator()() {
    Document doc;
    Elements e;
    config_struct *conf = new config_struct;

    conf->fps = -1;
    conf->audio_bitrate = -1;
    conf->video_bitrate = -1;
    conf->width = -1;
    conf->height = -1;
    conf->port = -1;

    try {
        doc.Parse(config.c_str());

	if(doc.HasMember("uri")) {
		if(doc["uri"].IsString())
        		conf->uri = doc["uri"].GetString();
	}

        if(doc.HasMember("fps")) {
		if(doc["fps"].IsInt()) {
		    conf->fps = doc["fps"].GetInt();
		}
		else {
		    conf->fps = std::atoi(doc["fps"].GetString());
		}
	}

        if(doc.HasMember("port")) {
		if(doc["port"].IsInt()) {
		    conf->port = doc["port"].GetInt();
		}
		else {
		    conf->port = std::atoi(doc["port"].GetString());
		}
	}

	if(doc.HasMember("acodec")) {
		if(doc["acodec"].IsString())
        		conf->acodec = doc["acodec"].GetString();
	}

	if(doc.HasMember("vcodec")) {
		if(doc["vcodec"].IsString())
        		conf->vcodec = doc["vcodec"].GetString();
	}

	if(doc.HasMember("audio_stream")) {
		if(doc["audio_stream"].IsString())
        		conf->audio_stream = doc["audio_stream"].GetString();
	}

	if(doc.HasMember("video_stream")) {
		if(doc["video_stream"].IsString())
        		conf->video_stream = doc["video_stream"].GetString();
	}

	if(doc.HasMember("sink")) {
		if(doc["sink"].IsString())
        		conf->sink = doc["sink"].GetString();
	}

	if(doc.HasMember("mux")) {
		if(doc["mux"].IsString())
        		conf->mux = doc["mux"].GetString();
	}

	if(doc.HasMember("video_bitrate")) {
		if(doc["video_bitrate"].IsInt())
			conf->video_bitrate = doc["video_bitrate"].GetInt();
		else
			conf->video_bitrate = std::atoi(doc["video_bitrate"].GetString());
	}

	if(doc.HasMember("audio_bitrate")) {
		if(doc["audio_bitrate"].IsInt())
			conf->audio_bitrate = doc["audio_bitrate"].GetInt();
		else
			conf->audio_bitrate = std::atoi(doc["audio_bitrate"].GetString());
	}

	if(doc.HasMember("width")) {
		if(doc["width"].IsInt())
			conf->width = doc["width"].GetInt();
		else
			conf->width = std::atoi(doc["width"].GetString());
	}

	if(doc.HasMember("height")) {
		if(doc["height"].IsInt())
			conf->height = doc["height"].GetInt();
		else
			conf->height = std::atoi(doc["height"].GetString());
	}

	if(doc.HasMember("host")) {
		if(doc["host"].IsString())
        		conf->host = doc["host"].GetString();
	}

	if(doc.HasMember("location")) {
		if(doc["location"].IsString())
        		conf->location = doc["location"].GetString();
	}

	if(doc.HasMember("random")) {
		if(doc["random"].IsString())
        		conf->random = doc["random"].GetString();
	}
	if (conf->random.empty()
	    || conf->uri.empty()
	    || conf->sink.empty()
	    || conf->mux.empty()) {	
		return;
	}
	conf->state = "stopped";

	mtx.lock();
	info[conf->random] = conf;
        configure_pipeline(e, conf);
	conf->state = "transcoding";
	mtx.unlock();

        magic(e, *conf);

	mtx.lock();
	info.erase(conf->random);
	mtx.unlock();

    }catch(abi::__forced_unwind&) {
    	throw;
    }catch(...) {
    	printf("it's a trap!\n");
    }

}

