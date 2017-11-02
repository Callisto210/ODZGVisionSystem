#include <pistache/net.h>
#include "rest/endpoints.hh"
#include <spdlog/spdlog.h>
#include <thread>
#include <vector>
extern "C" {
#include "codecs/codec_module.h"
}
namespace spd = spdlog;
using namespace Pistache;

#include "test_main.hh"
int main(int argc, char** argv) {
    std::vector<spdlog::sink_ptr> sinks;
    sinks.push_back(std::make_shared<spdlog::sinks::stdout_sink_mt>());
    sinks.push_back(std::make_shared<spdlog::sinks::daily_file_sink_mt>("logfile.txt", 15, 29));
    auto main_log = std::make_shared<spdlog::logger>("main", sinks.begin(), sinks.end());
    auto filter_log = std::make_shared<spdlog::logger>("filter", sinks.begin(), sinks.end());

    filter_log->set_level(spdlog::level::debug);
    spdlog::register_logger(filter_log);
    spdlog::register_logger(main_log);
    main_log->info("Starting...");
    Port port(8090);
    int threads = 1;
    Address addr(Ipv4::any(), port);
    main_log->info("Listening on 0.0.0.0:{} ",port);
    Endpoints api(addr);
    try {
        api.init(threads);
        std::thread api_thread = std::thread(&Endpoints::start, api);
        main_log->debug("Noted start of server");
        char s;
        std::thread back(test_pipeline);
        std::cin>>s;
        api.shutdown();
        main_log->info("called shutdown");
        api_thread.join();
        back.join();
    } catch(...) {
        main_log->warn("Catched an exception.");
        
    }
    return 0;
}
