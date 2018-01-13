#include <handler.hh>
#include <cxxabi.h>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <ctime>
#include <sstream>

map<string, config_struct *> streaming_handler::info;
mutex streaming_handler::mtx;

void streaming_handler::operator()() {
    Document doc;
    Elements e;
    config_struct *conf = new config_struct;

    conf->port = -1;


    try {
        doc.Parse(config.c_str());

	if(doc.HasMember("date")) {
		if(doc["date"].IsString())
        		conf->date = doc["date"].GetString();
	}

	if(doc.HasMember("time")) {
		if(doc["time"].IsString())
        		conf->time = doc["time"].GetString();
	}

	const Value& uri = doc["uri"];
	conf->n_uri = 0;
	for (SizeType i=0; i < uri.Size(); i++) {
		if(uri[i].IsString())
			conf->uri[conf->n_uri++] = uri[i].GetString();
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

	const Value& audio = doc["audio"];
    	conf->n_audio = 0;
	for (SizeType i=0; i < audio.Size(); i++) {

		const Value& doc = audio[i];
		conf->audio[conf->n_audio].audio_bitrate = -1;
		
		if(doc.HasMember("acodec")) {
			if(doc["acodec"].IsString())
				conf->audio[conf->n_audio].acodec = doc["acodec"].GetString();
		}

		if(doc.HasMember("audio_stream")) {
			if(doc["audio_stream"].IsString())
				conf->audio[conf->n_audio].audio_stream = doc["audio_stream"].GetString();
		}

		if(doc.HasMember("audio_bitrate")) {
			if(doc["audio_bitrate"].IsInt())
				conf->audio[conf->n_audio].audio_bitrate = doc["audio_bitrate"].GetInt();
			else
				conf->audio[conf->n_audio].audio_bitrate = std::atoi(doc["audio_bitrate"].GetString());
		}
		conf->n_audio++;
	}

	const Value& video = doc["video"];
    	conf->n_video = 0;
	for (SizeType i=0; i < video.Size(); i++) {

		const Value& doc = video[i];
		conf->video[conf->n_video].fps = -1;
		conf->video[conf->n_video].video_bitrate = -1;
		conf->video[conf->n_video].width = -1;
		conf->video[conf->n_video].height = -1;

		if(doc.HasMember("fps")) {
			if(doc["fps"].IsInt())
			    conf->video[conf->n_video].fps = doc["fps"].GetInt();
			else
			    conf->video[conf->n_video].fps = std::atoi(doc["fps"].GetString());
		}

		if(doc.HasMember("vcodec")) {
			if(doc["vcodec"].IsString())
				conf->video[conf->n_video].vcodec = doc["vcodec"].GetString();
		}

		if(doc.HasMember("video_stream")) {
			if(doc["video_stream"].IsString())
				conf->video[conf->n_video].video_stream = doc["video_stream"].GetString();
		}

		if(doc.HasMember("video_bitrate")) {
			if(doc["video_bitrate"].IsInt())
				conf->video[conf->n_video].video_bitrate = doc["video_bitrate"].GetInt();
			else
				conf->video[conf->n_video].video_bitrate = std::atoi(doc["video_bitrate"].GetString());
		}

		if(doc.HasMember("width")) {
			if(doc["width"].IsInt())
				conf->video[conf->n_video].width = doc["width"].GetInt();
			else
				conf->video[conf->n_video].width = std::atoi(doc["width"].GetString());
		}

		if(doc.HasMember("height")) {
			if(doc["height"].IsInt())
				conf->video[conf->n_video].height = doc["height"].GetInt();
			else
				conf->video[conf->n_video].height = std::atoi(doc["height"].GetString());
		}

		const Value& pip = doc["pip"];
		conf->video[conf->n_video].n_pip = 0;
		for (SizeType i=0; i < pip.Size(); i++) {
			const Value& doc = pip[i];
			conf->video[conf->n_video].pip[conf->video[conf->n_video].n_pip].pip_width = -1;
			conf->video[conf->n_video].pip[conf->video[conf->n_video].n_pip].pip_height = -1;
			conf->video[conf->n_video].pip[conf->video[conf->n_video].n_pip].x = -1;
			conf->video[conf->n_video].pip[conf->video[conf->n_video].n_pip].y = -1;
			if(doc.HasMember("pip_width")) {
				if(doc["pip_width"].IsInt())
					conf->video[conf->n_video].pip[conf->video[conf->n_video].n_pip].pip_width = doc["pip_width"].GetInt();
				else
					conf->video[conf->n_video].pip[conf->video[conf->n_video].n_pip].pip_width = std::atoi(doc["pip_width"].GetString());
			}

			if(doc.HasMember("pip_height")) {
				if(doc["pip_height"].IsInt())
					conf->video[conf->n_video].pip[conf->video[conf->n_video].n_pip].pip_height = doc["pip_height"].GetInt();
				else
					conf->video[conf->n_video].pip[conf->video[conf->n_video].n_pip].pip_height = std::atoi(doc["pip_height"].GetString());
			}

			if(doc.HasMember("x")) {
				if(doc["x"].IsInt())
					conf->video[conf->n_video].pip[conf->video[conf->n_video].n_pip].x = doc["x"].GetInt();
				else
					conf->video[conf->n_video].pip[conf->video[conf->n_video].n_pip].x = std::atoi(doc["x"].GetString());
			}

			if(doc.HasMember("y")) {
				if(doc["y"].IsInt())
					conf->video[conf->n_video].pip[conf->video[conf->n_video].n_pip].y = doc["y"].GetInt();
				else
					conf->video[conf->n_video].pip[conf->video[conf->n_video].n_pip].y = std::atoi(doc["y"].GetString());
			}

			if(doc.HasMember("pip_stream")) {
				if(doc["pip_stream"].IsString())
					conf->video[conf->n_video].pip[conf->video[conf->n_video].n_pip].pip_stream = doc["pip_stream"].GetString();
			}
			conf->video[conf->n_video].n_pip++;
		}
		conf->n_video++;

	}

	if (conf->random.empty()
	    || conf->uri[0].empty() /* At least one uri */
	    || conf->sink.empty()
	    || conf->mux.empty()) {	
		return;
	}
	conf->state = "stopped";

	if (!conf->date.empty() && !conf->time.empty()) {
		string dt = conf->date + " " + conf->time;
		istringstream ss(dt.c_str());
		struct tm when;
		cout << dt << endl;
		ss >> get_time(&when, "%Y-%m-%d %T");
		cout << put_time(&when, "%Y-%m-%d %T");
		this_thread::sleep_until(chrono::system_clock::from_time_t(mktime(&when)));
		cout << "Start!" << endl;
	}

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

