#include <handler.hh>
#include <cxxabi.h>

map<string, config_struct *> streaming_handler::info;
mutex streaming_handler::mtx;

void streaming_handler::operator()() {
    Document doc;
    Elements e;
    config_struct *conf = new config_struct;

    conf->port = -1;

    conf->n_audio = 1;
    conf->n_video = 1;

    try {
        doc.Parse(config.c_str());

	if(doc.HasMember("uri")) {
		if(doc["uri"].IsString())
        		conf->uri = doc["uri"].GetString();
	}

        if(doc.HasMember("port")) {
		if(doc["port"].IsInt()) {
		    conf->port = doc["port"].GetInt();
		}
		else {
		    conf->port = std::atoi(doc["port"].GetString());
		}
	}

	if(doc.HasMember("sink")) {
		if(doc["sink"].IsString())
        		conf->sink = doc["sink"].GetString();
	}

	if(doc.HasMember("mux")) {
		if(doc["mux"].IsString())
        		conf->mux = doc["mux"].GetString();
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
    if(doc.HasMember("nvideo")) {
        if(doc["nvideo"].IsInt())
            conf->n_video = doc["nvideo"].GetInt();
        else
            conf->n_video = std::atoi(doc["nvideo"].GetString());
    }
    if(doc.HasMember("naudio")) {
        if(doc["naudio"].IsInt())
            conf->n_audio = doc["naudio"].GetInt();
        else
            conf->n_audio = std::atoi(doc["naudio"].GetString());
    }
    if(doc.HasMember("audio")) {
        for (int i = 0; i < conf->n_audio; i++) {

            conf->audio[i].audio_bitrate = -1;

            if (doc["audio"][i].HasMember("acodec")) {
                if (doc["audio"][i]["acodec"].IsString())
                    conf->audio[i].acodec = doc["audio"][i]["acodec"].GetString();
            }

            if (doc["audio"][i].HasMember("audio_stream")) {
                if (doc["audio"][i]["audio_stream"].IsString())
                    conf->audio[i].audio_stream = doc["audio"][i]["audio_stream"].GetString();
            }

            if (doc["audio"][i].HasMember("audio_bitrate")) {
                if (doc["audio"][i]["audio_bitrate"].IsInt())
                    conf->audio[i].audio_bitrate = doc["audio"][i]["audio_bitrate"].GetInt();
                else
                    conf->audio[i].audio_bitrate = std::atoi(doc["audio"][i]["audio_bitrate"].GetString());
            }

        }
    }
    if(doc.HasMember("video")) {
        for (int i = 0; i < conf->n_video; i++) {

            conf->video[i].fps = -1;
            conf->video[i].video_bitrate = -1;
            conf->video[i].width = -1;
            conf->video[i].height = -1;

            if (doc["video"][i].HasMember("fps")) {
                if (doc["video"][i]["fps"].IsInt())
                    conf->video[i].fps = doc["video"][i]["fps"].GetInt();
                else
                    conf->video[i].fps = std::atoi(doc["video"][i]["fps"].GetString());
            }

            if (doc["video"][i].HasMember("vcodec")) {
                if (doc["video"][i]["vcodec"].IsString())
                    conf->video[i].vcodec = doc["video"][i]["vcodec"].GetString();
            }

            if (doc["video"][i].HasMember("video_stream")) {
                if (doc["video"][i]["video_stream"].IsString())
                    conf->video[i].video_stream = doc["video"][i]["video_stream"].GetString();
            }

            if (doc["video"][i].HasMember("video_bitrate")) {
                if (doc["video"][i]["video_bitrate"].IsInt())
                    conf->video[i].video_bitrate = doc["video"][i]["video_bitrate"].GetInt();
                else
                    conf->video[i].video_bitrate = std::atoi(doc["video"][i]["video_bitrate"].GetString());
            }

            if (doc["video"][i].HasMember("width")) {
                if (doc["video"][i]["width"].IsInt())
                    conf->video[i].width = doc["video"][i]["width"].GetInt();
                else
                    conf->video[i].width = std::atoi(doc["video"][i]["width"].GetString());
            }

            if (doc["video"][i].HasMember("height")) {
                if (doc["video"][i]["height"].IsInt())
                    conf->video[i].height = doc["video"][i]["height"].GetInt();
                else
                    conf->video[i].height = std::atoi(doc["video"][i]["height"].GetString());
            }

        }
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

