#include <string>

#include <config_generator.hh>
#include "rapidjson/stringbuffer.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"

using namespace std;
using namespace rapidjson; 

class streaming_handler {

private:
	string config;

public:
	streaming_handler(string config) {
		this->config = config;
	}

	void operator()();
};
